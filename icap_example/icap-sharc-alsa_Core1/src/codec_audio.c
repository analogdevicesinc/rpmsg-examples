/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */
#include <stdint.h>
#include <string.h>

#include "context.h"
#include "codec_audio.h"
#include "process_audio.h"

volatile unsigned DACOUT = 0;

void dacAudioOut(void *buffer, uint32_t maxSize, void *usrPtr)
{
	APP_CONTEXT *context = (APP_CONTEXT *)usrPtr;

	/* Clear the contents */
	memset(buffer, 0, maxSize);

	/* Process audio */
	processAudio(context, STREAM_ID_CODEC_OUT, DAC_DMA_CHANNELS,
		     SYSTEM_BLOCK_SIZE, sizeof(SYSTEM_AUDIO_TYPE), buffer,
		     true);

	DACOUT++;
}
