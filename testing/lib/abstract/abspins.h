#ifndef ABSPINS_H
#define ABSPINS_H

#include "driver/gpio.h"

// Pin conversion functions
/// @brief Convert an `int` pin number to a `gpio_num_t` pin.
/// @param pin `int` pin number, typically in the range of `0` to `GPIO_NUM_MAX - 1`.
/// @return `gpio_num_t` representation of the pin, or `GPIO_NUM_NC` if the pin is invalid.
gpio_num_t dPinNum(int pin);

/// @brief Convert a `gpio_num_t` pin to an `int` pin number.
/// @param pin `gpio_num_t` pin, typically in the range of `GPIO_NUM_0` to `GPIO_NUM_MAX - 1`.
/// @return `int` representation of the pin, or `-1` if the pin is invalid.
int dFromPin(gpio_num_t pin);

// GPIO pin configuration functions (gpio_num_t)
/// @brief Set a `gpio_num_t` pin to a specific `gpio_mode_t` mode.
/// @param pin `gpio_num_t` pin, typically in the range of `GPIO_NUM_0` to `GPIO_NUM_MAX - 1`.
/// @param mode `gpio_mode_t` mode to set the pin to, such as `GPIO_MODE_INPUT` or `GPIO_MODE_OUTPUT`.
void dPin(gpio_num_t pin, gpio_mode_t mode);

/// @brief Set a `gpio_num_t` pin to input mode.
/// @param pin `gpio_num_t` pin, typically in the range of `GPIO_NUM_0` to `GPIO_NUM_MAX - 1`.
void dPinIN(gpio_num_t pin);

/// @brief Set a `gpio_num_t` pin to output mode.
/// @param pin `gpio_num_t` pin, typically in the range of `GPIO_NUM_0` to `GPIO_NUM_MAX - 1`.
void dPinOUT(gpio_num_t pin);

// GPIO pin I/O functions (gpio_num_t)
/// @brief Write a value to a `gpio_num_t` pin (0 or 1).
/// @param pin `gpio_num_t` pin, typically in the range of `GPIO_NUM_0` to `GPIO_NUM_MAX - 1`.
/// @param value Value to write to the pin, either `0` or `1`.
void dWrite(gpio_num_t pin, int value);

/// @brief Read a value from a `gpio_num_t` pin (0 or 1).
/// @param pin `gpio_num_t` pin, typically in the range of `GPIO_NUM_0` to `GPIO_NUM_MAX - 1`.
/// @return Value read from the pin, either `0` or `1`.
int dRead(gpio_num_t pin);

// GPIO pin configuration functions (int)
/// @brief Set an `int` pin number to a specific `gpio_mode_t` mode.
/// @param pin `int` pin number, typically in the range of `0` to `GPIO_NUM_MAX - 1`.
/// @param mode `gpio_mode_t` mode to set the pin to, such as `GPIO_MODE_INPUT` or `GPIO_MODE_OUTPUT`.
void dIPin(int pin, gpio_mode_t mode);

/// @brief Set an `int` pin to input mode.
/// @param pin `int` pin number, typically in the range of `0` to `GPIO_NUM_MAX - 1`.
void dIPinIN(int pin);

/// @brief Set an `int` pin to output mode.
/// @param pin `int` pin number, typically in the range of `0` to `GPIO_NUM_MAX - 1`.
void dIPinOUT(int pin);

// GPIO pin I/O functions (int)
/// @brief Write a value to an `int` pin (0 or 1).
/// @param pin `int` pin number, typically in the range of `0` to `GPIO_NUM_MAX - 1`.
/// @param value Value to write to the pin, either `0` or `1`.
void dIWrite(int pin, int value);

/// @brief Read a value from an `int` pin (0 or 1).
/// @param pin `int` pin number, typically in the range of `0` to `GPIO_NUM_MAX - 1`.
/// @return Value read from the pin, either `0` or `1`.
int dIRead(int pin);

#endif // ABSPINS_H