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
#include <stdbool.h>

#if defined(__ADSPARM__)
#include <runtime/cache/adi_cache.h>
#else
#include <sys/cache.h>
#endif

#include "context.h"
#include "process_audio.h"
#include "icap_record_device.h"

#include "icap_io.h"
#include "linux_audio.h"
#include "route.h"
#include "gpio_pins.h"
#include <string.h>

#define syslog_print(...)
#define syslog_printf(...)

static STREAM_INFO STREAMS[STREAM_ID_MAX];

#pragma section("seg_l2_dmda_bw")
static SYSTEM_AUDIO_TYPE LINUX_OUT_BUFFER[2][LINUX_AUDIO_OUT_CHANNELS *
					     SYSTEM_BLOCK_SIZE] = { 0 };

#pragma section("seg_l2_dmda_bw")
static SYSTEM_AUDIO_TYPE LINUX_IN_BUFFER[2][LINUX_AUDIO_IN_CHANNELS_MAX *
					    SYSTEM_BLOCK_SIZE] = { 0 };

extern uint32_t linux_in_channels;

/* Optimize this module for speed */
#pragma optimize_for_speed

/* Invalidates incoming stream audio data if requested */
static void invalidateStreams(STREAM_INFO *streamInfo)
{
	STREAM_INFO *stream;
	size_t size;
	unsigned i;

	/* Invalidate all active streams if requested */
	for (i = 0; i < STREAM_ID_MAX; i++) {
		stream = &streamInfo[i];
		if ((stream->streamID != STREAM_ID_UNKNOWN)) {
			if (stream->flush && stream->invalidate) {
				size = (size_t)stream->numChannels *
				       stream->numFrames * stream->wordSize;
				flush_data_buffer(stream->data,
						  (char *)stream->data + size,
						  ADI_FLUSH_DATA_INV);
			}
		}
	}
}

/* Flushes outgoing stream audio data if requested */
static void flushStreams(STREAM_INFO *streamInfo)
{
	STREAM_INFO *stream;
	size_t size;
	unsigned i;

	/* Flush all active streams if requested */
	for (i = 0; i < STREAM_ID_MAX; i++) {
		stream = &streamInfo[i];
		if ((stream->streamID != STREAM_ID_UNKNOWN)) {
			if (stream->flush && !stream->invalidate) {
				size = (size_t)stream->numChannels *
				       stream->numFrames * stream->wordSize;
				flush_data_buffer(stream->data,
						  (char *)stream->data + size,
						  ADI_FLUSH_DATA_NOINV);
			}
		}
	}
}

/**
 * Clears (zeros) a section of the audio stream buffer for the given stream ID.
 *
 * This function sets the specified number of channels to zero for each frame,
 * starting at the given sinkOffset, for both the primary and alternate data buffers.
 *
 * Parameters:
 *   streamID   - The stream to clear (must not be STREAM_ID_UNKNOWN)
 *   sinkOffset - The starting channel offset in the buffer to clear
 *   channels   - The number of channels to clear per frame
 */
void clearStreamBuffer(STREAM_ID streamID, unsigned sinkOffset,
		       unsigned channels)
{
	STREAM_INFO *stream;
	size_t size;
	unsigned ch;

	// If the stream ID is invalid, do nothing
	if (streamID == STREAM_ID_UNKNOWN) {
		return;
	}

	// Get the stream info structure for the given stream ID
	stream = &STREAMS[streamID];

	// Calculate the total size to clear per frame (not used directly below)
	size = (size_t)channels * stream->numFrames * stream->wordSize;

	// Loop over both data buffers: primary (data) and alternate (altData)
	for (int d = 0; d < 2; ++d) {
		void *data = d == 0 ? stream->data : stream->altData;
		// Skip if buffer pointer is NULL
		if (data == NULL) {
			continue;
		}
		// For each frame, zero out the specified channels starting at sinkOffset
		for (int frame = 0; frame < stream->numFrames; frame++) {
			// Calculate the starting address for this frame and offset
			memset((char *)data + (sinkOffset +
					       frame * stream->numChannels) *
						      stream->wordSize,
			       0, (size_t)channels * stream->wordSize);
		}
	}
}

/* Routes audio between sources and sinks */

