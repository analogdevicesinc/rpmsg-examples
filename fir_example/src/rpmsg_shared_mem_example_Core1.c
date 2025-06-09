/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2023, Analog Devices, Inc. All rights reserved. 
*/

/*****************************************************************************
 * rpmsg_echo_example_Core1.c
 *****************************************************************************/
#include <string.h>

#include <sys/platform.h>
#include <sys/adi_core.h>
#include <stdlib.h>

#include <rpmsg_platform.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>

#include "fir_processing.h"
#include <drivers/fir/adi_fir.h>

#include <services/int/adi_sec.h>

#include "adi_initialize.h"
#include "rpmsg_shared_mem_example_Core1.h"

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

int run=1;

#define ADI_RESOURCE_TABLE_INIT_MAGIC (0xADE0AD0E)
#define ADI_RESOURCE_TABLE_SHARC1_OFFSET (0x400) //1KiB
/*
 * Expected resource table layout in the shared memory.
 * Initialized by ARM.
 */
RL_PACKED_BEGIN
struct sharc_resource_table {
	struct resource_table table_hdr;
	unsigned int offset[1];
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring vring[2];
}RL_PACKED_END;

RL_PACKED_BEGIN
struct adi_resource_table{
	uint8_t tag[16];
	uint32_t version;
	uint32_t initialized;
	uint32_t reserved[8];

	struct sharc_resource_table tbl;
}RL_PACKED_END;


const struct adi_resource_table rsc_tbl_local = {
		.tag = "AD-RESOURCE-TBL",
		.version = 1,
		.initialized = 0,

		.tbl.table_hdr = {
			/* resource table header */
			1, 								 /* version */
			1, /* number of table entries */
			{0, 0,},					 /* reserved fields */
		},
		.tbl.offset = {offsetof(struct sharc_resource_table, rpmsg_vdev),
		},
		.tbl.rpmsg_vdev = {RSC_VDEV, /* virtio dev type */
			7, /* it's rpmsg virtio */
			1, /* kick sharc0 */
			/* 1<<0 is VIRTIO_RPMSG_F_NS bit defined in virtio_rpmsg_bus.c */
			1<<0, 0, 0, 0, /* dfeatures, gfeatures, config len, status */
			2, /* num_of_vrings */
			{0, 0,}, /* reserved */
		},
		.tbl.vring = {
			{(uint32_t)-1, VRING_ALIGN, 512, 1, 0}, /* da allocated by remoteproc driver */
			{(uint32_t)-1, VRING_ALIGN, 512, 1, 0}, /* da allocated by remoteproc driver */
		},
};

/*
 * Two resource tables, one for each core.
 * The ___MCAPI_common_start address is defined in app.ldf
 */
extern "asm" struct adi_resource_table ___MCAPI_common_start;
volatile struct adi_resource_table *adi_resource_table;
volatile struct sharc_resource_table *resource_table;

/*
 * Rpmsg endpoints addresses.
 * Each end point on a rpmsg channel should have unique address
 */
#define EP_NUM_PER_CHAN 0x1
#define EP_CORE1_OFFSET 0x20
#define ECHO_CHEW_MEM_ADDRESS 0x1

/* Static variables for rpmsg-lite */
struct rpmsg_lite_instance rpmsg_ARM_channel;
struct rpmsg_lite_ept_static_context sharc_ARM_chew_mem_endpoint_context[EP_NUM_PER_CHAN];

/*
 * Declare endpoint info struct to keep endpoint pointer
 * and its rpmsg-lite istance (channel).
 * This is useful to pass as private pointer in rpmsg callback.
 * The private pointer can be used to pass other data to callback function.
 */
struct rpmsg_ep_info{
	struct rpmsg_lite_instance *rpmsg_instance;
	struct rpmsg_lite_endpoint *rpmsg_ept;
};

/*
const struct rpmsg_ep_info rpmsg_chew_mem_ep_to_ARM = {
		.rpmsg_instance = &rpmsg_ARM_channel,
		.rpmsg_ept = &sharc_ARM_chew_mem_endpoint_context.ept,
};
*/

struct rpmsg_ep_info rpmsg_chew_mem_ep_to_ARM[EP_NUM_PER_CHAN];

/*
 * Local rpmsg queue to offload message handling in main loop instead in interrupt.
 * The queue uses rpmsg-lite zero copy feature.
 */
