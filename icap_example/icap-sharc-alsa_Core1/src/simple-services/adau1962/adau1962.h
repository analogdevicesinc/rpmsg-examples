/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

#ifndef _adau1962_h
#define _adau1962_h

#include <stdint.h>

#include "twi_simple.h"

// ADAU1962 service return values
typedef enum {
	ADAU1962_SUCCESS, // Successful API call
	ADAU1962_ERROR // General failure
} ADAU1962_RESULT;

ADAU1962_RESULT init_adau1962(sTWI *twi, uint8_t adau_address);

#endif
