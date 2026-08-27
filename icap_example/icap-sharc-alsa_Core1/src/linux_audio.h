/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

#ifndef _linux_audio_h
#define _linux_audio_h

#include <stdint.h>

/* Out/In are from the perspective of the SHARC */
void linuxAudioIn(void *buffer, uint32_t size, void *usrPtr);

#endif
