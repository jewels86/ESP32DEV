#include "abspins.h"
#include "driver/gpio.h"

gpio_num_t dPinNum(int pin) {
    if (pin < 0 || pin >= GPIO_NUM_MAX) {
        return GPIO_NUM_NC;
    }
    return (gpio_num_t)pin;
}
int dFromPin(gpio_num_t pin) {
    if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
        return -1;
    }
    return (int)pin;
}

void dPin(gpio_num_t pin, gpio_mode_t mode) {
#if ESP_IDF_VERSION_MAJOR >= 4
    gpio_pad_select_gpio(pin);
#else
    esp_rom_gpio_pad_select_gpio(pin);
#endif
    gpio_set_direction(pin, mode);
}
void dPinIN(gpio_num_t pin) {
    dPin(pin, GPIO_MODE_INPUT);
}
void dPinOUT(gpio_num_t pin) {
    dPin(pin, GPIO_MODE_OUTPUT);
}

void dWrite(gpio_num_t pin, int value) {
    gpio_set_level(pin, value);
}
int dRead(gpio_num_t pin) {
    return gpio_get_level(pin);
}

void dIPin(int pin, gpio_mode_t mode) {
    dPin(dPinNum(pin), mode);
}
void dIPinIN(int pin) {
    dIPin(dPinNum(pin), GPIO_MODE_INPUT);
}
void dIPinOUT(int pin) {
    dIPin(dPinNum(pin), GPIO_MODE_OUTPUT);
}

void dIWrite(int pin, int value) {
    dWrite(dPinNum(pin), value);
}
int dIRead(int pin) {
    return dRead(dPinNum(pin));
}