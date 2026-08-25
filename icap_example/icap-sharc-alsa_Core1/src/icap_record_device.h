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

#ifndef ICAP_RECORD_DEVICE_H_
#define ICAP_RECORD_DEVICE_H_

#include <rpmsg_platform.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>
#include <icap_device.h>



extern struct rpmsg_lite_instance rpmsg_ARM_channel;
extern struct icap_instance icap_sharc_alsa_record;
extern struct icap_device_buffer icap_sharc_alsa_record_buffer;

int init_sharc_alsa_record(void);

#endif /* ICAP_RECORD_DEVICE_H_ */

