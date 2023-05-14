#include "wifi.h"
#include "sensor.h"
#include "http_request.h"
#include "ble.h"
#include "flash.h"

void app_main(void)
{
    flash_init();
    wifi_init_sta();
    ble_init();
}
