#include "abswifi.h"
#include "absnvs.h"
#include "abstcp-v4/server.h"
#include "abstcp-v4/client.h"
#include "abstcp-v4/tools.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <string.h>
#include <sys/socket.h>

static const char *TAG = "BENCHMARK";

// Benchmark results structure
typedef struct {
    int64_t nvs_init_time_us;
    int64_t wifi_init_time_us;
    int64_t server_start_time_us;
    int64_t client_connect_time_us;
    int64_t total_send_time_us;
    int64_t total_recv_time_us;
    int64_t network_scan_time_us;
    int64_t client_cleanup_time_us;
    int message_count;
    int total_bytes_sent;
    int total_bytes_received;
    double avg_send_time_per_message_us;
    double avg_recv_time_per_message_us;
    double throughput_kbps;
} benchmark_results_t;

static benchmark_results_t bench_results = {0};

// Custom recv function for the client with timing
ssize_t client_recv_func(int sockfd, void *buf, size_t len, int flags) {
    int64_t start_time = esp_timer_get_time();
    ssize_t result = recv(sockfd, buf, len, flags);
    int64_t end_time = esp_timer_get_time();
    
    if (result > 0) {
        bench_results.total_recv_time_us += (end_time - start_time);
        bench_results.total_bytes_received += result;
        ESP_LOGI(TAG, "RECV: %d bytes in %lld us", (int)result, end_time - start_time);
    }
    
    return result;
}

// Example response function for the TCP server with timing
int echo_response_handler(const char *request_data, int request_len, char *response_buffer, int response_buffer_size, void *user_data) {
    int64_t start_time = esp_timer_get_time();
    
    ESP_LOGI(TAG, "SERVER: Processing request (%d bytes): %.*s", request_len, request_len, request_data);
    
    // Simple echo response with a prefix
    const char *prefix = "Echo: ";
    int prefix_len = strlen(prefix);
    
    int response_len = -1;
    if (prefix_len + request_len < response_buffer_size) {
        strcpy(response_buffer, prefix);
        memcpy(response_buffer + prefix_len, request_data, request_len);
        response_len = prefix_len + request_len;
    }
    
    int64_t end_time = esp_timer_get_time();
    ESP_LOGI(TAG, "SERVER: Response prepared in %lld us (%d bytes)", end_time - start_time, response_len);
    
    return response_len;
}

