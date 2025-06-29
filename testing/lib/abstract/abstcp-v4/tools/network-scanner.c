#include "abstcp-v4/client.h"
#include "abstcp-v4/tools/network-scanner.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef ESP_PLATFORM
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Helper function to convert IP string to integer
static uint32_t ip_to_int(const char *ip) {
    struct in_addr addr;
    if (inet_aton(ip, &addr) == 0) {
        return 0;
    }
    return ntohl(addr.s_addr);
}

// Helper function to convert integer to IP string
static void int_to_ip(uint32_t ip, char *buffer) {
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    strcpy(buffer, inet_ntoa(addr));
}

// Callback function for handling received data during port scan
static ssize_t scan_recv_callback(int sockfd, void *buf, size_t len, int flags) {
    // For network scanning, we just need to know if connection succeeded
    // We don't need to process the received data
    return 0;
}

// Function to scan a single port on a host
static bool scan_port(const char *host, uint16_t port, int timeout) {
    send_func_t send_func = client(host, NULL, port, scan_recv_callback);
    
    if (send_func != NULL) {
        // Connection successful - port is open
        client_cleanup();
        return true;
    }
    
    return false;
}

// Function to create a new network device entry
static network_device_t *create_network_device(const char *ip) {
    network_device_t *device = malloc(sizeof(network_device_t));
    if (!device) return NULL;
    
    // Initialize device structure
    memset(device, 0, sizeof(network_device_t));
    
    // Set IP address
    device->ipv4 = malloc(strlen(ip) + 1);
    if (device->ipv4) {
        strcpy(device->ipv4, ip);
    }
    
    device->online = false;
    device->port_count = 0;
    device->open_ports = NULL;
    device->hostname = NULL;
    device->mac_address = NULL;
    device->vendor = NULL;
    device->device_type = NULL;
    device->os_fingerprint = NULL;
    device->services = NULL;
    device->signal_strength = 0;
    device->first_seen = time(NULL);
    device->last_seen = time(NULL);
    
    return device;
}

// Function to add an open port to a device
static void add_open_port(network_device_t *device, uint16_t port) {
    device->port_count++;
    device->open_ports = realloc(device->open_ports, device->port_count * sizeof(int));
    if (device->open_ports) {
        device->open_ports[device->port_count - 1] = port;
    }
}

network_scan_result_t *network_scan(network_scan_options_t *options) {
    if (!options || !options->start_ip || !options->end_ip) {
        return NULL;
    }
    
    // Create result structure
    network_scan_result_t *result = malloc(sizeof(network_scan_result_t));
    if (!result) return NULL;
    
    result->device_count = 0;
    result->max_devices = 256; // Initial capacity
    result->devices = malloc(result->max_devices * sizeof(network_device_t));
    if (!result->devices) {
        free(result);
        return NULL;
    }
    
    // Convert IP range to integers for iteration
    uint32_t start_ip = ip_to_int(options->start_ip);
    uint32_t end_ip = ip_to_int(options->end_ip);
    
    if (start_ip == 0 || end_ip == 0 || start_ip > end_ip) {
        free(result->devices);
        free(result);
        return NULL;
    }
    
    // Default ports to scan if none specified
    int default_ports[] = {22, 23, 25, 53, 80, 110, 143, 443, 993, 995};
    int *ports_to_scan = options->ports ? options->ports : default_ports;
    int port_count = options->ports ? 0 : 10; // Count ports if using default
    
    // Count ports if custom list provided
    if (options->ports) {
        while (options->ports[port_count] != 0) {
            port_count++;
        }
    }
    
    printf("Starting network scan from %s to %s\n", options->start_ip, options->end_ip);
    
    // Scan each IP in the range
    for (uint32_t current_ip = start_ip; current_ip <= end_ip; current_ip++) {
        char ip_str[16];
        int_to_ip(current_ip, ip_str);
        
        printf("Scanning %s...\n", ip_str);
        
        network_device_t *device = NULL;
        bool host_responsive = false;
        
        // Scan each port for this IP
        for (int p = 0; p < port_count; p++) {
            uint16_t port = ports_to_scan[p];
            
            // Try to connect with retry logic
            bool port_open = false;
            int retries = options->retry_count > 0 ? options->retry_count : 1;
            
            for (int retry = 0; retry < retries && !port_open; retry++) {
                if (scan_port(ip_str, port, options->timeout)) {
                    port_open = true;
                    host_responsive = true;
                    
                    // Create device entry if this is the first open port found
                    if (!device) {
                        device = create_network_device(ip_str);
                        if (!device) continue;
                        device->online = true;
                    }
                    
                    // Add the open port
                    add_open_port(device, port);
                    printf("  Port %d: OPEN\n", port);
                }
                
                if (retry < retries - 1) {
#ifdef ESP_PLATFORM
                    vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay between retries
#else
                    usleep(100000); // 100ms delay between retries
#endif
                }
            }
        }
        
        // Add device to results if any ports were found open
        if (device && host_responsive) {
            // Expand results array if needed
            if (result->device_count >= result->max_devices) {
                result->max_devices *= 2;
                result->devices = realloc(result->devices, 
                    result->max_devices * sizeof(network_device_t));
                if (!result->devices) {
                    // Handle realloc failure
                    free(device->ipv4);
                    free(device->open_ports);
                    free(device);
                    break;
                }
            }
            
            // Copy device to results
            memcpy(&result->devices[result->device_count], device, sizeof(network_device_t));
            result->device_count++;
            free(device); // Free the temporary device structure
        }
    }
    
    printf("Network scan completed. Found %d responsive devices.\n", result->device_count);
    
    return result;
}

// Function to free scan results
void free_scan_result(network_scan_result_t *result) {
    if (!result) return;
    
    for (int i = 0; i < result->device_count; i++) {
        network_device_t *device = &result->devices[i];
        free(device->ipv4);
        free(device->ipv6);
        free(device->open_ports);
        free(device->hostname);
        free(device->mac_address);
        free(device->vendor);
        free(device->device_type);
        free(device->os_fingerprint);
        
        if (device->services) {
            // Free service strings if they exist
            for (int j = 0; device->services[j]; j++) {
                free(device->services[j]);
            }
            free(device->services);
        }
    }
    
    free(result->devices);
    free(result);
}