#define MAX_RPMSG_COUNT 16
typedef uint16_t sm_atomic_t;
struct _rpmsg_msg{
	uint8_t *payload;
	uint32_t payload_len;
	uint32_t src;
	void *priv;
};
volatile struct _rpmsg_msg rpmsg_msg_queue[MAX_RPMSG_COUNT];
volatile sm_atomic_t rpmsg_msg_queue_head = 0;
volatile sm_atomic_t rpmsg_msg_queue_tail = 0;

/*
 * Helper struct which represents memory ranges used by a vring.
 */
struct _mem_range{
	uint32_t start;
	uint32_t end;
};

/*
 * Helper function which reads memory ranges used by a vring.
 */
void vring_get_descriptor_range(volatile struct fw_rsc_vdev_vring *vring, struct _mem_range *range){
	struct vring_desc *desc = (struct vring_desc *)vring->da;
	range->start = (uint32_t)desc;
	range->end = (uint32_t)desc + vring_size(vring->num, vring->align);
}
void vring_get_buffer_range(volatile struct fw_rsc_vdev_vring *vring, struct _mem_range *range){
	struct vring_desc *desc = (struct vring_desc *)vring->da;
	uint32_t num = 2 * vring->num; // vring0 descriptor has pointer to buffers for both vrings
	range->start = (uint32_t)desc->addr;
	range->end = (uint32_t)desc->addr + num * (RL_BUFFER_PAYLOAD_SIZE +16);
}

void init_rsc_tbl(void) {

	switch(adi_core_id()){
	case ADI_CORE_ARM:
		return;
	case ADI_CORE_SHARC0:
		adi_resource_table = &___MCAPI_common_start;
		resource_table = &___MCAPI_common_start.tbl;
		break;
	case ADI_CORE_SHARC1:
		adi_resource_table = (struct adi_resource_table *)
			((uint32_t)&___MCAPI_common_start + ADI_RESOURCE_TABLE_SHARC1_OFFSET);
		resource_table = &adi_resource_table->tbl;
		break;
	default:
		// should never happen
		break;
	}

	/* Don't initialize if remoteproc driver has already */
	if(strcmp((const char *)adi_resource_table->tag, (const char *)rsc_tbl_local.tag)){
		*adi_resource_table = rsc_tbl_local;

		switch(adi_core_id()){
		case ADI_CORE_ARM:
			return;
		case ADI_CORE_SHARC0:
			adi_resource_table->tbl.rpmsg_vdev.notifyid = 1;
			adi_resource_table->tbl.vring[0].notifyid = 1;
			adi_resource_table->tbl.vring[1].notifyid = 1;
			break;
		case ADI_CORE_SHARC1:
			adi_resource_table->tbl.rpmsg_vdev.notifyid = 2;
			adi_resource_table->tbl.vring[0].notifyid = 2;
			adi_resource_table->tbl.vring[1].notifyid = 2;
			break;
		default:
			// should never happen
			break;
		}

	}
}

int rsc_tbl_ready(void) {
	/* 0x1 acknowledge, 0x2 driver found, 0x4 driver ready*/
	return resource_table->rpmsg_vdev.status == 7;
}

/*
 * Initialize rpmsg channel to ARM core
 */
int rpmsg_init_channel_to_ARM(void){
	struct rpmsg_lite_instance *rpmsg_instance;
	adiCacheStatus status;
	struct _mem_range range0;
	struct _mem_range range1;

	init_rsc_tbl();
	while(!rsc_tbl_ready()){
		/* Wait for resource table to be initialized by ARM*/
	}

	// Get memory range which needs disabled cache
	// Read vring descriptors memory range
	vring_get_descriptor_range(&resource_table->vring[0], &range0);
	vring_get_descriptor_range(&resource_table->vring[1], &range1);
	range0.start = min(range0.start, range1.start);
	range0.end = max(range0.end, range1.end);
	// Disable cache for the descriptors memory range
	status = adi_cache_set_range ((void *)range0.start,
						(void *)(range0.end),
						adi_cache_rr6,
						adi_cache_noncacheable_range);

	// Read vring buffer memory range
	// vring1 has its own descriptors but share buffers with vring0
	vring_get_buffer_range(&resource_table->vring[0], &range1);
	// Disable cache for the vring buffer range
	status = adi_cache_set_range ((void *)range1.start,
						(void *)(range1.end),
						adi_cache_rr7,
						adi_cache_noncacheable_range);

	rpmsg_instance = rpmsg_lite_remote_init(
			(void*)&resource_table->rpmsg_vdev,
			RL_PLATFORM_SHARC_ARM_LINK_ID,
			RL_SHM_VDEV,
			&rpmsg_ARM_channel);
	if(rpmsg_instance == RL_NULL){
		return -1;
	}

	adi_resource_table->initialized = ADI_RESOURCE_TABLE_INIT_MAGIC;

	/*
	 * Wait until ARM notifies the channel is up.
	 */
	while(!rpmsg_lite_is_link_up(rpmsg_instance));
	return 0;
}


