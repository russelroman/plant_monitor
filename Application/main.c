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
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

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
  NRF_LOG_FLUSH();

  nrf_drv_twi_tx(&m_twi, address, wakeup_command, sizeof(wakeup_command), false);
  nrf_delay_ms(1);

  nrf_drv_twi_tx(&m_twi, address, measure_command, sizeof(measure_command), false);
  nrf_delay_ms(20);
  nrf_drv_twi_rx(&m_twi, address, sample_data, sizeof(sample_data));

  // TODO: Verify checksum

  *temperature = -45 + (175.0f)*(sample_data[0] << 8U) | sample_data[1]) / (65536);
  *humidity = (100.0f)*((sample_data[3] << 8U) | sample_data[4]) / (65536);

  return 0; // TODO: Return Error
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
   ret_code_t err_code;
    uint8_t address;
    uint8_t sample_data;
    bool detected_device = false;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("TWI scanner started.");
    NRF_LOG_FLUSH();
    twi_init();

    float temp;
    float hum;

     #if 0
    read_data_shtc(&temp, &hum);
    NRF_LOG_INFO("Temp is " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(temp));
    NRF_LOG_INFO("Hum " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(hum));
    NRF_LOG_FLUSH();
    #endif

    while (true)
    {
        /* Empty loop. */
         read_data_shtc(&temp, &hum);
         NRF_LOG_INFO("Temp is " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(temp));
         NRF_LOG_INFO("Hum " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(hum));
         NRF_LOG_FLUSH();

         nrf_delay_ms(1000);
    }
}

/**
 *@}
 **/
