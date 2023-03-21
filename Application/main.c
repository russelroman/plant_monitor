/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_timer.h"

#include "bsp_btn_ble.h"

#include "nrf_pwr_mgmt.h"

#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"

#include "nrf_ble_qwr.h"

#include "nrf_ble_gatt.h"

#include "ble_advdata.h"
#include "ble_advertising.h"

#include "ble_conn_params.h"

#include "nrf_drv_twi.h"
#include "nrf_delay.h"

/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127

const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
uint8_t sample_data[6];

void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

static int read_data_shtc(float *temperature, float *humidity)
{

  uint8_t address = 0x70;
  uint8_t wakeup_command[2] = {0x35, 0x17};
  uint8_t sleep_command[2] = {0xB0, 0x98};
  uint8_t measure_command[2] = {0x78, 0x66};
  
  NRF_LOG_INFO("Read temp");
 // NRF_LOG_FLUSH();

  nrf_drv_twi_tx(&m_twi, address, wakeup_command, sizeof(wakeup_command), false);
  nrf_delay_us(250);  // Wakeup Time based in datasheet.

  nrf_drv_twi_tx(&m_twi, address, measure_command, sizeof(measure_command), false);
  nrf_delay_ms(20);  // Measure Time for Normal Mode.
  nrf_drv_twi_rx(&m_twi, address, sample_data, sizeof(sample_data));

  // TODO: Verify checksum

  *temperature = -45 + (175.0f)*((sample_data[0] << 8U) | sample_data[1]) / (65536);
  *humidity = (100.0f)*((sample_data[3] << 8U) | sample_data[4]) / (65536);

  return 0; // TODO: Return Error
}

#define APP_BLE_CONN_CFG_TAG 1U
#define APP_BLE_OBSERVER_PRIO 3U

#define APP_ADV_INTERVAL  300
#define APP_ADV_DURATION  0   // No Timeout

/* Constants for GAP Service */
#define DEVICE_NAME         "x-parasite"

/* Set MIN_CONN_INTERVAL to 100ms 
   Soft device uses RTC0 which has minimum time of 1.25ms.
   Thus, we need to convert 100ms according to unit of 1.25ms.
*/
#define MIN_CONN_INTERVAL   MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL   MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY       0
#define CONN_SUP_TIMEOUT    MSEC_TO_UNITS(2000, UNIT_10_MS) // Timeout after 2 seconds.

#define FIRST_CONN_PARMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)
#define NEXT_CONN_PARMS_UPDATE_DELAY    APP_TIMER_TICKS(30000)
#define MAX_CONN_PARMS_UPDATE_COUNT     3U

#define LED_INTERVAL APP_TIMER_TICKS(100)



NRF_BLE_QWR_DEF(m_qwr); // Use QWRS if connecting with multiple devices
NRF_BLE_GATT_DEF(m_gatt);
BLE_ADVERTISING_DEF(m_advertising);


APP_TIMER_DEF(m_app_timer_id);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;


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


/* Step 10 Setting connection parameters*/
static void conn_params_init(void)
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
static void services_init(void)
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

typedef struct
{
  float tempe_val;
} tempe_data_t;

typedef struct
{
  float humid_val;
} humid_data_t;

typedef struct
{
  uint16_t light_val;
} light_data_t;

typedef struct
{
  float moist_val;
} moist_data_t;

typedef struct
{
  tempe_data_t tempe_data;
  humid_data_t humid_data;
  light_data_t light_data;
  moist_data_t moist_data;
} sensor_data_t;

enum sensor_type
{
  TEMPE_TYPE = 1,
  HUMID_TYPE = 2,
  LIGHT_TYPE = 3,
  MOISTURE_TYPE = 4
};


uint16_t float_to_uint16(float num_float)
{
  uint16_t num_uint16;
  uint8_t int_part;
  uint8_t frac_part;

  int_part = (int)num_float;
  frac_part = (num_float - int_part) * 100U;

  num_uint16 = ((uint16_t)int_part << 8U) | frac_part;

  return num_uint16;
}

/*
  Brief: Packs sensor data for BLE transmission
  Arguments: 
    Sensor data struct
    Pointer to the array container
  Return:
    Success/Fail Value
  Precondition:
  Post-condition:
    Pointer to the array container will contain the packed sensor data
*/
int pack_sensor_data(sensor_data_t *sensor_data, uint8_t *ble_manuf_data)
{
  uint16_t tempe_data;
  uint16_t humid_data;
  uint16_t light_data;
  uint16_t moist_data;

  // Check if size of the container is enough

  // Convert the float data from sensors to uint16
  tempe_data = float_to_uint16(sensor_data->tempe_data.tempe_val);
  humid_data = float_to_uint16(sensor_data->humid_data.humid_val);
  moist_data = float_to_uint16(sensor_data->moist_data.moist_val);
  light_data = sensor_data->light_data.light_val;

  // Little-Endian Order
  // Temperature TLV
  ble_manuf_data[0] = TEMPE_TYPE;
  ble_manuf_data[1] = 2;
  ble_manuf_data[2] = (uint8_t)((tempe_data & 0xFF00)>> 8U);
  ble_manuf_data[3] = (uint8_t)(tempe_data & 0x00FF);

  ble_manuf_data[4] = HUMID_TYPE;
  ble_manuf_data[5] = 2;
  ble_manuf_data[6] = (uint8_t)((humid_data & 0xFF00)>> 8U);
  ble_manuf_data[7] = (uint8_t)(humid_data & 0x00FF);

  ble_manuf_data[8] = LIGHT_TYPE;
  ble_manuf_data[9] = 2;
  ble_manuf_data[10] = (uint8_t)((light_data & 0xFF00)>> 8U);
  ble_manuf_data[11] = (uint8_t)(light_data & 0x00FF);

  ble_manuf_data[12] = MOISTURE_TYPE;
  ble_manuf_data[13] = 2;
  ble_manuf_data[14] = (uint8_t)((moist_data & 0xFF00)>> 8U);
  ble_manuf_data[15] = (uint8_t)(moist_data & 0x00FF);
}


