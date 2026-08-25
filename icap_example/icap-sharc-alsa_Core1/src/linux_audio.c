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

#include <icap_io.h>
#include <stdint.h>
#include <string.h>

#include "context.h"
#include "linux_audio.h"
#include "process_audio.h"
#include "icap_record_device.h"
#include "icap_playback_device.h"

volatile unsigned LINUXIN = 0;

/* Out/In are from the perspective of the SHARC */

void linuxAudioIn(void *buffer, uint32_t maxSize, void *usrPtr)
{
    APP_CONTEXT *context = (APP_CONTEXT *)usrPtr;

    UNUSED(context);

    /*
     * This buffer is for audio from ALSA.  It should be passed by reference
     * to ALSA if possible. It is in SHARC0 L2 cached memory and will be
     * invalidated by process_audio().  If it's not possible to pass by
     * reference then hang on to the pointer and fill it before the next
     * audio frame is ready and modify the code in process_audio() to not
     * invalidate the buffer to save cycles.
     *
     * The buffer size in bytes is:
     *   sizeof(SYSTEM_AUDIO_TYPE) * LINUX_AUDIO_IN_CHANNELS_MAX * SYSTEM_BLOCK_SIZE
     *
     * These defines are in SHARC0/context.h
     *
     */

    int32_t* ptr = buffer;
    for (int i=0; i<maxSize/sizeof(ptr[0]); ++i) {
        *ptr++ = get_s32(&icap_sharc_alsa_playback_buffer);
    }

    LINUXIN++;
}
