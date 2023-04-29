#include <stdint.h>
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"


void saadc_callback_handler(nrf_drv_saadc_evt_t const *p_event)
{

}

void saadc_init(void)
{
  ret_code_t err_code;

  //nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

  #if 1
  nrf_saadc_channel_config_t channel_config = 
  {                                                   
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,      
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,      
    .gain       = NRF_SAADC_GAIN1_4,                
    .reference  = NRF_SAADC_REFERENCE_VDD4,     
    .acq_time   = NRF_SAADC_ACQTIME_10US,           
    .mode       = NRF_SAADC_MODE_SINGLE_ENDED,      
    .burst      = NRF_SAADC_BURST_DISABLED,         
    .pin_p      = (nrf_saadc_input_t)(NRF_SAADC_INPUT_AIN0),    
    .pin_n      = NRF_SAADC_INPUT_DISABLED 
  };
  #endif

  err_code = nrf_drv_saadc_init(NULL, saadc_callback_handler);
  APP_ERROR_CHECK(err_code);

  err_code = nrfx_saadc_channel_init(0, &channel_config);
  APP_ERROR_CHECK(err_code);
}