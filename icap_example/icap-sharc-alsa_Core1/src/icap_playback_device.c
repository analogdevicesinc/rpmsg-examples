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
#include "icap_playback_device.h"

#include <sys/platform.h>
#include <sys/adi_core.h>
#include "context.h"
#include "context.h"

#define ICAP_SHARC_ALSA_PLAYBACK_EP_ADDRESS 21

struct icap_instance icap_sharc_alsa_playback;

struct rpmsg_lite_ept_static_context icap_sharc_alsa_playback_endpoint_context;

struct icap_device_buffer icap_sharc_alsa_playback_buffer = {.buf_id = 1};

struct icap_subdevice_features sport_playback_device_features = {
	.type = ICAP_DEV_PLAYBACK,
	.src_buf_max = 1,
	.dst_buf_max = 0,
	.channels_min = LINUX_AUDIO_IN_CHANNELS_MIN,
	.channels_max = LINUX_AUDIO_IN_CHANNELS_MAX,
	.formats = ICAP_FMTBIT_S32_LE,
	.rates = ICAP_RATE_48000,
};

uint32_t linux_in_channels = 0;

int32_t disable_playback_cache(void) {
	uint32_t start, end;
	int32_t ret;
	adi_cache_set_disable_range(adi_cache_rr5);

	start = icap_sharc_alsa_playback_buffer.descr.buf;
	end = start + icap_sharc_alsa_playback_buffer.descr.buf_size;

	// Disable cache for the descriptors memory range
	ret = adi_cache_set_range ((void *)start,
						(void *)(end),
						adi_cache_rr5,
						adi_cache_noncacheable_range);
	// The delay is required after cache is disabled
	platform_time_delay(200);
	return ret;
}


int create_icap_sharc_alsa_playback_endpoint(void)
{
	struct rpmsg_lite_endpoint *rpmsg_ept;
	int ret;

	rpmsg_ept = rpmsg_lite_create_ept(
			&rpmsg_ARM_channel,
			ICAP_SHARC_ALSA_PLAYBACK_EP_ADDRESS + adi_core_id(),
			&icap_call_back,
			(void*)&icap_sharc_alsa_playback,
			&icap_sharc_alsa_playback_endpoint_context);
	if(rpmsg_ept == RL_NULL){
		return -1;
	}
	return 0;
}

int announce_icap_sharc_alsa_playback_endpoint(void)
{
	struct rpmsg_lite_endpoint *rpmsg_ept = icap_sharc_alsa_playback.transport.rpmsg_ept;
	int ret;

	ret = rpmsg_ns_announce(
			&rpmsg_ARM_channel,
			rpmsg_ept,
			"sharc-alsa",
			RL_NS_CREATE);
	if(ret != RL_SUCCESS){
		return -1;
	}

	return 0;
}

int remove_icap_sharc_alsa_playback_endpoint(void)
{
	struct rpmsg_lite_endpoint *rpmsg_ept;
	int ret;

	ret = rpmsg_ns_announce(
			icap_sharc_alsa_playback.transport.rpmsg_instance,
			icap_sharc_alsa_playback.transport.rpmsg_ept,
			"sharc-alsa",
			RL_NS_DESTROY);

	if(ret) {
		return ret;
	}
	return rpmsg_lite_destroy_ept(
			icap_sharc_alsa_playback.transport.rpmsg_instance,
			icap_sharc_alsa_playback.transport.rpmsg_ept);
}


int32_t icap_sharc_alsa_playback_get_subdevices(struct icap_instance *icap)
{
	/* We have only one playback subdev */
	return 1;
}

/* Copy features from corresponding icap-linux-soport stream params*/
int32_t icap_sharc_alsa_playback_get_subdevice_features(struct icap_instance *icap, uint32_t subdev_id, struct icap_subdevice_features *features)
{
	if (subdev_id >= 1){
		return -ICAP_ERROR_INVALID;
	}
	memcpy(features, &sport_playback_device_features, sizeof(struct icap_subdevice_features));
	return 0;
}

int32_t icap_sharc_alsa_playback_start(struct icap_instance *icap, uint32_t subdev_id)
{
	if (subdev_id >= 1){
		return -ICAP_ERROR_INVALID;
	}
	icap_sharc_alsa_playback_buffer.acks = 0;
	icap_sharc_alsa_playback_buffer.in_use = 1;
	return 0;
}

int32_t icap_sharc_alsa_playback_stop(struct icap_instance *icap, uint32_t subdev_id)
{
	if (subdev_id >= 1){
		return -ICAP_ERROR_INVALID;
	}
	icap_sharc_alsa_playback_buffer.in_use = 0;
	return 0;
}

int32_t icap_sharc_alsa_playback_add_src(struct icap_instance *icap, struct icap_buf_descriptor *buf)
{

	memcpy(&icap_sharc_alsa_playback_buffer.descr, buf, sizeof(struct icap_buf_descriptor));
	linux_in_channels = icap_sharc_alsa_playback_buffer.descr.channels;
	icap_sharc_alsa_playback_buffer.offset = 0;
	icap_sharc_alsa_playback_buffer.frag_level = 0;
	icap_sharc_alsa_playback_buffer.icap = icap;
	icap_sharc_alsa_playback_buffer.in_use = 0;
	disable_playback_cache();
	return icap_sharc_alsa_playback_buffer.buf_id;
}

int32_t icap_sharc_alsa_playback_add_dst(struct icap_instance *icap, struct icap_buf_descriptor *buf)
{
	return -ICAP_ERROR_NOT_SUP;
}

int32_t icap_sharc_alsa_playback_frag_ready_response(struct icap_instance *icap, int32_t buf_id)
{
	if(icap_sharc_alsa_playback_buffer.buf_id == buf_id) {
		icap_sharc_alsa_playback_buffer.acks--;
	}
	return 0;
}


struct icap_device_callbacks icap_sharc_alsa_playback_callbacks = {
		.get_subdevices = icap_sharc_alsa_playback_get_subdevices,
		.get_subdevice_features = icap_sharc_alsa_playback_get_subdevice_features,
		.add_src = icap_sharc_alsa_playback_add_src,
		.add_dst = icap_sharc_alsa_playback_add_dst,
		.start = icap_sharc_alsa_playback_start,
		.stop = icap_sharc_alsa_playback_stop,
		.frag_ready_response = icap_sharc_alsa_playback_frag_ready_response,
};

int init_sharc_alsa_playback(void)
{
	int ret;
	ret = create_icap_sharc_alsa_playback_endpoint();
	if(ret) {
		return ret;
	}

	icap_sharc_alsa_playback.transport.rpmsg_instance = &rpmsg_ARM_channel;
	icap_sharc_alsa_playback.transport.rpmsg_ept = &icap_sharc_alsa_playback_endpoint_context.ept;
	ret = icap_device_init(&icap_sharc_alsa_playback, "playback", &icap_sharc_alsa_playback_callbacks, NULL);
	if(ret) {
		return ret;
	}

	ret = announce_icap_sharc_alsa_playback_endpoint();
	if(ret) {
		return ret;
	}
	return 0;
}

