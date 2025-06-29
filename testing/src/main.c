#include "abswifi.h"
#include "absnvs.h"
#include "abstcp-v4/server.h"
#include "abstcp-v4/client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <sys/socket.h>

static const char *TAG = "main";

// Custom recv function for the client
ssize_t client_recv_func(int sockfd, void *buf, size_t len, int flags) {
    return recv(sockfd, buf, len, flags);
}

// Example response function for the TCP server
int echo_response_handler(const char *request_data, int request_len, char *response_buffer, int response_buffer_size, void *user_data) {
    ESP_LOGI(TAG, "Server received: %.*s", request_len, request_data);
    
    // Simple echo response with a prefix
    const char *prefix = "Echo: ";
    int prefix_len = strlen(prefix);
    
    if (prefix_len + request_len < response_buffer_size) {
        strcpy(response_buffer, prefix);
        memcpy(response_buffer + prefix_len, request_data, request_len);
        return prefix_len + request_len;
    }
    
    return -1; // Response too large
}

// Task to demonstrate client functionality
void client_task(void *pvParameters) {
    // Wait a bit for the server to start
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGI(TAG, "Starting TCP client...");
    
    // Create client connection (connecting to our own server)
    send_func_t send_func = client("192.168.4.1", NULL, 8080, client_recv_func);
    
    if (send_func != NULL) {
        ESP_LOGI(TAG, "Client connected successfully");
        
        // Send some test messages
        const char *messages[] = {
            "Hello from client!",
            "This is message 2",
            "Final test message"
        };
        
        for (int i = 0; i < 3; i++) {
            ESP_LOGI(TAG, "Sending message %d: %s", i + 1, messages[i]);
            ssize_t sent = send_func(0, messages[i], strlen(messages[i]), 0);
            
            if (sent > 0) {
                ESP_LOGI(TAG, "Message %d sent successfully (%d bytes)", i + 1, (int)sent);
            } else {
                ESP_LOGE(TAG, "Failed to send message %d", i + 1);
            }
            
            // Wait between messages
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        // Cleanup client
        client_cleanup();
        ESP_LOGI(TAG, "Client disconnected");
    } else {
        ESP_LOGE(TAG, "Failed to connect client");
    }
    
    vTaskDelete(NULL);
}

void app_main(void) {
    initialize_nvs();
    
    // Initialize WiFi Access Point
    wifi_init_softap("esp32-ap", "", 4, 1);
    
    // Wait for WiFi to be ready
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Starting TCP server on port 8080...");
    
    // Start TCP server
    server_options_t server_opts = {
        .keepalive_idle = 30,
        .keepalive_interval = 5,
        .keepalive_count = 3,
        .max_connections = 5
    };
    
    int result = tcp_server_start(8080, NULL, echo_response_handler, NULL, &server_opts);
    if (result == 0) {
        ESP_LOGI(TAG, "TCP server started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TCP server");
        return;
    }
    
    // Create a task to demonstrate client functionality
    xTaskCreate(client_task, "tcp_client", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Application setup complete");
}