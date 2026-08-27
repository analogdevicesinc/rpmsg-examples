/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

#ifndef _codec_audio_h
#define _codec_audio_h

#include <stdint.h>

void dacAudioOut(void *buffer, uint32_t size, void *usrPtr);
#endif
