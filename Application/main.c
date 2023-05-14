#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "ble_stack.h"
#include "temp_hum.h"
#include "light.h"


static void idle_state_handle(void)
{
  if(NRF_LOG_PROCESS() == false)
  {
    nrf_pwr_mgmt_run();
  }
}


static void power_mgmt_init(void)
{
  ret_code_t err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}


static void leds_init(void)
{
  ret_code_t err_code = bsp_init(BSP_INIT_LEDS, NULL);
  APP_ERROR_CHECK(err_code);
}


static void log_init()
{
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}


int main()
{
  log_init();
  timers_init();
  leds_init();
  power_mgmt_init();
  temp_hum_init();
  set_random_static_address();
  ble_stack_init();
  advertising_init();
  get_random_static_address();
  advertising_start();
  start_timer();
  
  NRF_LOG_INFO("BLE APP STARTED..");

  while(1)
  {
    idle_state_handle();
  }
}







































