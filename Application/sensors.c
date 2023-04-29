#include <stdint.h>
#include "sensors.h"
#include "temp_hum.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_saadc.h"

enum sensor_type
{
  TEMPE_TYPE = 1,
  HUMID_TYPE = 2,
  LIGHT_TYPE = 3,
  MOISTURE_TYPE = 4
};

float lux_val = 0.0f;
float current_photo = 0;
float voltage_photo = 0;
float ref_voltage = 3.0 / 4;
const float lux_sun = 10000.0f;
const float current_sun = 3.59e-3f;
const float photo_res_val = 470;
const float scaling = 4;

static sensor_data_t sensor_data;


float tempe_read(void)
{
  return sensor_data.tempe_data.tempe_val;
}

void tempe_write(float temperature)
{
  sensor_data.tempe_data.tempe_val = temperature; 
}

float humid_read(void)
{
  return sensor_data.humid_data.humid_val;
}

void humid_write(float humidity)
{
  sensor_data.humid_data.humid_val = humidity;
}

float light_read(void)
{
  return sensor_data.light_data.light_val;
}

void light_write(float light)
{
  sensor_data.light_data.light_val = light; 
}

float moist_read(void)
{
  return sensor_data.moist_data.moist_val;
}

void moist_write(float moisture)
{
  sensor_data.moist_data.moist_val = moisture; 
}


int pack_sensor_data(uint8_t *ble_manuf_data)
{
  uint16_t tempe_data;
  uint16_t humid_data;
  uint16_t light_data;
  uint16_t moist_data;

  // Check if size of the container is enough

  tempe_data = sensor_data.tempe_data.tempe_val * 100U;
  humid_data = sensor_data.humid_data.humid_val * 100U;
  moist_data = sensor_data.moist_data.moist_val * 100U;
  light_data = sensor_data.light_data.light_val;

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



void sensor_data_update(void)
{
  float temp = 30.0f;
  float hum = 60.0f;

  read_data_shtc(&temp, &hum);
  NRF_LOG_INFO("Temp: " NRF_LOG_FLOAT_MARKER " C", NRF_LOG_FLOAT(temp));
  NRF_LOG_INFO("Hum: " NRF_LOG_FLOAT_MARKER " %%", NRF_LOG_FLOAT(hum));

  nrf_saadc_value_t adc_val;

  nrfx_saadc_sample_convert(0, &adc_val);
  NRF_LOG_INFO("ADC Value: %d", adc_val);

  if(adc_val < 0)
  {
    adc_val = 0;
  }

  voltage_photo = (ref_voltage / 1024.0f) * adc_val * scaling;
  current_photo = voltage_photo / photo_res_val;

  lux_val = (current_photo / current_sun) * lux_sun;
  NRF_LOG_INFO("Lux: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(lux_val));

  float moist_val;

  tempe_write(temp);
  humid_write(hum);
  light_write(lux_val);
  moist_write(50);
}





