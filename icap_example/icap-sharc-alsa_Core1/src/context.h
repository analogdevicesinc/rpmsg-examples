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
/* Standard includes. */
#ifndef _context_h
#define _context_h

#include <stdint.h>
#include <stdbool.h>

/* Simple driver includes */
#include "twi_simple.h"
#include "sport_simple.h"

/* oss-services includes */

/* Project includes */
#include "route.h"

/* Misc defines */
#define UNUSED(expr) do { (void)(expr); } while (0)

/*
 * WARNING: While reasonable effort has gone into uniformly using a
 *          consistent audio data type (i.e. int32, int16, etc) throughout
 *          the code, changing SYSTEM_AUDIO_TYPE from 'int32_t' to a
 *          different type may need to be manually configured elsewhere
 *          in the system.
 *
 * WARNING: If changing DAC audio channels, be sure to update
 *          and confirm the SPORT initialization in 'init.c' and
 *          audio processing functions in 'codec_audio.c'.
 *
 */
#define SYSTEM_MCLK_RATE            (24576000)
#define SYSTEM_SAMPLE_RATE          (48000)
#define SYSTEM_BLOCK_SIZE           (64)
#define SYSTEM_AUDIO_TYPE           int32_t
#define SYSTEM_MAX_CHANNELS         (32)

/* In/Out are from the perspective of the SHARC */

#define DAC_DMA_CHANNELS            (16)
#define LINUX_AUDIO_IN_CHANNELS_MAX (16)
#define LINUX_AUDIO_IN_CHANNELS_MIN (1)
#define LINUX_AUDIO_OUT_CHANNELS    (16)
#define LINUX_AUDIO_OUT_RATE 48000

#define ICAP_RECORD_EN   // record updates are commented with this Macro
/*
 * The main application context.  Used as a container for a
 * variety of useful pointers, handles, etc., between various
 * modules and subsystems.
 */
typedef struct _APP_CONTEXT {

    /* Device handles */
    sTWI *twi2Handle;
    sTWI *adau1962TwiHandle;
    sSPORT *dacSportOutHandle;
    /* Audio routing table */
    ROUTE_INFO *routingTable;

} APP_CONTEXT;

/* Make the application context available for convenience */
extern APP_CONTEXT mainAppContext;

#endif
