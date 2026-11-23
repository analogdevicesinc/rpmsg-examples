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

int main()
{
	int run=1;
	int i=0;

	// Initializes modules/components imported to the project
	adi_initComponents();
	
	(void)adi_rsc_hdr.initialized;
	(void)((volatile struct sharcfx_resource_table *)&rsc_table)->rpmsg_vdev.status;

	/* Begin adding your custom code here */
	while(1) {
		XT_WAITI(0);
	}

	return 0;
}

