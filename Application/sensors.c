#include <stdint.h>
#include "sensors.h"


enum sensor_type
{
  TEMPE_TYPE = 1,
  HUMID_TYPE = 2,
  LIGHT_TYPE = 3,
  MOISTURE_TYPE = 4
};

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

