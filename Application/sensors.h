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


#endif