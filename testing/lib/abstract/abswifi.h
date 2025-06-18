#ifndef ABSWIFI_H
#define ABSWIFI_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define WIFI_CONNECTED_BIT BIT0

static char* wifi_ssid = "your_wifi_ssid";
static char* wifi_password = "your_wifi_password";



#endif // ABSWIFI_H