/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

#ifndef _process_audio_h
#define _process_audio_h

#include <stdint.h>

#include "context.h"
#include "route.h"

void processAudio(APP_CONTEXT *context, STREAM_ID streamID,
		  unsigned numChannels, unsigned numFrames, unsigned wordSize,
		  void *data, bool flush);

void clearStreamBuffer(STREAM_ID streamID, unsigned int sinkOffset,
		       unsigned int channels);

#endif