/*
 * Keep ingesting data until buffer is full.
 * Once done, send a reply that all is well and you are
 * chewing through it.
 */

/* 0x0 = not initialized, > 0x0 = init done but not full, sizeof(buffer) = init and full */
uint32_t input_buf_fill = 0; //tracks nr of elements in input/output buf
uint32_t output_buf_fill = FIR_WINDOW_SIZE;
uint32_t output_packet_size = 0;

float *FirInputBuff3;
float output_data[FIR_WINDOW_SIZE];
int max_nr_floats = (RL_BUFFER_PAYLOAD_SIZE/sizeof(float));

#define CM_IN_PROGRESS	0xF
#define CM_TRANSMIT_OP	0xE
#define CM_INGEST_IP	0x1

void chew(float *data, uint32_t data_len, char *res)
{
	int requested_data_size;
	uint32_t nr_transfers, output_offset;


	/* If data is ready, go ahead and dump
	 * the stats:
	 *
	 * no of values verified
	 * time taken for accelerator to compute
	 * Verification pass/fail
	 *
	 * Else
	 *
	 * return string "Processing data"
	 * */

	switch (run) {

		case CM_IN_PROGRESS:

			//run has not been reset, FIR has probably not returned
			res[0] = 'F';
			break;


		//run has been reset, start sending out the buffer contents
		case CM_TRANSMIT_OP:

			//start sending data out on each read,
			//assuming sizeof(res) is >= RL_BUFFER_PAYLOAD_SIZE*sizeof(float)
			nr_transfers = 0;
			output_offset = FIR_WINDOW_SIZE - output_buf_fill; //total data - data left to be transmitted = data transmitted

			if(output_buf_fill > max_nr_floats)
				nr_transfers = max_nr_floats;
			else
				nr_transfers = output_buf_fill;

			for(unsigned int i=0; i<nr_transfers; i++)
				*((float *)res + i) = output_data[output_offset + i];

			output_buf_fill -= nr_transfers; //decrease the data to be transferred
			output_packet_size = nr_transfers * sizeof(float);
			break;

		case CM_INGEST_IP:

			if (!FirInputBuff3)
				FirInputBuff3 = (float *) malloc(TAPS2+FIR_WINDOW_SIZE-1);

			/* Fill input buff with all input taps, once full, next message kicks off FIR */
			while (input_buf_fill < TAPS2)
			{
				for(int i=0; i<data_len/sizeof(float); i++)
					*(FirInputBuff3 + input_buf_fill + i) = *((float *)data + i);

				input_buf_fill += data_len/sizeof(float);

				requested_data_size = (TAPS2 - input_buf_fill)*sizeof(float);
				snprintf(res, 10, "%d\n", requested_data_size);

				if (requested_data_size == 0) {
					run = CM_IN_PROGRESS;
					output_buf_fill = FIR_WINDOW_SIZE; //prepare for sending output
					input_buf_fill = 0; //reset for next input
				}

				return;
			}

			break;

		default:
			return;

	}


}

/*
 * Rpmsg callback function which can be assigned to multiple endpoints.
 * It executes in interrupt context.
 *
 * This function expects the co-processor to reveal a physical memory location and
 * size of data via RPMsg (0x<32_bit_addr_in_hex> <nr_bytes_in_decimal>).
 * Then, the processor "chews" through the data to write back the result at the same location.
 *
 * Once this is completed, the processor then notifies the co-processor about the
 * completion of the task.
 */
int32_t chew_mem_call_back(void *payload, uint32_t payload_len, uint32_t src, void *priv){
	struct rpmsg_ep_info *_rpmsg_ep_info = (struct rpmsg_ep_info *)priv;
	char *data=payload;
	char reply[RL_BUFFER_PAYLOAD_SIZE];

	int32_t ret;
	uint32_t reply_size;

	if (payload_len < RL_BUFFER_PAYLOAD_SIZE)
		data[payload_len] = '\0';

	chew(payload, payload_len, reply);

	payload = reply;

	if (run == CM_TRANSMIT_OP) {
		reply_size = output_packet_size;

		//This needs to be here to make sure the last packet does not
		//end up being in the else condition
		if (output_buf_fill <= 0)
			run = CM_INGEST_IP;
	} else {
		reply_size = strlen(payload);
	}


	// Send the message back to its origin endpoint
	ret = rpmsg_lite_send(
		_rpmsg_ep_info->rpmsg_instance,
		_rpmsg_ep_info->rpmsg_ept,
		src,
		payload,
		reply_size,
		100);

	if (ret < 0)
		return RL_NOT_READY;

	return RL_SUCCESS;
}

