#include <stdint.h>
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"


void saadc_callback_handler(nrf_drv_saadc_evt_t const *p_event)
{

}

void saadc_init(void)
{
  ret_code_t err_code;

  nrf_saadc_channel_config_t channel_config_light = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
  nrf_saadc_channel_config_t channel_config_bat = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
  nrf_saadc_channel_config_t channel_config_soil = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN5);

  err_code = nrf_drv_saadc_init(NULL, saadc_callback_handler);
  APP_ERROR_CHECK(err_code);

  err_code = nrfx_saadc_channel_init(0, &channel_config_light);
  APP_ERROR_CHECK(err_code);

  err_code = nrfx_saadc_channel_init(1, &channel_config_bat);
  APP_ERROR_CHECK(err_code);

  err_code = nrfx_saadc_channel_init(5, &channel_config_soil);
  APP_ERROR_CHECK(err_code);
}