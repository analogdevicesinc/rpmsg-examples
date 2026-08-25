/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

#ifndef _clocks_h
#define _clocks_h

#define CCLK (1000000000)
#define SCLK0 (CCLK / 8)

/*
 * Need to override and round up the TWI prescale for the TWI simple
 * driver since SCLK0 is not evenly divisible to create the 10MHz time
 * reference.
 */
#define TWI_SIMPLE_PRESCALE ((SCLK0 / 10000000u) + 1)

#endif
