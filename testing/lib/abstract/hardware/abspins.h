#ifndef ABSPINS_H
#define ABSPINS_H

#include "driver/gpio.h"

// Pin utility functions

gpio_num_t dPinNum(int pin);
int dFromPin(gpio_num_t pin);

// Pin configuration functions (gpio_num_t versions)

void dPin(gpio_num_t pin, gpio_mode_t mode);
void dPinIN(gpio_num_t pin);
void dPinOUT(gpio_num_t pin);

// Pin I/O functions (gpio_num_t versions)

void dWrite(gpio_num_t pin, int value);
int dRead(gpio_num_t pin);

// Pin configuration functions (int versions)

void dPin(int pin, gpio_mode_t mode);
void dPinIN(int pin);
void dPinOUT(int pin);

// Pin I/O functions (int versions)

void dWrite(int pin, int value);
int dRead(int pin);

#endif // ABSPINS_H