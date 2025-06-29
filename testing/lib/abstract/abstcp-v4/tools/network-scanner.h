#ifndef NETWORK_SCANNER_H
#define NETWORK_SCANNER_H

#include <stdbool.h>
#include <time.h>

typedef struct {
    char *ipv4;
    char *ipv6;
    int *open_ports;

    char *hostname;
    char *mac_address;
    char *vendor;
    char *device_type;

    char *os_fingerprint;
    char **services;
    int port_count;

    bool online;
    time_t last_seen;
    time_t first_seen;
    int signal_strength;
} network_device_t;

typedef struct {
    network_device_t *devices;
    int device_count;
    int max_devices;
} network_scan_result_t;

typedef struct {
    int timeout;
    char *start_ip;
    char *end_ip;
    int *ports;
    int retry_count;
} network_scan_options_t;

// Function declarations
network_scan_result_t *network_scan(network_scan_options_t *options);
void free_scan_result(network_scan_result_t *result);

#endif // NETWORK_SCANNER_H