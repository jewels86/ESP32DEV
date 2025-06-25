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

/// @brief Initialize the Wi-Fi soft access point (AP) mode with the given parameters.
/// @param ssid The SSID of the soft AP.
/// @param password The password for the soft AP.
/// @param max_conn The maximum number of connections allowed to the soft AP.
/// @param channel The channel on which the soft AP will operate.
void wifi_init_softap(const char* ssid, const char* password, int max_conn, int channel);

#endif // ABSWIFI_H