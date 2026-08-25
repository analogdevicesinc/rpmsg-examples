/**
 * Copyright (c) 2026 - Analog Devices Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc.
 * and its licensors.
 *
 * This software is subject to the terms and conditions of the license set
 * forth in the project LICENSE file. Downloading, reproducing, distributing or
 * otherwise using the software constitutes acceptance of the license. The
 * software may not be used except as expressly authorized under the license.
 */

#ifndef _gpio_pins_h
#define _gpio_pins_h

#include <services/gpio/adi_gpio.h>

typedef struct _GPIO_CONFIG {
    ADI_GPIO_PORT port;
    uint32_t pinNum;
    ADI_GPIO_DIRECTION dir;
    bool state;
} GPIO_CONFIG;

typedef enum _GPIO_PIN_ID {
    GPIO_PIN_UNKNOWN = -1,
    GPIO_PIN_SOMCRR_PB1 = 0,
    GPIO_PIN_SOMCRR_PB2,
    GPIO_PIN_SOMCRR_LED7,
    GPIO_PIN_SOMCRR_LED10,
    GPIO_PIN_SOMCRR_LED9,
    GPIO_PIN_SOMCRR_A2B1_IRQ,
    GPIO_PIN_SOMCRR_PTPPPS0,
    GPIO_PIN_MAX
} GPIO_PIN_ID;

bool gpio_get_pin(GPIO_CONFIG *pinConfig, GPIO_PIN_ID pinId);
void gpio_set_pin(GPIO_CONFIG *pinConfig, GPIO_PIN_ID pinId, bool state);
void gpio_toggle_pin(GPIO_CONFIG *pinConfig, GPIO_PIN_ID pinId);
void gpio_init_pins(GPIO_CONFIG *pinConfig, int numPins);

extern GPIO_CONFIG gpioPins[];

#endif
