#include <stdio.h>
#include <stdint.h>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include "sensor.h"
#include "api_key.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT "80"
#define WEB_PATH "/"

static const char *TAG = "example";

char http_request_buf[1024];
char temp_str[6] = "37.55";
    
typedef struct 
{
    char * URI;
    char * protocol;
    char * web_server;
    char * port;
} get_request_t;


char sensor_val_string[4][6];

enum 
{
    TEMP_VAL_INDEX,
    HUM_VAL_INDEX,
    LIGHT_VAL_INDEX,
    MOIST_VAL_INDEX,
};


sensor_val_t sensor_val;


int float_to_string(char *sensor_val_string, float sensor_val)
{
    uint8_t int_part;
    uint8_t frac_part;

    int_part = (int)sensor_val;
    frac_part = (sensor_val - int_part) * 100U;

    sensor_val_string[0] = int_part / 10 + '0';
    sensor_val_string[1] = int_part % 10 + '0';
    sensor_val_string[2] =  '.';
    sensor_val_string[3] = frac_part / 10 + '0';
    sensor_val_string[4] = frac_part % 10 + '0';
    sensor_val_string[5] = '\0';

    return 0;
}

int uint16_to_string(char *sensor_val_string, uint16_t sensor_val)
{
    sensor_val_string[0] = sensor_val / 10000 + '0';
    sensor_val_string[1] = (sensor_val % 10000) / 1000 + '0';
    sensor_val_string[2] = (sensor_val % 1000) / 100 + '0';
    sensor_val_string[3] = (sensor_val % 100) / 10 + '0';
    sensor_val_string[4] = (sensor_val % 10) + '0';
    sensor_val_string[5] = '\0';

    return 0;
}



int build_http_get_request(char * p_request, get_request_t * p_get_request)
{
    // Request Line
    strcat(p_request, "GET ");
    strcat(p_request, p_get_request->URI);
    strcat(p_request, " ");
    strcat(p_request, p_get_request->protocol);
    strcat(p_request, "\r\n");

    // Header
    strcat(p_request, "Host: ");
    strcat(p_request, p_get_request->web_server);
    strcat(p_request, ":");
    strcat(p_request, p_get_request->port);
    strcat(p_request, "\r\n\r\n");

    return 1;
}

void http_get_task(void)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

    if(err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    else
    {

    }

    /* Code to print the resolved IP.
       Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        ESP_LOGE(TAG, "... Failed to allocate socket.");
        freeaddrinfo(res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    else
    {
        ESP_LOGI(TAG, "... allocated socket");
    }

    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
        close(s);
        freeaddrinfo(res);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
    else
    {
        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);
    }

    for(int i = 0; i < MOIST_VAL_INDEX + 1; ++i)
    {
        if(i == 0)
        {
            float_to_string(sensor_val_string[TEMP_VAL_INDEX], sensor_val.temp_val);
        }
        else if (i == 1)
        {
            float_to_string(sensor_val_string[HUM_VAL_INDEX], sensor_val.hum_val);
        }
        else if (i == 2)
        {
            uint16_to_string(sensor_val_string[LIGHT_VAL_INDEX], sensor_val.light_val);
        }
        else if (i == 3)
        {
            float_to_string(sensor_val_string[MOIST_VAL_INDEX], sensor_val.moist_val);
        }
        else
        {
            
        }
    }

    char URI[200] = UPDATE_KEY;
    strcat(URI, "&field1=");
    strcat(URI, sensor_val_string[TEMP_VAL_INDEX]);
    strcat(URI, "&field2=");
    strcat(URI, sensor_val_string[HUM_VAL_INDEX]);
    strcat(URI, "&field3=");
    strcat(URI, sensor_val_string[LIGHT_VAL_INDEX]);
    strcat(URI, "&field4=");
    strcat(URI, sensor_val_string[MOIST_VAL_INDEX]);

    char protocol[] = "HTTP/1.1";
    char web_server[] = "api.thingspeak.com";
    char port[] = "80";

    get_request_t get_request;

    get_request.URI = URI;
    get_request.protocol = protocol;
    get_request.web_server = web_server;
    get_request.port = port;

    build_http_get_request(http_request_buf, &get_request);

    if (write(s, http_request_buf, strlen(http_request_buf)) < 0) {
        ESP_LOGE(TAG, "... socket send failed");
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
    else
    {
        ESP_LOGI(TAG, "... socket send success");
    }

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;

    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
            sizeof(receiving_timeout)) < 0) {
        ESP_LOGE(TAG, "... failed to set socket receiving timeout");
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
    else
    {
        ESP_LOGI(TAG, "... set socket receiving timeout success");
    }

    /* Read HTTP response */
    do {
        bzero(recv_buf, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf)-1);
        for(int i = 0; i < r; i++) {
            putchar(recv_buf[i]);
        }
    } while(r > 0);

    ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
}