#ifndef ABSWIFI_H
#define ABSWIFI_H

#include <stdbool.h>
#include <stdint.h>

/// @brief Initialize the Wi-Fi station mode with the given SSID and password.
/// @param ssid The SSID of the Wi-Fi network.
/// @param password The password for the Wi-Fi network.
void wifi_init_sta(const char* ssid, const char* password);

/// @brief Scan for available Wi-Fi networks.
/// @param scan_list_size The maximum number of access points to scan for.
/// @param use_channel_bitmap If true, use channel bitmap for scanning; otherwise, scan all channels.
void wifi_scan(uint16_t scan_list_size, bool use_channel_bitmap);

#endif // ABSWIFI_H