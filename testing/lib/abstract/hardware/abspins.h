#ifndef ABSPINS_H
#define ABSPINS_H

#include "driver/gpio.h"

#define HIGH 1
#define LOW 0

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

void dIPin(int pin, gpio_mode_t mode);
void dIPinIN(int pin);
void dIPinOUT(int pin);

// Pin I/O functions (int versions)

void dIWrite(int pin, int value);
int dIRead(int pin);

#endif // ABSPINS_H