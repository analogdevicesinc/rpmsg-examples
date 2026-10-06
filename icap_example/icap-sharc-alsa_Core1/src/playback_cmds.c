/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
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

/***********************************************************************

// Details of the play back settings
const char inputargs[] =
	"[ <idx> <src> <src offset> <dst> <dst offset> <channels> ]\n"
    "  idx         - Routing index\n"
	"  src         - Source stream\n"
	"  src offset  - Source channel offset\n"
	"  dst         - Destination stream\n"
	"  dst offset  - Destination channel offset\n"
    "  channels    - Number of channels\n"
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
	unsigned idx, srcOffset, sinkOffset, channels;
	STREAM_ID srcID, sinkID;

	char *endptr;
	long val;

	val = strtol(argv[0], &endptr, 10);
	if (endptr == argv[0] || *endptr != '\0' || val < 0 ||
	    val >= MAX_AUDIO_ROUTES)
		return;
	idx = (unsigned)val;

	route = context->routingTable + idx;
	srcID = route->srcID;
	srcOffset = route->srcOffset;
	sinkID = route->sinkID;
	sinkOffset = route->sinkOffset;
	channels = route->channels;

	/* Gather the source info */
	srcID = str2stream(argv[1], true);
	if (srcID == STREAM_ID_MAX)
		return;

	val = strtol(argv[2], &endptr, 10);
	if (endptr == argv[2] || *endptr != '\0' || val < 0)
		return;
	srcOffset = (unsigned)val;

	/* Gather the sink info */
	sinkID = str2stream(argv[3], false);
	if (sinkID == STREAM_ID_MAX)
		return;

	val = strtol(argv[4], &endptr, 10);
	if (endptr == argv[4] || *endptr != '\0' || val < 0)
		return;
	sinkOffset = (unsigned)val;

	/* Get the number of channels */
	val = strtol(argv[5], &endptr, 10);
	if (endptr == argv[5] || *endptr != '\0' || val < 0)
		return;
	channels = (unsigned)val;

	clearStreamBuffer(route->sinkID, route->sinkOffset, route->channels);

	route->srcID = srcID;
	route->srcOffset = srcOffset;
	route->sinkID = sinkID;
	route->sinkOffset = sinkOffset;
	route->channels = channels;
}
