#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "abstcp-v4/server.h"

static const char *TAG = "abstcp-v4-server";

// Default keepalive settings
#define DEFAULT_KEEPALIVE_IDLE     7200
#define DEFAULT_KEEPALIVE_INTERVAL 75
#define DEFAULT_KEEPALIVE_COUNT    9

typedef struct {
    uint16_t port;
    const char *host;
    response_func_t response_callback;
    void *user_data;
    int keepalive_idle;
    int keepalive_interval;
    int keepalive_count;
    int max_connections;
} server_config_t;

static void handle_client_connection(int sock, response_func_t response_callback, void *user_data)
{
    char rx_buffer[1024];
    int len;

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            break;
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed by client");
            break;
        } else {
            rx_buffer[len] = 0; // Null-terminate received data
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            // Call the response callback function
            if (response_callback) {
                char response_buffer[1024];
                int response_len = response_callback(rx_buffer, len, response_buffer, sizeof(response_buffer), user_data);
                
                if (response_len > 0) {
                    // Send response back to client
                    int to_write = response_len;
                    while (to_write > 0) {
                        int written = send(sock, response_buffer + (response_len - to_write), to_write, 0);
                        if (written < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            return;
                        }
                        to_write -= written;
                    }
                    ESP_LOGI(TAG, "Sent %d bytes response", response_len);
                }
            }
        }
    } while (len > 0);
}

static void tcp_server_task(void *pvParameters)
{
    server_config_t *config = (server_config_t *)pvParameters;
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    struct sockaddr_storage dest_addr;

    // Configure IPv4 address
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    if (config->host && strlen(config->host) > 0) {
        inet_pton(AF_INET, config->host, &dest_addr_ip4->sin_addr);
    } else {
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    }
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(config->port);

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", config->port);

    err = listen(listen_sock, config->max_connections);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {
        ESP_LOGI(TAG, "Socket listening on port %d", config->port);

        struct sockaddr_storage source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set TCP keepalive options
        int keepAlive = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &config->keepalive_idle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &config->keepalive_interval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &config->keepalive_count, sizeof(int));
        
        // Convert client IP address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted from IP address: %s", addr_str);

        handle_client_connection(sock, config->response_callback, config->user_data);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

int tcp_server_start(uint16_t port, const char *host, response_func_t response_callback, void *user_data, server_options_t *options)
{
    server_config_t *config = malloc(sizeof(server_config_t));
    if (!config) {
        ESP_LOGE(TAG, "Failed to allocate memory for server config");
        return -1;
    }

    config->port = port;
    config->host = host;
    config->response_callback = response_callback;
    config->user_data = user_data;
    
    // Apply options or use defaults
    if (options) {
        config->keepalive_idle = options->keepalive_idle > 0 ? options->keepalive_idle : DEFAULT_KEEPALIVE_IDLE;
        config->keepalive_interval = options->keepalive_interval > 0 ? options->keepalive_interval : DEFAULT_KEEPALIVE_INTERVAL;
        config->keepalive_count = options->keepalive_count > 0 ? options->keepalive_count : DEFAULT_KEEPALIVE_COUNT;
        config->max_connections = options->max_connections > 0 ? options->max_connections : 1;
    } else {
        config->keepalive_idle = DEFAULT_KEEPALIVE_IDLE;
        config->keepalive_interval = DEFAULT_KEEPALIVE_INTERVAL;
        config->keepalive_count = DEFAULT_KEEPALIVE_COUNT;
        config->max_connections = 1;
    }

    // Create server task
    BaseType_t result = xTaskCreate(tcp_server_task, "tcp_server", 4096, config, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create TCP server task");
        free(config);
        return -1;
    }

    ESP_LOGI(TAG, "TCP server started on %s:%d", host ? host : "0.0.0.0", port);
    return 0;
}
