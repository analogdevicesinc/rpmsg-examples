/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
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