// Task to demonstrate client functionality with comprehensive benchmarking
void client_task(void *pvParameters) {
    // Wait a bit for the server to start
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGI(TAG, "=== STARTING CLIENT BENCHMARK ===");
    
    // Benchmark client connection
    int64_t connect_start = esp_timer_get_time();
    send_func_t send_func = client("192.168.4.1", NULL, 8080, client_recv_func);
    int64_t connect_end = esp_timer_get_time();
    
    bench_results.client_connect_time_us = connect_end - connect_start;
    ESP_LOGI(TAG, "CLIENT_CONNECT: %lld us", bench_results.client_connect_time_us);
    
    if (send_func != NULL) {
        ESP_LOGI(TAG, "Client connected successfully");
        
        // Test messages of varying sizes
        const char *messages[] = {
            "Small message",
            "This is a medium-sized test message with more content to test throughput",
            "This is a large test message designed to benchmark network performance with substantial data payload. It contains multiple sentences and various characters to simulate real-world usage patterns and stress test the TCP implementation.",
            "PING",
            "Status check message",
            "Performance test data with numbers: 1234567890 and symbols: !@#$%^&*()",
            "Final benchmark message to complete the testing sequence"
        };
        
        int num_messages = sizeof(messages) / sizeof(messages[0]);
        bench_results.message_count = num_messages;
        
        // Send and time each message
        for (int i = 0; i < num_messages; i++) {
            ESP_LOGI(TAG, "--- Message %d/%d ---", i + 1, num_messages);
            
            int64_t send_start = esp_timer_get_time();
            ssize_t sent = send_func(0, messages[i], strlen(messages[i]), 0);
            int64_t send_end = esp_timer_get_time();
            
            int64_t message_send_time = send_end - send_start;
            bench_results.total_send_time_us += message_send_time;
            
            if (sent > 0) {
                bench_results.total_bytes_sent += sent;
                ESP_LOGI(TAG, "SEND: %d bytes in %lld us (%.2f KB/s)", 
                        (int)sent, message_send_time,
                        (double)(sent * 1000000) / (message_send_time * 1024));
            } else {
                ESP_LOGE(TAG, "SEND_FAILED: Message %d", i + 1);
            }
            
            // Wait for response (this will trigger recv timing in client_recv_func)
            char response_buffer[512];
            int64_t recv_start = esp_timer_get_time();
            ssize_t received = recv(0, response_buffer, sizeof(response_buffer) - 1, 0);
            int64_t recv_end = esp_timer_get_time();
            
            if (received > 0) {
                response_buffer[received] = '\0';
                ESP_LOGI(TAG, "RESPONSE: %.*s", (int)received, response_buffer);
                ESP_LOGI(TAG, "RECV_TOTAL: %d bytes in %lld us", (int)received, recv_end - recv_start);
            }
            
            // Wait between messages
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        
        // Benchmark client cleanup
        int64_t cleanup_start = esp_timer_get_time();
        client_cleanup();
        int64_t cleanup_end = esp_timer_get_time();
        
        bench_results.client_cleanup_time_us = cleanup_end - cleanup_start;
        ESP_LOGI(TAG, "CLIENT_CLEANUP: %lld us", bench_results.client_cleanup_time_us);
        
        // Calculate averages
        if (bench_results.message_count > 0) {
            bench_results.avg_send_time_per_message_us = 
                (double)bench_results.total_send_time_us / bench_results.message_count;
            bench_results.avg_recv_time_per_message_us = 
                (double)bench_results.total_recv_time_us / bench_results.message_count;
        }
        
        // Calculate throughput
        if (bench_results.total_send_time_us > 0) {
            bench_results.throughput_kbps = 
                (double)(bench_results.total_bytes_sent * 8 * 1000000) / 
                (bench_results.total_send_time_us * 1024);
        }
        
        ESP_LOGI(TAG, "=== CLIENT BENCHMARK COMPLETE ===");
    } else {
        ESP_LOGE(TAG, "CLIENT_CONNECT_FAILED");
    }
    
    vTaskDelete(NULL);
}

// Function to print comprehensive benchmark results
void print_benchmark_results(void) {
    ESP_LOGI(TAG, "\n");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "       COMPREHENSIVE BENCHMARK RESULTS");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "INITIALIZATION TIMINGS:");
    ESP_LOGI(TAG, "  NVS Init:           %lld us (%.2f ms)", 
             bench_results.nvs_init_time_us, bench_results.nvs_init_time_us / 1000.0);
    ESP_LOGI(TAG, "  WiFi Init:          %lld us (%.2f ms)", 
             bench_results.wifi_init_time_us, bench_results.wifi_init_time_us / 1000.0);
    ESP_LOGI(TAG, "  Server Start:       %lld us (%.2f ms)", 
             bench_results.server_start_time_us, bench_results.server_start_time_us / 1000.0);
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "CONNECTION TIMINGS:");
    ESP_LOGI(TAG, "  Client Connect:     %lld us (%.2f ms)", 
             bench_results.client_connect_time_us, bench_results.client_connect_time_us / 1000.0);
    ESP_LOGI(TAG, "  Client Cleanup:     %lld us (%.2f ms)", 
             bench_results.client_cleanup_time_us, bench_results.client_cleanup_time_us / 1000.0);
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "DATA TRANSFER STATISTICS:");
    ESP_LOGI(TAG, "  Messages Sent:      %d", bench_results.message_count);
    ESP_LOGI(TAG, "  Total Bytes Sent:   %d bytes", bench_results.total_bytes_sent);
    ESP_LOGI(TAG, "  Total Bytes Recv:   %d bytes", bench_results.total_bytes_received);
    ESP_LOGI(TAG, "  Total Send Time:    %lld us (%.2f ms)", 
             bench_results.total_send_time_us, bench_results.total_send_time_us / 1000.0);
    ESP_LOGI(TAG, "  Total Recv Time:    %lld us (%.2f ms)", 
             bench_results.total_recv_time_us, bench_results.total_recv_time_us / 1000.0);
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "PERFORMANCE METRICS:");
    ESP_LOGI(TAG, "  Avg Send/Message:   %.2f us (%.2f ms)", 
             bench_results.avg_send_time_per_message_us, bench_results.avg_send_time_per_message_us / 1000.0);
    ESP_LOGI(TAG, "  Avg Recv/Message:   %.2f us (%.2f ms)", 
             bench_results.avg_recv_time_per_message_us, bench_results.avg_recv_time_per_message_us / 1000.0);
    ESP_LOGI(TAG, "  Send Throughput:    %.2f Kbps", bench_results.throughput_kbps);
    ESP_LOGI(TAG, "  Network Scan:       %lld us (%.2f ms)", 
             bench_results.network_scan_time_us, bench_results.network_scan_time_us / 1000.0);
    ESP_LOGI(TAG, "");
    
    // Calculate total benchmark time
    int64_t total_time = bench_results.nvs_init_time_us + 
                        bench_results.wifi_init_time_us + 
                        bench_results.server_start_time_us + 
                        bench_results.client_connect_time_us + 
                        bench_results.total_send_time_us + 
                        bench_results.total_recv_time_us + 
                        bench_results.network_scan_time_us + 
                        bench_results.client_cleanup_time_us;
    
    ESP_LOGI(TAG, "SUMMARY:");
    ESP_LOGI(TAG, "  Total Benchmark:    %lld us (%.2f ms)", total_time, total_time / 1000.0);
    ESP_LOGI(TAG, "  Memory Usage:       %d bytes free", (int)esp_get_free_heap_size());
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
}

