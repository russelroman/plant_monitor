#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ble.h"
#include "ble_advertising.h"
#include "app_timer.h"
#include "bsp.h"
#include "nrf_drv_saadc.h"

#include "sensors.h"
#include "temp_hum.h"
#include "light.h"


#define SENSOR_UPDATE_INTERVAL APP_TIMER_TICKS(5000)  // Unit of 1 ms
#define APP_BLE_CONN_CFG_TAG 1U
#define DEVICE_NAME         "x-parasite"
#define APP_ADV_INTERVAL  3200     // Units of 0.625 ms
#define APP_ADV_DURATION  0       // No Timeout


BLE_ADVERTISING_DEF(m_advertising);
APP_TIMER_DEF(m_app_timer_id);


static ble_advdata_t new_advdata;


void ble_stack_init(void)
{
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  uint32_t ram_start = 0U;

  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
}


static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
  ret_code_t err_code;

  switch(ble_adv_evt)
  {
    case BLE_ADV_EVT_FAST:

      NRF_LOG_INFO("Fast advertising...");

      break;

    case BLE_ADV_EVT_IDLE:

      NRF_LOG_INFO("Idle...");
     
      break;

    default:
      break;
  }
}


void advertising_init(void)
{
  ret_code_t err_code;

  ble_advertising_init_t init;
  ble_advdata_manuf_data_t advert_data;

  uint8_t manu_data[30];

  sensor_data_update();
  pack_sensor_data(manu_data);

  memset(&init, 0, sizeof(init));

  init.advdata.name_type = BLE_ADVDATA_SHORT_NAME;
  init.advdata.short_name_len = 3;

  init.advdata.include_appearance = false;

  init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

  init.config.ble_adv_fast_enabled = true;
  init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
  init.config.ble_adv_fast_timeout = APP_ADV_DURATION;

  advert_data.company_identifier = 0x0001;
  advert_data.data.p_data = manu_data;
  advert_data.data.size = 16;

  init.advdata.p_manuf_specific_data = &advert_data;

  init.evt_handler = on_adv_evt;

  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


void advertising_start(void)
{
    ret_code_t err_code;

    /* The ble_advertising_start sets the advertising mode 
       to a mode according to its parameter. In this application,
       the advertising mode must be Non-connectable and Non-scannable
       so the GAP parameters must be configured after ble_advertising_start.
    */

    err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_stop(m_advertising.adv_handle);
    APP_ERROR_CHECK(err_code);
 
    ble_gap_adv_params_t  adv_params;
  
    memcpy(&adv_params, &m_advertising.adv_params, sizeof(adv_params));

    adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;

    err_code = sd_ble_gap_adv_set_configure(&m_advertising.adv_handle,
                                            &m_advertising.adv_data,
                                            &adv_params);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_start(m_advertising.adv_handle, m_advertising.conn_cfg_tag);
    APP_ERROR_CHECK(err_code);
}




static void app_timer_handler(void *p_context)
{
  ret_code_t err_code;
 
  nrf_gpio_pin_toggle(LED_4);
  uint8_t manu_data[30];

  ble_advdata_manuf_data_t manuf_data;
  manuf_data.data.p_data = manu_data;
  manuf_data.data.size = 16;

  manuf_data.company_identifier = 0x0001;

  new_advdata.name_type = BLE_ADVDATA_SHORT_NAME;
  new_advdata.short_name_len = 3;
  
  new_advdata.p_manuf_specific_data = &manuf_data;
  
  sensor_data_update();
  pack_sensor_data(manu_data);

  err_code = ble_advertising_advdata_update(&m_advertising, &new_advdata, NULL);
  APP_ERROR_CHECK(err_code);
}


void timers_init(void)
{
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, app_timer_handler);
  APP_ERROR_CHECK(err_code);
}

void start_timer(void)
{
  uint32_t err_code = app_timer_start(m_app_timer_id, SENSOR_UPDATE_INTERVAL, NULL);
}


void set_random_static_address(void)
{
  ret_code_t err_code;

  static ble_gap_addr_t rs_addr;

  rs_addr.addr[0] = 0xCE;
  rs_addr.addr[1] = 0x98;
  rs_addr.addr[2] = 0x23;
  rs_addr.addr[3] = 0x66;
  rs_addr.addr[4] = 0x33;
  rs_addr.addr[5] = 0xff;

  rs_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;

  err_code = sd_ble_gap_addr_set(&rs_addr);
  if(err_code != NRF_SUCCESS)
  {
    NRF_LOG_INFO("Failed to set random static address");
  }
}


void get_random_static_address(void)
{
  ret_code_t err_code;

  static ble_gap_addr_t my_addr;

  err_code = sd_ble_gap_addr_get(&my_addr);

  if(err_code == NRF_SUCCESS)
  {
    NRF_LOG_INFO("Address Type: %02X", my_addr.addr_type);
    NRF_LOG_INFO("Device Address is %02X:%02X:%02X:%02X:%02X:%02X",
                                    my_addr.addr[0],  my_addr.addr[1],
                                    my_addr.addr[2],  my_addr.addr[3],
                                    my_addr.addr[4],  my_addr.addr[5]   ) ;
  }
}