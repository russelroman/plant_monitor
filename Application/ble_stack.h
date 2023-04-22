#ifndef  BLE_H
#define  BLE_H

void ble_stack_init(void);
void gap_params_init(void);
void gatt_init(void);
void conn_params_init(void);
void services_init(void);
void advertising_init(void);
void advertising_start(void);
void start_timer(void);
void timers_init(void);

#endif