void app_main(void) {
    ESP_LOGI(TAG, "=== STARTING ESP32 NETWORK BENCHMARK ===");
    
    // Initialize benchmark results
    memset(&bench_results, 0, sizeof(benchmark_results_t));
    
    // Benchmark NVS initialization
    int64_t nvs_start = esp_timer_get_time();
    initialize_nvs();
    int64_t nvs_end = esp_timer_get_time();
    bench_results.nvs_init_time_us = nvs_end - nvs_start;
    ESP_LOGI(TAG, "NVS_INIT: %lld us", bench_results.nvs_init_time_us);
    
    // Benchmark WiFi initialization
    int64_t wifi_start = esp_timer_get_time();
    wifi_init_softap("esp32-ap", "", 4, 1);
    int64_t wifi_end = esp_timer_get_time();
    bench_results.wifi_init_time_us = wifi_end - wifi_start;
    ESP_LOGI(TAG, "WIFI_INIT: %lld us", bench_results.wifi_init_time_us);
    
    // Wait for WiFi to be ready
    ESP_LOGI(TAG, "Waiting for WiFi to stabilize...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Benchmark TCP server start
    int64_t server_start = esp_timer_get_time();
    server_options_t server_opts = {
        .keepalive_idle = 30,
        .keepalive_interval = 5,
        .keepalive_count = 3,
        .max_connections = 5
    };
    
    int result = tcp_server_start(8080, NULL, echo_response_handler, NULL, &server_opts);
    int64_t server_end = esp_timer_get_time();
    bench_results.server_start_time_us = server_end - server_start;
    ESP_LOGI(TAG, "SERVER_START: %lld us", bench_results.server_start_time_us);
    
    if (result == 0) {
        ESP_LOGI(TAG, "TCP server started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TCP server");
        return;
    }
    
    // Create a task to demonstrate client functionality
    xTaskCreate(client_task, "tcp_client_benchmark", 8192, NULL, 5, NULL);
    
    // Wait for client task to complete
    vTaskDelay(pdMS_TO_TICKS(8000));
    
    ESP_LOGI(TAG, "=== STARTING NETWORK SCAN BENCHMARK ===");
    
    // Benchmark network scan
    int64_t scan_start = esp_timer_get_time();
    
    network_scan_options_t *scan_options = malloc(sizeof(network_scan_options_t));
    scan_options->timeout = 1000; 
    scan_options->start_ip = "192.168.4.1";
    scan_options->end_ip = "192.168.4.255";
    scan_options->ports = NULL;
    scan_options->retry_count = 3;

    network_scan_result_t *scan_result = network_scan(scan_options);
    int64_t scan_end = esp_timer_get_time();
    bench_results.network_scan_time_us = scan_end - scan_start;
    ESP_LOGI(TAG, "NETWORK_SCAN: %lld us", bench_results.network_scan_time_us);

    if (scan_result != NULL) {
        ESP_LOGI(TAG, "Network scan results (%d devices found):", scan_result->device_count);
        for (int i = 0; i < scan_result->device_count; i++) {
            ESP_LOGI(TAG, "  Host: %s, Port: %d, Status: %s",
                     scan_result->devices[i].ipv4,
                     scan_result->devices[i].open_ports[0],
                     scan_result->devices[i].online ? "ONLINE" : "OFFLINE");
        }
        free_scan_result(scan_result);
    } else {
        ESP_LOGI(TAG, "No scan results or scan failed.");
    }
    free(scan_options);
    
    // Wait a bit more for any remaining operations
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Print comprehensive benchmark results
    print_benchmark_results();
    
    ESP_LOGI(TAG, "=== BENCHMARK COMPLETE ===");
}