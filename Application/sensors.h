#ifndef  SENSORS_H
#define  SENSORS_H

#include <stdint.h>

typedef struct
{
  float tempe_val;
} tempe_data_t;

typedef struct
{
  float humid_val;
} humid_data_t;

typedef struct
{
  uint16_t light_val;
} light_data_t;

typedef struct
{
  float moist_val;
} moist_data_t;

typedef struct
{
  tempe_data_t tempe_data;
  humid_data_t humid_data;
  light_data_t light_data;
  moist_data_t moist_data;
} sensor_data_t;

//sensor_data_t * sensor_data_get(void);

float tempe_read(void);
void tempe_write(float temperature);
float humid_read(void);
void humid_write(float humidity);
float light_read(void);
void light_write(float light);
float moist_read(void);
void moist_write(float moisture);
int pack_sensor_data(uint8_t *ble_manuf_data);
void sensor_data_update(void);

#endif