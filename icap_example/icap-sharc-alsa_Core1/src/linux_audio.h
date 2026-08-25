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

#ifndef _linux_audio_h
#define _linux_audio_h

#include <stdint.h>

/* Out/In are from the perspective of the SHARC */
void linuxAudioIn(void *buffer, uint32_t size, void *usrPtr);

#endif
