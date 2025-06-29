#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "abstcp-v4/client.h"

static const char *TAG = "abstcp-v4-client";

typedef ssize_t (*recv_func_t)(int sockfd, void *buf, size_t len, int flags);
typedef ssize_t (*send_func_t)(int sockfd, const void *buf, size_t len, int flags);

static int client_sock = -1;
static recv_func_t client_recv_callback = NULL;

static ssize_t client_send_func(int sockfd, const void *buf, size_t len, int flags) {
    if (client_sock == -1) {
        ESP_LOGE(TAG, "Client not connected");
        return -1;
    }
    
    ssize_t sent = send(client_sock, buf, len, flags);
    if (sent < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return sent;
    }
    
    if (client_recv_callback) {
        char rx_buffer[1024];
        int recv_len = client_recv_callback(client_sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (recv_len < 0) {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
        } else if (recv_len > 0) {
            rx_buffer[recv_len] = 0; // Null-terminate
            ESP_LOGI(TAG, "Received %d bytes:", recv_len);
            ESP_LOGI(TAG, "%s", rx_buffer);
        }
    }
    
    return sent;
}

send_func_t client(const char *host, const char *target, uint16_t port, recv_func_t recv_callback) {
    struct sockaddr_in dest_addr;
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    
    client_recv_callback = recv_callback;
    
    inet_pton(AF_INET, host, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    
    client_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (client_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return NULL;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host, port);
    
    int err = connect(client_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(client_sock);
        client_sock = -1;
        return NULL;
    }
    ESP_LOGI(TAG, "Successfully connected");
    
    return client_send_func;
}

void client_cleanup(void) {
    if (client_sock != -1) {
        ESP_LOGI(TAG, "Shutting down client socket");
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
        client_sock = -1;
        client_recv_callback = NULL;
    }
}