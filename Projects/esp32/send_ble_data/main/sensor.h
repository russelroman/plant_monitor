#include <stdint.h>

typedef struct
{
    float temp_val;
    float hum_val;
    uint16_t light_val;
    float moist_val;
} sensor_val_t;