/*
 * Create first endpoint on the rpmsg channel and announce its existence.
 * Core id is added to ECHO_EP_ADDRESS so the address is different for each core.
 */
int rpmsg_init_chew_mem_endpoint_to_ARM(int channel){
	struct rpmsg_lite_endpoint *rpmsg_ept;
	int ret;

	rpmsg_ept = rpmsg_lite_create_ept(
			&rpmsg_ARM_channel,
			ECHO_CHEW_MEM_ADDRESS + adi_core_id()*EP_CORE1_OFFSET + channel,
			&chew_mem_call_back,
			(void*)&rpmsg_chew_mem_ep_to_ARM[channel],
			&sharc_ARM_chew_mem_endpoint_context[channel]);
	if(rpmsg_ept == RL_NULL){
		return -1;
	}

	ret = rpmsg_ns_announce(
			&rpmsg_ARM_channel,
			rpmsg_ept,
			"sharc-chew_mem",
			RL_NS_CREATE);
	if(ret != RL_SUCCESS){
		return -1;
	}
	return 0;
}

#define ADI_SEC_SSI_STRIDE          1u
/*
 * Check if watchdog was enabled. This to fix issues on Linux side, when
 * watchdog is enabled from a driver and SHARC Core's reset it when firmware is loaded
 */
bool adi_sec_Watchdog0_Enabled(void) {

	volatile uint32_t *pSCTL;
	uint32_t secSCTL;

	/* SID used as offset to point at correct SCTL for WDOG0 */
	pSCTL = pREG_SEC0_SCTL0 + (INTR_WDOG0_EXP << ADI_SEC_SSI_STRIDE);

	/* Get the contents of source control register */
	secSCTL = *pSCTL;

	if ((secSCTL & (BITM_SEC_FCTL_SREN|BITM_SEC_SCTL_FEN)) == (BITM_SEC_FCTL_SREN|BITM_SEC_SCTL_FEN)) {
		return true;
	}

	return false;
}


int main(int argc, char *argv[])
{
	int i=0;
	bool wdt_set=false;

	//check if watchdog SEC was set, fixes if ARM Linux drivers using the watchdog
	wdt_set = adi_sec_Watchdog0_Enabled();

	// Initializes modules/components imported to the project
	adi_initComponents();

	if (wdt_set) {
		adi_sec_EnableSystemReset(true);
		adi_sec_EnableSource(INTR_WDOG0_EXP, 1);
		adi_sec_EnableFault(INTR_WDOG0_EXP, 1);
		adi_sec_EnableSFI(true);
	}

	//Initialise endpoints
	for (i=0;i<EP_NUM_PER_CHAN;i++)  {
		rpmsg_chew_mem_ep_to_ARM[i].rpmsg_instance = &rpmsg_ARM_channel;
		rpmsg_chew_mem_ep_to_ARM[i].rpmsg_ept = &sharc_ARM_chew_mem_endpoint_context[i].ept;
	}

	// Initialize rpmsg channel to the ARM core and create endpoints
	rpmsg_init_channel_to_ARM();
	for (i=0;i<EP_NUM_PER_CHAN;i++)
		rpmsg_init_chew_mem_endpoint_to_ARM(i);

	while(run) {

		//if not in progress then we are either sending
		//output or taking input
		if (run != CM_IN_PROGRESS)
			continue;

		/*************** FIR TASKS ***************/
		if (execute_single_channel(output_data))
			break;

		//ready to transmit output
		run = CM_TRANSMIT_OP;
		/*************** FIR TASKS ***************/
	}

	// Close notify system we are about to close endpoints.
	for (i=0;i<EP_NUM_PER_CHAN;i++) {
		rpmsg_ns_announce(
				rpmsg_chew_mem_ep_to_ARM[i].rpmsg_instance,
				rpmsg_chew_mem_ep_to_ARM[i].rpmsg_ept,
				"sharc-chew_mem",
				RL_NS_DESTROY);
	}

	// Close endpoints and the rpmsg channel
	for (i=0;i<EP_NUM_PER_CHAN;i++) {
		rpmsg_lite_destroy_ept(rpmsg_chew_mem_ep_to_ARM[i].rpmsg_instance, rpmsg_chew_mem_ep_to_ARM[i].rpmsg_ept);
		rpmsg_lite_deinit(&rpmsg_ARM_channel);
	}
	return 0;
}

