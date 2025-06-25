#include "abswifi.h"
#include "absnvs.h"

void app_main(void) {
    initialize_nvs();
    wifi_init_softap("esp32-ap", "", 4, 1);
}