static void routeAudio(STREAM_INFO *streamInfo, unsigned numStreams,
		       ROUTE_INFO *routeInfo, unsigned numRoutes)
{
	ROUTE_INFO *route;
	STREAM_INFO *src, *sink;
	unsigned channels;
	SYSTEM_AUDIO_TYPE *in;
	unsigned inChannel, outChannel;
	unsigned frame;
	unsigned channel;
	SYSTEM_AUDIO_TYPE sample;

	unsigned attenuationShift;
	int32_t *ip_ptr;
	int32_t *out_ptr;
	unsigned channel_route;

	route = &routeInfo[0];

	src = &streamInfo[route->srcID];
	sink = &streamInfo[route->sinkID];

	if ((src->data == NULL) || (sink->data == NULL)) {
		return;
	}

	inChannel = route->srcOffset;
	outChannel = route->sinkOffset;

	channels = route->channels;
	channel_route = route->channel_route;
	in = (SYSTEM_AUDIO_TYPE *)src->data + inChannel;
	int32_t *out32 = (int32_t *)sink->data + outChannel;
	int16_t *out16 = (int16_t *)sink->data + outChannel;

	ip_ptr = (int32_t *)sink->data + outChannel;
	out_ptr = (int32_t *)sink->data + outChannel;

	for (frame = 0; frame < src->numFrames; frame++) {
		for (channel = 0; channel < channels; channel++) {
			if ((outChannel + channel) < sink->numChannels) {
				if ((inChannel + channel) < src->numChannels) {
					sample = *(in + channel);
				} else {
					sample = 0;
				}
				if (sink->wordSize == sizeof(int32_t)) {
					*(out32 + channel) = sample;

				} else {
					*(out16 + channel) = sample >> 16;
				}
			}
		}
		in += src->numChannels;
		out32 += sink->numChannels;
	}

	/* This routes data from channels 4�16 to DAC outputs 1�12.*/

	if (channel_route == 1) {
		for (int fr = 0; fr < src->numFrames; fr++) {
			for (int in_ch = 4, op_ch = 0;
			     in_ch < sink->numChannels; in_ch++, op_ch++) {
				out_ptr[fr * sink->numChannels + op_ch] =
					ip_ptr[fr * sink->numChannels + in_ch];
			}

			for (int op_ch = 12; op_ch < sink->numChannels;
			     op_ch++) {
				out_ptr[fr * sink->numChannels + op_ch] = 0;
			}
		}
	}

#ifdef ICAP_RECORD_EN

	if (icap_sharc_alsa_playback_buffer.in_use == 1) {
		for (int i = 0; i < SYSTEM_BLOCK_SIZE; i++) {
			for (int j = 0; j < LINUX_AUDIO_OUT_CHANNELS; ++j) {
				put_s32(&icap_sharc_alsa_record_buffer,
					out_ptr[i * LINUX_AUDIO_OUT_CHANNELS +
						j]);
			}
		}
	}

#endif
}

static void inline setStreamInfo(STREAM_ID streamID, unsigned numChannels,
				 unsigned numFrames, unsigned wordSize,
				 void *data, bool flush, bool invalidate)
{
	STREAM_INFO *streamInfo;
	streamInfo = STREAMS + streamID;
	streamInfo->streamID = streamID;
	streamInfo->numChannels = numChannels;
	streamInfo->numFrames = numFrames;
	streamInfo->wordSize = wordSize;
	streamInfo->altData = streamInfo->data != NULL ? streamInfo->data :
							 data;
	streamInfo->data = data;
	streamInfo->flush = flush;
	streamInfo->invalidate = invalidate;
}

#define STREAM2BIT(x) (1 << x)
#define ALL_STREAMS \
	(STREAM2BIT(STREAM_ID_CODEC_IN) | STREAM2BIT(STREAM_ID_CODEC_OUT))

/*
 * This function processes audio when all physical source/sinks are ready.
 */
void processAudio(APP_CONTEXT *context, STREAM_ID streamID,
		  unsigned numChannels, unsigned numFrames, unsigned wordSize,
		  void *data, bool flush)
{
	static unsigned pingPong = 0;

	/* Store stream info.  The SPORT driver already invalidates incoming buffers. */
	setStreamInfo(streamID, numChannels, numFrames, wordSize, data, flush,
		      false);

	linuxAudioIn(LINUX_IN_BUFFER[pingPong],
		     (linux_in_channels * sizeof(SYSTEM_AUDIO_TYPE) *
		      SYSTEM_BLOCK_SIZE),
		     context);

	/* Configure "clockless" streams */
	setStreamInfo(STREAM_ID_LINUX_IN, linux_in_channels, SYSTEM_BLOCK_SIZE,
		      sizeof(SYSTEM_AUDIO_TYPE), LINUX_IN_BUFFER[pingPong],
		      false, false);
	/* Switch to the "inactive" buffers */
	pingPong = pingPong ? 0 : 1;

	invalidateStreams(STREAMS);
	routeAudio(STREAMS, STREAM_ID_MAX, context->routingTable,
		   MAX_AUDIO_ROUTES);
	flushStreams(STREAMS);
}
