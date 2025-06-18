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
static void wifi_event_handler(void *arg, esp_event_base_t event_base, 
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

void wifi_init_sta(void (*log)(char* msg)) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    wifi_config_t wifi_config
}