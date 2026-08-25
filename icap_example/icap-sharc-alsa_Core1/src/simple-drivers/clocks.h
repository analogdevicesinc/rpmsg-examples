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
