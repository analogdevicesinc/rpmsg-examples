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

#ifndef _rpmsg_init_h
#define _rpmsg_init_h

#include "rpmsg_lite.h"

/*
 * Declare endpoint info struct to keep endpoint pointer
 * and its rpmsg-lite istance (channel).
 * This is useful to pass as private pointer in rpmsg callback.
 * The private pointer can be used to pass other data to callback function.
 */

void rsc_table_init_and_wait(void);
int rpmsg_init_channel_to_ARM(void);
struct rpmsg_lite_instance *get_rpmsg_arm_channel(void);

#endif
