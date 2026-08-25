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

#ifndef _init_h
#define _init_h

#include <stdint.h>
#include <stdbool.h>

#include "context.h"
#include "gpio_pins.h"

void gpio_init(void);
void umm_heap_init(void);

void mclk_init(APP_CONTEXT *context);
void disable_mclk(APP_CONTEXT *context);
void enable_mclk(APP_CONTEXT *context);

void adau1962_init(APP_CONTEXT *context);

void audio_routing_init(APP_CONTEXT *context);

#endif
