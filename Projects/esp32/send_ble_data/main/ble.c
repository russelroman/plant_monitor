#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#include "esp_log.h"

#include "sensor.h"
#include "http_request.h"


extern sensor_val_t sensor_val;

uint8_t ble_addr_type;
void ble_app_scan(void);

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_hs_adv_fields fields;

    switch (event->type)
    {
    // NimBLE event discovery
    case BLE_GAP_EVENT_DISC:

        ESP_LOGI("GAP", "GAP EVENT DISCOVERY");
        ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);

        if (fields.name_len > 0)
        {
            printf("Name: %.*s\n", fields.name_len, fields.name);
        }

        const uint8_t * man_data = fields.mfg_data;

        // Will be NULL in scan response because manufactuing data is empy in scan response
        if(man_data != (void*)NULL)
        {
            for(int i = 0; i < fields.mfg_data_len - 2; ++i)
            {
                printf("%02X ", man_data[i + 2]);
            }

            sensor_val.temp_val = ((uint16_t)man_data[2 + 2] << 8U) | man_data[3 + 2];
            sensor_val.temp_val = sensor_val.temp_val / 100;
            printf("Temp is %4.2f\n", sensor_val.temp_val);

            sensor_val.hum_val = ((uint16_t)man_data[2 + 6] << 8U) | man_data[7 + 2];
            sensor_val.hum_val = sensor_val.hum_val / 100;
            printf("Hum is %4.2f\n", sensor_val.hum_val);

            sensor_val.light_val = ((uint16_t)man_data[2 + 10] << 8U) | man_data[11 + 2];

            printf("Light is %d\n", sensor_val.light_val);
            printf("Light val is %02X and %02X \n", man_data[2 + 10],  man_data[11 + 2]);

            sensor_val.moist_val = ((uint16_t)man_data[2 + 14] << 8U) | man_data[15 + 2];
            sensor_val.moist_val = sensor_val.moist_val / 100;
            printf("Moisture is %4.2f\n", sensor_val.moist_val);

            http_get_task();
        }
        else
        {
            printf("NULL\n");
        }


        

    break;

    default:
        break;
    }
    return 0;
}

void ble_app_scan(void)
{
    printf("Start scanning ...\n");

    struct ble_gap_disc_params disc_params;
    disc_params.filter_duplicates = 0;
    disc_params.passive = 0;
    disc_params.itvl = 0;
    disc_params.window = 0;
    disc_params.filter_policy = 0;
    disc_params.limited = 0;

    ble_gap_disc(ble_addr_type, BLE_HS_FOREVER, &disc_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_scan();                          
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}


void ble_init()
{
    nimble_port_init();                             // 3 - Initialize the controller stack
    ble_svc_gap_device_name_set("BLE-Scan-Client"); // 4 - Set device name characteristic
    ble_svc_gap_init();                             // 4 - Initialize GAP service
    ble_hs_cfg.sync_cb = ble_app_on_sync;           // 5 - Set application
    nimble_port_freertos_init(host_task);           // 6 - Set infinite task
}

