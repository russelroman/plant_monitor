#include <stdint.h>
#include "sensors.h"
#include "temp_hum.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_saadc.h"
#include "light.h"

enum sensor_type
{
  TEMPE_TYPE = 1,
  HUMID_TYPE = 2,
  LIGHT_TYPE = 3,
  MOISTURE_TYPE = 4
};


/*
  10,000 Lux = Max Ambient Sunlight
  The Collector Current is 3.6mA at 10,000 Lux.
  Using 450 ohms emitter resitor, the adc value
  is 461. And emitter voltage is 1.62 V
*/
const uint16_t sun_lux = 10000;

const float scaling = 6;
const float ref_voltage = 0.6;  

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

static inline float get_soil_moisture_percent(float battery_voltage,
                                              int16_t raw_adc_output) {
  const double x = battery_voltage;
  const double dry = -11.7f * x * x + 101.0f * x + 306.0f;
  const double wet = 3.42f * x * x - 4.98f * x + 19.0f;
  const float percent = (raw_adc_output - dry) / (wet - dry);
  
  return percent;
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

  ret_code_t err_code;

  read_data_shtc(&temp, &hum);
  NRF_LOG_INFO("Temp: " NRF_LOG_FLOAT_MARKER " C", NRF_LOG_FLOAT(temp));
  NRF_LOG_INFO("Hum: " NRF_LOG_FLOAT_MARKER " %%", NRF_LOG_FLOAT(hum));

  nrf_saadc_value_t light_adc_val;
  nrf_saadc_value_t bat_adc_val;
  nrf_saadc_value_t soil_adc_val;

  saadc_init();

  /* Light Sensor */

  nrfx_saadc_sample_convert(0, &light_adc_val);
  NRF_LOG_INFO("Light ADC Value: %d", light_adc_val);

  if(light_adc_val < 0)
  {
    light_adc_val = 0;
  }

  float lux_val = 0;
  float voltage_photo = 0;

  voltage_photo = (ref_voltage / 1024.0f) * light_adc_val * scaling;
  lux_val = (voltage_photo / 1.62f) * sun_lux;

  NRF_LOG_INFO("Lux: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(lux_val));
  
  /* Battery */

  float battery_voltage = 3.2;

  nrfx_saadc_sample_convert(1, &bat_adc_val);
  NRF_LOG_INFO("Battery ADC: %d", bat_adc_val);

  battery_voltage = bat_adc_val * (ref_voltage / 1024) * scaling;
  NRF_LOG_INFO("Battery Voltage: " NRF_LOG_FLOAT_MARKER "V", NRF_LOG_FLOAT(battery_voltage));

  /* Moisture */

  float soil_moisture;

  nrfx_saadc_sample_convert(5, &soil_adc_val);
  NRF_LOG_INFO("Moisture ADC: %d", soil_adc_val);

  if(soil_adc_val < 0)
  {
    soil_adc_val = 0;
  }

  soil_moisture = get_soil_moisture_percent(battery_voltage, soil_adc_val);

  if(soil_moisture < 0)
  {
    soil_moisture = 0;
  }
  NRF_LOG_INFO("Soil Moisture: " NRF_LOG_FLOAT_MARKER " %%", NRF_LOG_FLOAT(soil_moisture));

  nrf_drv_saadc_uninit();

  tempe_write(temp);
  humid_write(hum);
  light_write(lux_val);
  moist_write(soil_moisture);
}





