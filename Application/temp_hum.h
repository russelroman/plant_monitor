#ifndef  SENSORS_H
#define  SENSORS_H

#include <stdint.h>

void temp_hum_init(void);
int read_data_shtc(float *temperature, float *humidity);

#endif