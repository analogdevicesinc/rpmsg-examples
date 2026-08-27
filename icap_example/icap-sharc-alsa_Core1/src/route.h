/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

#ifndef _route_h
#define _route_h

/* Audio routing */
#define MAX_AUDIO_ROUTES 24

/*
 * Audio stream identifiers (31 max)
 */
typedef enum _STREAM_ID {
	STREAM_ID_UNKNOWN = 0,
	STREAM_ID_CODEC_IN,
	STREAM_ID_CODEC_OUT,
	STREAM_ID_LINUX_IN,
	STREAM_ID_LINUX_OUT,
	STREAM_ID_MAX
} STREAM_ID;

typedef struct _STREAM_INFO {
	STREAM_ID streamID;
	unsigned numChannels;
	unsigned numFrames;
	unsigned wordSize;
	bool flush;
	bool invalidate;
	void *data;
	void *altData;
} STREAM_INFO;

typedef struct _ROUTE_INFO {
	STREAM_ID srcID;
	STREAM_ID sinkID;
	unsigned srcOffset;
	unsigned sinkOffset;
	unsigned channels;
	unsigned channel_route;

} ROUTE_INFO;

#endif
