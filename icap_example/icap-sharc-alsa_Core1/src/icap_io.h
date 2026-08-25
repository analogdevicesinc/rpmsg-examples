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

#include "icap_playback_device.h"

#ifndef __ICAP_IO_H__
#define __ICAP_IO_H__

struct icap_device_buffer {
	struct icap_buf_descriptor descr;
	uint32_t offset;
	uint32_t frag_level;
	uint32_t buf_id;
	struct icap_instance *icap;
	uint32_t in_use;
	int32_t acks;
};

void update_frags(struct icap_device_buffer *buf, uint32_t frags);
void inc_buf_pos(struct icap_device_buffer *buf, uint32_t size);
int16_t get_s16(struct icap_device_buffer *buf);
void put_s16(struct icap_device_buffer *buf, int16_t data);
int32_t get_s32(struct icap_device_buffer *buf);
void put_s32(struct icap_device_buffer *buf, int32_t data);
int32_t icap_call_back(void *payload, uint32_t payload_len, uint32_t src,
		       void *priv);
void inc_buf_pos_capture(struct icap_device_buffer *buf, uint32_t size);

#endif
