# Plant Monitor

This project is inspired by b-parasite project by rbaron (https://github.com/rbaron/b-parasite).
My goal in this project is to learn HTTP and BLE by modifying b-parasite based on my learning
needs.

Plant Monitor will get the soil moisture, illuminance, temperature and humidity of the environment.
The device will transmit sensor data to ESP32 Gateway through BLE packets. Then the gateway will transmit
the sensor data to ThingSpeak for monitoring and analysis through HTTP Requests.

## Building the Project

The project uses the NRF SDK from Nordic Semiconductors for the BLE device.
The project also uses the ESP-IDF from Espressif for the Gateway.

### Building the Project for BLE Device

![image](https://github.com/russelroman/plant_monitor/assets/120916430/82d1668f-ca5b-4701-bfe8-7463cd3cca00)

Tools:<br />
SDK: NRF SDK 17.1.0<br />
IDE: Segger Embedded Studio Release 5.62<br />

The project stucture looks like the image.

The Middleware folder contains the NRF SDK. The user must include the SDK
as it is not included on the repository. Please follow the following structure.

Middleware
---> nrf
--------> nrf_sdk_xx

The nrf_sdk_xx folder will contain the SDK.
Go to Projects -> nrf -> ses and open plant_monitor.emProject. Click Build.







