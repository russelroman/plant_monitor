#include <stdint.h>
#include "nrf_drv_twi.h"
#include "sdk_config.h"
#include "nrf_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_delay.h"

/* TWI instance ID. */
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

 /* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127

#define ARDUINO_SCL_PIN             NRF_GPIO_PIN_MAP(1, 10)    // SCL signal pin
#define ARDUINO_SDA_PIN             NRF_GPIO_PIN_MAP(1, 13)    // SDA signal pin

const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
uint8_t sample_data[6];

void temp_hum_init(void)
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

int read_data_shtc(float *temperature, float *humidity)
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

  nrf_drv_twi_tx(&m_twi, address, sleep_command, sizeof(sleep_command), false);

  // TODO: Verify checksum

  *temperature = -45 + (175.0f)*((sample_data[0] << 8U) | sample_data[1]) / (65536);
  *humidity = (100.0f)*((sample_data[3] << 8U) | sample_data[4]) / (65536);

  return 0; // TODO: Return Error
}