/*****************************************************************************
 * rpmsg_echo_example_Core1.c
 *****************************************************************************/
#include <string.h>

#include <sys/platform.h>
#include <sys/adi_core.h>

#include <rpmsg_platform.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>

#include <services/int/adi_sec.h>

#include "adi_initialize.h"
#include "rpmsg_echo_example_Core1.h"

#include <sys/platform.h>

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

#define ADI_RESOURCE_TABLE_TAG "AD-RESOURCE-TBL"
#define ADI_RESOURCE_TABLE_VERSION (1U)
#define ADI_RESOURCE_TABLE_INIT_MAGIC (0xADE0AD0E)
#define ADI_RESOURCE_TABLE_SHARC1_OFFSET (0x400) //1KiB

#define RSC_VRING0_ADDR (0x20400000U)
#define RSC_VRING1_ADDR (0x20402000U)
#define RSC_VRING_NUM (128U) /* Number of descriptors Linux set up in an 8KB vriwng with 4KB alignment */

/*
 * Expected resource table layout in the shared memory.
 * Initialized by ARM.
 */
RL_PACKED_BEGIN
struct sharcfx_resource_table {
	struct resource_table table_hdr;
	unsigned int offset[1];
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring vring[2];
}RL_PACKED_END;

RL_PACKED_BEGIN
struct adi_resource_table {
	uint8_t tag[16];
	uint32_t version;
	uint32_t initialized;
	uint32_t reserved[8];
}RL_PACKED_END;

volatile struct adi_resource_table adi_rsc_hdr
	__attribute__((section(".adi_rsc_hdr"), aligned(4))) = {
			.tag = ADI_RESOURCE_TABLE_TAG,
			.version = ADI_RESOURCE_TABLE_VERSION,
			.initialized = 0,
	};

struct sharcfx_resource_table rsc_table
	__attribute((section(".resource_table"), aligned(4))) = {
		.table_hdr = {
			/* resource table header */
			.ver = 1, /* version */
			.num = 1, /* number of table entries */
		},
		.offset = {
				offsetof(struct sharcfx_resource_table, rpmsg_vdev)
		},
		.rpmsg_vdev = {
			.type = RSC_VDEV, /* virtio dev type */
			.id = 7, /* it's rpmsg virtio */
			.notifyid = 0, /* unused */
			.dfeatures = 1 << 0,
			.num_of_vrings = 2,

		},
		.vring = {
			{
				.da = RSC_VRING0_ADDR,
				.align = VRING_ALIGN,
				.num = RSC_VRING_NUM,
				.notifyid = 0,
			},
			{
				.da = RSC_VRING1_ADDR,
				.align = VRING_ALIGN,
				.num = RSC_VRING_NUM,
				.notifyid = 1,
			},
		},
};

volatile struct sharcfx_resource_table *resource_table =
        (volatile struct sharcfx_resource_table *)&rsc_table;

/*
 * Rpmsg endpoints addresses.
 * Each end point on a rpmsg channel should have unique address
 */
#define EP_NUM_PER_CHAN 0x20
#define EP_CORE1_OFFSET 0x20
#define ECHO_EP_ADDRESS 0x100
#define ECHO_CAP_EP_ADDRESS 0x140

/* Static variables for rpmsg-lite */
struct rpmsg_lite_instance rpmsg_ARM_channel;
struct rpmsg_lite_ept_static_context sharc_ARM_echo_endpoint_context[EP_NUM_PER_CHAN];
struct rpmsg_lite_ept_static_context sharc_ARM_echo_cap_endpoint_context[EP_NUM_PER_CHAN];

struct rpmsg_ep_info{
	struct rpmsg_lite_instance *rpmsg_instance;
	struct rpmsg_lite_endpoint *rpmsg_ept;
};

struct rpmsg_ep_info rpmsg_echo_ep_to_ARM[EP_NUM_PER_CHAN];
struct rpmsg_ep_info rpmsg_echo_cap_ep_to_ARM[EP_NUM_PER_CHAN];

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

int rsc_tbl_ready(void) {
	/* 0x1 acknowledge, 0x2 driver found, 0x4 driver ready*/
	return resource_table->rpmsg_vdev.status == 7;
}

/*
 * Initialize rpmsg channel to ARM core
 */
int rpmsg_init_channel_to_ARM(void){
	struct rpmsg_lite_instance *rpmsg_instance;

	while(!rsc_tbl_ready()){
		/* Wait for resource table to be initialized by ARM*/
	}

	rpmsg_instance = rpmsg_lite_remote_init(
			(void*)&resource_table->rpmsg_vdev, RL_PLATFORM_SHARC_ARM_LINK_ID,
			RL_SHM_VDEV, &rpmsg_ARM_channel);

	if(rpmsg_instance == RL_NULL){
		return -1;
	}

	adi_rsc_hdr.initialized = ADI_RESOURCE_TABLE_INIT_MAGIC;

	/*
	 * Wait until ARM notifies the channel is up.
	 */
	while(!rpmsg_lite_is_link_up(rpmsg_instance));

	return 0;
}

int main()
{
	int run=1;
	int i=0;

	// Initializes modules/components imported to the project
	adi_initComponents();
	
	(void)adi_rsc_hdr.initialized;
	(void)((volatile struct sharcfx_resource_table *)&rsc_table)->rpmsg_vdev.status;

	//Initialise endpoints
	for (i=0;i<EP_NUM_PER_CHAN;i++)  {
		rpmsg_echo_ep_to_ARM[i].rpmsg_instance = &rpmsg_ARM_channel;
		rpmsg_echo_ep_to_ARM[i].rpmsg_ept = &sharc_ARM_echo_endpoint_context[i].ept;
	}
	for (i=0;i<EP_CORE1_OFFSET;i++)  {
		rpmsg_echo_cap_ep_to_ARM[i].rpmsg_instance = &rpmsg_ARM_channel;
		rpmsg_echo_cap_ep_to_ARM[i].rpmsg_ept = &sharc_ARM_echo_cap_endpoint_context[i].ept;
	}

	// Initialize rpmsg channel to the ARM core and create two endpoints
	rpmsg_init_channel_to_ARM();
	for (i=0;i<EP_NUM_PER_CHAN;i++) {
		//rpmsg_init_echo_endpoint_to_ARM(i);
		//rpmsg_init_echo_cap_endpoint_to_ARM(i);
	}

	/* Begin adding your custom code here */
	while(1) {
		XT_WAITI(0);
	}

	return 0;
}

