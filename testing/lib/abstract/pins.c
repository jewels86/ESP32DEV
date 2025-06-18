#include "abstract.h"
#include "driver/gpio.h"

// Convert an integer pin number to a `gpio_num_t` type.
gpio_num_t dPinNum(int pin) {
    if (pin < 0 || pin >= GPIO_NUM_MAX) {
        return GPIO_NUM_NC; // Not connected
    }
    return (gpio_num_t)pin;
}
// Convert a `gpio_num_t` pin to an integer pin number.
int dFromPin(gpio_num_t pin) {
    if (pin < GPIO_NUM_0 || pin >= GPIO_NUM_MAX) {
        return -1; // Invalid pin
    }
    return (int)pin;
}

// Set a pin to a specific mode.
void dPin(gpio_num_t pin, gpio_mode_t mode) {
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, mode);
}
// Set a pin to input mode.
void dPinIN(gpio_num_t pin) {
    dPin(pin, GPIO_MODE_INPUT);
}
// Set a pin to output mode.
void dPinOUT(gpio_num_t pin) {
    dPin(pin, GPIO_MODE_OUTPUT);
}
// Write a value to a pin (0 or 1).
void dWrite(gpio_num_t pin, int value) {
    gpio_set_level(pin, value);
}
// Read a value from a pin (0 or 1).
int dRead(gpio_num_t pin) {
    return gpio_get_level(pin);
}

// Set a pin to a specific mode using an integer pin number.
void dPin(int pin, gpio_mode_t mode) {
    dPin(dPinNum(pin), mode);
}
// Set a pin to input mode using an integer pin number.
void dPinIN(int pin) {
    dPinIN(dPinNum(pin));
}
// Set a pin to output mode using an integer pin number.
void dPinOUT(int pin) {
    dPinOUT(dPinNum(pin));
}
// Write a value to a pin using an integer pin number (0 or 1).
void dWrite(int pin, int value) {
    dWrite(dPinNum(pin), value);
}
// Read a value from a pin using an integer pin number (0 or 1).
int dRead(int pin) {
    return dRead(dPinNum(pin));
}