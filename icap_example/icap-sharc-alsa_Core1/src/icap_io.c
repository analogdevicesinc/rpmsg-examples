/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */
#include <icap_io.h>
#include "icap_playback_device.h"
#include <sys/cache.h>
#include <stdint.h>
void update_frags(struct icap_device_buffer *buf, uint32_t frags)
{
	struct icap_buf_frags buf_frags;
	if (buf->descr.report_frags) {
		buf_frags.buf_id = buf->buf_id;
		buf_frags.frags = frags;
		icap_frag_ready(buf->icap, &buf_frags);
		buf->acks++;
	}
}

void inc_buf_pos(struct icap_device_buffer *buf, uint32_t size)
{
	int i;
	int8_t *addr;

	buf->offset += size;
	if (buf->offset >= buf->descr.buf_size) {
		buf->offset -= buf->descr.buf_size;
	}
	buf->frag_level += size;
	if (buf->frag_level >= buf->descr.frag_size) {
		buf->frag_level = 0;
		update_frags(buf, 1);
	}
}

void inc_buf_pos_capture(struct icap_device_buffer *buf, uint32_t size)
{
	int completed_frag;
	int total_frags;

	char *frag_start;
	char *frag_end;

	buf->offset += size;

	if (buf->offset >= buf->descr.buf_size) {
		buf->offset -= buf->descr.buf_size;
	}

	buf->frag_level += size;

	if (buf->frag_level >= buf->descr.frag_size) {
		buf->frag_level = 0;
		total_frags = buf->descr.buf_size / buf->descr.frag_size;

		completed_frag = buf->offset / buf->descr.frag_size;

		if (completed_frag == 0) {
			completed_frag = total_frags - 1;
		} else {
			completed_frag--;
		}

		frag_start = (char *)(uintptr_t)buf->descr.buf +
			     (completed_frag * buf->descr.frag_size);

		frag_end = (char *)frag_start + buf->descr.frag_size;

		flush_data_buffer(frag_start, frag_end, ADI_FLUSH_DATA_INV);
		update_frags(buf, 1);
	}
}

int16_t get_s16(struct icap_device_buffer *buf)
{
	int16_t *addr;
	int16_t data;
	if (!buf->in_use) {
		return 0;
	}
	addr = (int16_t *)((uint32_t)buf->descr.buf + buf->offset);
	data = *addr;
	inc_buf_pos(buf, sizeof(int16_t));
	return data;
}

void put_s16(struct icap_device_buffer *buf, int16_t data)
{
	int16_t *addr;
	if (!buf->in_use) {
		return;
	}
	addr = (int16_t *)((uint32_t)buf->descr.buf + buf->offset);
	*addr = data;
	inc_buf_pos(buf, sizeof(int16_t));
}

int32_t get_s32(struct icap_device_buffer *buf)
{
	int32_t *addr;
	int32_t data;
	if (!buf->in_use) {
		return 0;
	}

	addr = (int32_t *)((uint32_t)buf->descr.buf + buf->offset);
	data = *addr;
	inc_buf_pos(buf, sizeof(int32_t));
	return data;
}

void put_s32(struct icap_device_buffer *buf, int32_t data)
{
	int32_t *addr;
	if (!buf->in_use) {
		return;
	}

	addr = (int32_t *)((uint32_t)buf->descr.buf + buf->offset);
	*addr = data;
	inc_buf_pos_capture(buf, sizeof(int32_t));
}

#define _MIN(a, b) ((a) < (b) ? (a) : (b))
#define _MAX(a, b) ((a) > (b) ? (a) : (b))

int32_t icap_call_back(void *payload, uint32_t payload_len, uint32_t src,
		       void *priv)
{
	struct icap_instance *icap = (struct icap_instance *)priv;
	union icap_remote_addr src_addr;
	src_addr.rpmsg_addr = src;
	return icap_put_msg(icap, &src_addr, payload, payload_len);
}
