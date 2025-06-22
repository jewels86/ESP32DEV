#include "abswifi.h"
#include "absnvs.h"

void app_main() {
    initialize_nvs();
    wifi_scan(10, 0);
}