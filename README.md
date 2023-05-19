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

### Building the Project for ESP32 Gateway

Tools:<br />
SDK: ESP-IDF 5.0<br />

ESP-IDF uses CMake to generate Makefile for the build system.

The project needs the WiFi Credentials and the API Key of Thingspeak channel.
Thus, the user needs to create wifi_credentials.h and api_key.h header files
inside the Projects/esp32/send_ble_data/main folder.

The wifi_credentials.h must contain the following macros:

![image](https://github.com/russelroman/plant_monitor/assets/120916430/1d6fc85a-2d75-48f8-ad15-12c3242637f2)

And the api_key.h must contain the following macros:

![image](https://github.com/russelroman/plant_monitor/assets/120916430/4521980b-c2af-4c92-bbd2-1fa3f38b6f58)

Open the ESP-IDF 5.0 command prompt. Please go to Project/esp32/send_ble_data. And enter the following
command to build the project.

    idf.py build

## Hardware Components

1. NRF52840 Dongle
2. Capacitive Moisture Sensor v1.2
3. Adafruit Sensirion SHTC3 Temperature & Humidity Sensor
4. Adafruit ALS-PT19 Analog Light Sensor Breakout
5. ESP32 DevKit-C





















