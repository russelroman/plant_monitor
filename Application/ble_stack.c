#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_ble_gatt.h"

#include "nrf_ble_qwr.h"

#include "ble_conn_params.h"

#include "ble.h"

#include "ble_advertising.h"

#include "app_timer.h"

#include "bsp.h"

#include "sensors.h"
#include "temp_hum.h"
#include "light.h"

#include "nrf_drv_saadc.h"

#define LED_INTERVAL APP_TIMER_TICKS(20000)

#define APP_BLE_CONN_CFG_TAG 1U
#define APP_BLE_OBSERVER_PRIO 3U

#define DEVICE_NAME         "x-parasite"

#define FIRST_CONN_PARMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARMS_UPDATE_DELAY    APP_TIMER_TICKS(30000)
#define MAX_CONN_PARMS_UPDATE_COUNT     3U

#define APP_ADV_INTERVAL  300
#define APP_ADV_DURATION  0   // No Timeout



/* Set MIN_CONN_INTERVAL to 100ms 
   Soft device uses RTC0 which has minimum time of 1.25ms.
   Thus, we need to convert 100ms according to unit of 1.25ms.
*/
#define MIN_CONN_INTERVAL   MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL   MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY       0
#define CONN_SUP_TIMEOUT    MSEC_TO_UNITS(2000, UNIT_10_MS) // Timeout after 2 seconds.

NRF_BLE_GATT_DEF(m_gatt);

NRF_BLE_QWR_DEF(m_qwr); // Use QWRS if connecting with multiple devices

BLE_ADVERTISING_DEF(m_advertising);

APP_TIMER_DEF(m_app_timer_id);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;


/* Step 5.1 : BLE Event handler */
void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
  ret_code_t err_code = NRF_SUCCESS;

  switch(p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_DISCONNECTED:

      NRF_LOG_INFO("Device is Disconnected!!!");
      
      break;

    case BLE_GAP_EVT_CONNECTED:

      NRF_LOG_INFO("Device is Connected!!!");

      //err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
      //APP_ERROR_CHECK(err_code);

      //m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

     // err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
     // APP_ERROR_CHECK(err_code);

      break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    
      #if 0
      NRF_LOG_INFO("PHY Update Request!!!");

      ble_gap_phys_t const phys = 
      {
        .rx_phys = BLE_GAP_PHY_AUTO,
        .tx_phys = BLE_GAP_PHY_AUTO
      };

      err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
      #endif

      break;
  }
  
}


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
}


void gap_params_init(void)
{
  ret_code_t err_code;

  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);  // No Protection

  err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
  APP_ERROR_CHECK(err_code);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  APP_ERROR_CHECK(err_code);
}

void gatt_init(void)
{
  ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL); // Passing NULL will not handle GATT Events
  APP_ERROR_CHECK(err_code);
}

static void conn_params_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}

/* Step 10. 1 Handler for connection parameters update */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
  ret_code_t err_code;

  if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
  {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }

  if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
  {
    
  }
}

void conn_params_init(void)
{
  ret_code_t err_code;

  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params = NULL;

  cp_init.first_conn_params_update_delay = FIRST_CONN_PARMS_UPDATE_DELAY;

  cp_init.next_conn_params_update_delay = NEXT_CONN_PARMS_UPDATE_DELAY;

  cp_init.max_conn_params_update_count = MAX_CONN_PARMS_UPDATE_COUNT;

  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;

  cp_init.disconnect_on_fail = false;

  cp_init.error_handler = conn_params_error_handler;

  cp_init.evt_handler = on_conn_params_evt;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);

}

/* Step 9.1 Error handler for queue writer */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);

}

/* Step 9. Initialize Services */
void services_init(void)
{
  ret_code_t err_code;

  nrf_ble_qwr_init_t qwr_init = {0};

  qwr_init.error_handler = nrf_qwr_error_handler;

  err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
  APP_ERROR_CHECK(err_code);
}


/* Step 8.1 Adv. Handlder*/
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
  ret_code_t err_code;

  switch(ble_adv_evt)
  {
    case BLE_ADV_EVT_FAST:

      NRF_LOG_INFO("Fast advertising...");
      err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
      APP_ERROR_CHECK(err_code);

      break;

    case BLE_ADV_EVT_IDLE:

      NRF_LOG_INFO("Idle...");
      err_code = bsp_indication_set(BSP_INDICATE_IDLE);
      APP_ERROR_CHECK(err_code);

      break;

    default:
      break;
  }
}


extern sensor_data_t sensor_data;

/* Step 8 */
void advertising_init(void)
{
  ret_code_t err_code;

  ble_advertising_init_t init;
  ble_advdata_manuf_data_t advert_data;

  uint8_t manu_data[30];
  sensor_data.tempe_data.tempe_val = 37.57f;
  sensor_data.humid_data.humid_val = 37.57f;
  sensor_data.light_data.light_val = 5200;
  sensor_data.moist_data.moist_val = 37.57f;

  pack_sensor_data(&sensor_data, manu_data);

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

/* Step 11 Start Advertisement */
void advertising_start(void)
{
    ret_code_t err_code;
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


ble_advdata_t new_advdata;
ble_advdata_t new_srdata;

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
  new_srdata.name_type = BLE_ADVDATA_FULL_NAME;

  /**/
  #if 1
  float temp;
  float hum;
  float lux_val = 0.0f;
  float current_photo = 0;
  float voltage_photo = 0;
  float voltage_supply = 3.0f;
  const float lux_sun = 10000.0f;
  const float current_sun = 3.59e-3f;
  const float photo_res_val = 470;

  read_data_shtc(&temp, &hum);
  NRF_LOG_INFO("Temp: " NRF_LOG_FLOAT_MARKER " C", NRF_LOG_FLOAT(temp));
  NRF_LOG_INFO("Hum: " NRF_LOG_FLOAT_MARKER " %%", NRF_LOG_FLOAT(hum));
 
  #endif

  nrf_saadc_value_t adc_val;

  nrfx_saadc_sample_convert(0, &adc_val);
  NRF_LOG_INFO("ADC Value: %d", adc_val);

  voltage_photo = (voltage_supply / 1024.0f) * adc_val;
  current_photo = voltage_photo / photo_res_val;

  lux_val = (current_photo / current_sun) * lux_sun;
  NRF_LOG_INFO("Lux: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(lux_val));
  
  sensor_data.tempe_data.tempe_val = temp;
  sensor_data.humid_data.humid_val = hum;
  sensor_data.light_data.light_val = lux_val;
  sensor_data.moist_data.moist_val = 50;

  pack_sensor_data(&sensor_data, manu_data);

  err_code = ble_advertising_advdata_update(&m_advertising, &new_advdata, NULL);
  APP_ERROR_CHECK(err_code); 
}

/* Step 2: Init App timer */
void timers_init(void)
{
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, app_timer_handler);
  APP_ERROR_CHECK(err_code);


}

void start_timer(void)
{
  uint32_t err_code = app_timer_start(m_app_timer_id, LED_INTERVAL, NULL);
}