sensor_data_t sensor_data;

/* Step 8 */
static void advertising_init(void)
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

  // Scan response
  init.srdata.name_type = BLE_ADVDATA_FULL_NAME;

  
  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}



/* Step 7 */
static void gatt_init(void)
{
  ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL); // Passing NULL will not handle GATT Events
  APP_ERROR_CHECK(err_code);
}


/* Step 6 */
static void gap_params_init(void)
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


/* Step 5.1 : BLE Event handler */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
  ret_code_t err_code = NRF_SUCCESS;

  switch(p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_DISCONNECTED:

      NRF_LOG_INFO("Device is Disconnected!!!");
      
      break;

    case BLE_GAP_EVT_CONNECTED:

      NRF_LOG_INFO("Device is Connected!!!");

      err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
     
      APP_ERROR_CHECK(err_code);

      m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

      err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
      APP_ERROR_CHECK(err_code);

      break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:

      NRF_LOG_INFO("PHY Update Request!!!");

      ble_gap_phys_t const phys = 
      {
        .rx_phys = BLE_GAP_PHY_AUTO,
        .tx_phys = BLE_GAP_PHY_AUTO
      };

      err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);

      break;
  }
  
}


/* Step 5: BLE Stack Init */
static void ble_stack_init(void)
{
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  uint32_t ram_start = 0U;

  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/* Step 4.1: Idle State Handle */
static void idle_state_handle(void)
{
  if(NRF_LOG_PROCESS() == false)
  {
    nrf_pwr_mgmt_run();
  }
}

/* Step 4: Power Management */
static void power_mgmt_init(void)
{
  ret_code_t err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}


/* Step 3: BPS Init (LEDS) */
static void leds_init(void)
{
  ret_code_t err_code = bsp_init(BSP_INIT_LEDS, NULL);
  APP_ERROR_CHECK(err_code);
}


ble_advdata_t new_advdata;
ble_advdata_t new_srdata;

static void app_timer_handler(void *p_context)
{
  ret_code_t err_code;
  NRF_LOG_INFO("APP TIMER");
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

  read_data_shtc(&temp, &hum);
  NRF_LOG_INFO("Temp: " NRF_LOG_FLOAT_MARKER " C", NRF_LOG_FLOAT(temp));
  NRF_LOG_INFO("Hum: " NRF_LOG_FLOAT_MARKER " %%", NRF_LOG_FLOAT(hum));
 
  #endif
  
  static uint32_t i = 0;
  ++i;
    sensor_data.tempe_data.tempe_val = 37.57f + i;
    sensor_data.humid_data.humid_val = 37.57f + i;
    sensor_data.light_data.light_val = 5200 + i;
    sensor_data.moist_data.moist_val = 37.57f + i;

  pack_sensor_data(&sensor_data, manu_data);

  err_code = ble_advertising_advdata_update(&m_advertising, &new_advdata, &new_srdata);
  APP_ERROR_CHECK(err_code); 
}

/* Step 2: Init App timer */
static void timers_init(void)
{
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, app_timer_handler);
  APP_ERROR_CHECK(err_code);


}


/* Step 1: Init the Logger */
static void log_init()
{
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}




/* Step 11 Start Advertisement */
static void advertising_start(void)
{
   ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
   APP_ERROR_CHECK(err_code);
}


static void set_random_static_address(void)
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
    NRF_LOG_INFO("Failed to set random static address")
  }
}

static void get_random_static_address(void)
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


int main()
{
  log_init();
  timers_init();
  leds_init();
  power_mgmt_init();
  ble_stack_init();
  gap_params_init();
  gatt_init();
  advertising_init();
  services_init();
  conn_params_init();

  twi_init();

  NRF_LOG_INFO("BLE APP STARTED..");

  set_random_static_address();

  advertising_start();

  get_random_static_address();

  uint32_t err_code = app_timer_start(m_app_timer_id, LED_INTERVAL, NULL);

  while(1)
  {
    idle_state_handle();
    
    #if 0	
    /* Empty loop. */
    float temp, hum;
         read_data_shtc(&temp, &hum);
         NRF_LOG_INFO("Temp: " NRF_LOG_FLOAT_MARKER " C", NRF_LOG_FLOAT(temp));
         NRF_LOG_INFO("Hum: " NRF_LOG_FLOAT_MARKER " %%", NRF_LOG_FLOAT(hum));
         NRF_LOG_FLUSH();
         nrf_delay_ms(1000);
    #endif
  }
  
	
}







































