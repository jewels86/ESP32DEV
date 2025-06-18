#include "abstract.h"
#include "abswifi.h"

static const char* TAG = "wifi";
static EventGroupHandle_t wifi_event_group;

/// @brief Handles WiFi and IP events for the ESP32 WiFi station.
/// 
/// This function is registered as an event handler for WiFi and IP events.
/// It manages connection attempts, reconnections, and logs status messages.
/// 
/// - On WiFi start: attempts to connect to the configured WiFi network.
/// - On WiFi disconnect: attempts to reconnect and clears the connection bit.
/// - On IP acquisition: logs the assigned IP address and sets the connection bit.
/// 
/// @param arg User-defined argument (unused).
/// @param event_base The base identifier of the event (e.g., WIFI_EVENT, IP_EVENT).
/// @param event_id The specific event ID (e.g., WIFI_EVENT_STA_START).
/// @param event_data Pointer to event-specific data.
/// @param log Function pointer for logging status messages.
static void event_handler(void *arg, esp_event_base_t event_base, 
    int32_t event_id, void* event_data, void (*log)(char* msg)) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        log("WiFi disconnected, trying to reconnect...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char ip_msg[64];
        snprintf(ip_msg, sizeof(ip_msg), "WiFi connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        log(ip_msg);
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_manager_start(const char *ssid, const char *pass) {
    wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);

    wifi_config_t wifi_cfg = {};
    strncpy((char *)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid));
    strncpy((char *)wifi_cfg.sta.password, pass, sizeof(wifi_cfg.sta.password));
    wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    esp_wifi_start();
}

void wifi_manager_wait_for_connection(void) {
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}
