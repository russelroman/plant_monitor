#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_ble_gatt.h"

#include "ble.h"

#define APP_BLE_CONN_CFG_TAG 1U
#define APP_BLE_OBSERVER_PRIO 3U

#define DEVICE_NAME         "x-parasite"


/* Set MIN_CONN_INTERVAL to 100ms 
   Soft device uses RTC0 which has minimum time of 1.25ms.
   Thus, we need to convert 100ms according to unit of 1.25ms.
*/
#define MIN_CONN_INTERVAL   MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL   MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY       0
#define CONN_SUP_TIMEOUT    MSEC_TO_UNITS(2000, UNIT_10_MS) // Timeout after 2 seconds.

NRF_BLE_GATT_DEF(m_gatt);


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

  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
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