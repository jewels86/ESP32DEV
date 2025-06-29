#ifndef ABSTCP_V4_SERVER_H
#define ABSTCP_V4_SERVER_H

#include <stdint.h>
#include <sys/types.h>

/// @brief Response callback function type
/// 
/// @param request_data Pointer to the received data
/// @param request_len Length of the received data
/// @param response_buffer Buffer to write the response to
/// @param response_buffer_size Maximum size of the response buffer
/// @param user_data User-provided data passed to the callback
/// @return Length of the response data, or 0 if no response should be sent, or -1 on error
typedef int (*response_func_t)(const char *request_data, int request_len, char *response_buffer, int response_buffer_size, void *user_data);

/// @brief Server configuration options
typedef struct {
    int keepalive_idle;        ///< TCP keepalive idle time in seconds (0 = use default)
    int keepalive_interval;    ///< TCP keepalive interval in seconds (0 = use default)
    int keepalive_count;       ///< TCP keepalive count (0 = use default)
    int max_connections;       ///< Maximum number of pending connections (0 = use default of 1)
} server_options_t;

/// @brief Start a TCP server
/// 
/// @param port The port number to listen on
/// @param host The host address to bind to (NULL or empty string for INADDR_ANY)
/// @param response_callback Function to call when data is received from clients
/// @param user_data User data to pass to the response callback
/// @param options Optional server configuration (can be NULL for defaults)
/// @return 0 on success, -1 on failure
int tcp_server_start(uint16_t port, const char *host, response_func_t response_callback, void *user_data, server_options_t *options);

#endif // ABSTCP_V4_SERVER_H