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

/*
 * This code has been modified by Analog Devices, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "context.h"
#include "route.h"
#include "process_audio.h"

/***********************************************************************
 * Main application context
 **********************************************************************/
static APP_CONTEXT *context = &mainAppContext;

extern unsigned route_setting;

/***********************************************************************

// Details of the play back settings
const char inputargs[] =
    "[ <idx> <src> <dst> <channels> <channel route> ]\n"
    "  idx         - Routing index\n"
    "  src ID  - Source stream index\n"
    "  dst ID  - Destination stream index\n"
    "  channels    - Number of channels\n"
	"  channel route - route which 12 channels to DAC\n"
    " No arguments\n";
**********************************************************************/

STREAM_ID str2stream(char *stream, bool src)
{
	if (strcmp(stream, "codec") == 0) {
		return (src ? STREAM_ID_CODEC_IN : STREAM_ID_CODEC_OUT);
	} else if (strcmp(stream, "linux") == 0 || strcmp(stream, "wav") == 0) {
		return (src ? STREAM_ID_LINUX_IN : STREAM_ID_LINUX_OUT);
	} else if (strcmp(stream, "off") == 0) {
		return (STREAM_ID_UNKNOWN);
	}

	return (STREAM_ID_MAX);
}

void apply_playback_settings(char **argv)
{
	ROUTE_INFO *route;
	unsigned idx, srcOffset, sinkOffset, channels, attenuation, mix,
		channel_route;
	STREAM_ID srcID, sinkID;

	char *endptr;
	long val;

	val = strtol(argv[0], &endptr, 10);
	idx = (unsigned)val;

	route = context->routingTable + idx;
	srcID = route->srcID;
	srcOffset = route->srcOffset;
	sinkID = route->sinkID;
	sinkOffset = route->sinkOffset;
	channels = route->channels;
	channel_route = route->channel_route;

	/* Gather the source info */
	srcID = str2stream(argv[1], true);
	if (srcID == STREAM_ID_MAX)
		return;
	srcOffset = 0;

	/* Gather the sink info */
	sinkID = str2stream(argv[2], false);
	if (sinkID == STREAM_ID_MAX)
		return;
	sinkOffset = 0;

	/* Get the number of channels */
	val = strtol(argv[3], &endptr, 10);
	if (endptr == argv[3] || *endptr != '\0')
		return;
	channels = (unsigned)val;

	/* Get the channel routing value */

	channel_route = route_setting;

	clearStreamBuffer(route->sinkID, route->sinkOffset, route->channels);

	route->srcID = srcID;
	route->srcOffset = srcOffset;
	route->sinkID = sinkID;
	route->sinkOffset = sinkOffset;
	route->channels = channels;
	route->channel_route = channel_route;
}
