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
    processAudio(context, STREAM_ID_CODEC_OUT,
        DAC_DMA_CHANNELS, SYSTEM_BLOCK_SIZE, sizeof(SYSTEM_AUDIO_TYPE),
        buffer, true
    );

    DACOUT++;
}
