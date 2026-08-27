/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
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
