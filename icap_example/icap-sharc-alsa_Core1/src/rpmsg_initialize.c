/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */
#include "rpmsg_lite.h"
#include "rpmsg_arm_ep.h"
#include "icap_playback_device.h"
#include "icap_record_device.h"
#include "context.h"

// Create a struct where we can gather return values and view them easily within the debugger
// The final member, deadbeef, is a sanity check and is given a known value. If it doesn't have
// this value when viewed in the debugger, then something is out of  sync.
struct _initResults {
	int rsc_table_init_and_wait_completed;
	int waiting_completed;
	int res_rpmsg_init_echo_endpoint_to_ARM;
	int res_rpmsg_init_echo_cap_endpoint_to_ARM;
	int res_init_sharc_alsa_playback;
	int res_init_sharc_alsa_record;
	int deadbeef;
};

struct _initResults initResults = {
	0x9999, // rsc_table_init_and_wait_completed
	0x9999, // waiting_completed
	0x9999, // res_rpmsg_init_echo_endpoint_to_ARM
	0x9999, // res_rpmsg_init_echo_cap_endpoint_to_ARM
	0x9999, // res_init_sharc_alsa_playback
	0x9999, // res_init_sharc_alsa_record
	0xdeadbeef // deadbeef.
};

void rpmsg_initialize(void)
{
	int Result = 0u;

	rsc_table_init_and_wait();

	initResults.rsc_table_init_and_wait_completed = 1;

	if (Result == 0u) {
		Result = rpmsg_init_channel_to_ARM();
	}

	while (!rpmsg_lite_is_link_up(get_rpmsg_arm_channel())) {
		/* Wait until ARM notifies the channel is up */
	}

	initResults.waiting_completed = 1;

	if (Result == 0u) {
		Result = initResults.res_init_sharc_alsa_playback =
			init_sharc_alsa_playback();
	}
#ifdef ICAP_RECORD_EN
	if (Result == 0u) {
		Result = initResults.res_init_sharc_alsa_record =
			init_sharc_alsa_record();
	}
#endif
}
