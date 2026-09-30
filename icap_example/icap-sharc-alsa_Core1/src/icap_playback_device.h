/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */
#ifndef ICAP_PLAYBACK_DEVICE_H_
#define ICAP_PLAYBACK_DEVICE_H_

#include <rpmsg_platform.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>
#include <icap_device.h>

extern struct rpmsg_lite_instance rpmsg_ARM_channel;
extern struct icap_instance icap_sharc_alsa_playback;
extern struct icap_device_buffer icap_sharc_alsa_playback_buffer;

int init_sharc_alsa_playback(void);

#endif /* ICAP_PLAYBACK_DEVICE_H_ */
