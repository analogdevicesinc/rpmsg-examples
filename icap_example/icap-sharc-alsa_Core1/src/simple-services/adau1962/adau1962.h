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

#ifndef _adau1962_h
#define _adau1962_h

#include <stdint.h>

#include "twi_simple.h"

// ADAU1962 service return values
typedef enum
{
    ADAU1962_SUCCESS,  // Successful API call
    ADAU1962_ERROR     // General failure
} ADAU1962_RESULT;

ADAU1962_RESULT init_adau1962(sTWI *twi, uint8_t adau_address);

#endif
