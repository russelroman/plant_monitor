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

#include "nrf_delay.h"

#include "ble_stack.h"


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


/* Step 1: Init the Logger */
static void log_init()
{
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
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
  advertising_init();

  temp_hum_init();
  saadc_init();

  set_random_static_address();

  advertising_start();

  get_random_static_address();

  start_timer();

  NRF_LOG_INFO("BLE APP STARTED..");

  while(1)
  {
    idle_state_handle();
  }
}







































