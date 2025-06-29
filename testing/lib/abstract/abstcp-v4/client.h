#ifndef ABSTCP_V4_CLIENT_H
#define ABSTCP_V4_CLIENT_H

#include <stdint.h>
#include <sys/types.h>

/// @brief Function pointer type for receiving data
typedef ssize_t (*recv_func_t)(int sockfd, void *buf, size_t len, int flags);
/// @brief Function pointer type for sending data
typedef ssize_t (*send_func_t)(int sockfd, const void *buf, size_t len, int flags);

/// @brief Create a TCP client connection
/// @param host The hostname or IP address to connect to
/// @param target Target parameter (currently unused)
/// @param port The port number to connect to
/// @param recv_callback Optional callback function for handling received data
/// @return send_func_t Function pointer for sending data, or NULL on failure
send_func_t client(const char *host, const char *target, uint16_t port, recv_func_t recv_callback);

/// @brief Clean up the client resources
void client_cleanup(void);

#endif // ABSTCP_V4_CLIENT_H
