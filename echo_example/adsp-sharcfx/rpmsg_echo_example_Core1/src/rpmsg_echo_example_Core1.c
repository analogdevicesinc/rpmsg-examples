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

volatile struct _rpmsg_msg rpmsg_msg_queue[MAX_RPMSG_COUNT];
volatile sm_atomic_t rpmsg_msg_queue_head = 0;
volatile sm_atomic_t rpmsg_msg_queue_tail = 0;

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

/*
 * Rpmsg callback function wich can be assigned to multiple endpoints.
 * It executes in interrupt context.
 */
int32_t echo_call_back(void *payload, uint32_t payload_len, uint32_t src, void *priv){
	struct rpmsg_ep_info *_rpmsg_ep_info = (struct rpmsg_ep_info *)priv;
	char* data=payload;
	char append_msg[32];
	int32_t ret;

	// put in string ending for strcat_s
	if (payload_len < RL_BUFFER_PAYLOAD_SIZE)
		data[payload_len] = '\0';

	// Attach sharc core info to the response
	snprintf(append_msg, sizeof(append_msg), " => echo from Core%d\n", adi_core_id());
	strcat_s(payload, RL_BUFFER_PAYLOAD_SIZE, append_msg);

	// Send the message back to its origin endpoint
	ret = rpmsg_lite_send(
		_rpmsg_ep_info->rpmsg_instance,
		_rpmsg_ep_info->rpmsg_ept,
		src,
		payload,
		strlen(payload),
		100);
	if (ret < 0){
		//handle error
	}
	return RL_SUCCESS;
}

/*
 * Create first endpoint on the rpmsg channel and announce its existence.
 * Core id is added to ECHO_EP_ADDRESS so the address is different for each core.
 */
int rpmsg_init_echo_endpoint_to_ARM(int channel){
	struct rpmsg_lite_endpoint *rpmsg_ept;
	int ret;

	rpmsg_ept = rpmsg_lite_create_ept(
			&rpmsg_ARM_channel,
			ECHO_EP_ADDRESS + adi_core_id()*EP_CORE1_OFFSET + channel,
			&echo_call_back,
			(void*)&rpmsg_echo_ep_to_ARM[channel],
			&sharc_ARM_echo_endpoint_context[channel]);
	if(rpmsg_ept == RL_NULL){
		return -1;
	}

	ret = rpmsg_ns_announce(
			&rpmsg_ARM_channel,
			rpmsg_ept,
			"sharc-echo",
			RL_NS_CREATE);
	if(ret != RL_SUCCESS){
		return -1;
	}
	return 0;
}

/*
 * Rpmsg callbacks execute in interrupt context.
 * This rpmsg callback function returns RL_HOLD to tell rpmsg-lite
 * to hold message buffer for later processing, zero-copy feature.
 * The message can be put into queue for processing in idle loop instead of interrupt.
 *
 * After the message is processed in needs to be returned to the pool using:
 * rpmsg_lite_release_rx_buffer()
 */
int32_t echo_cap_call_back(void *payload, uint32_t payload_len, uint32_t src, void *priv){
	sm_atomic_t head_next = rpmsg_msg_queue_head + 1;
	if (head_next >= MAX_RPMSG_COUNT){
		head_next = 0;
	}

	// Check if queue is full
	if(head_next == rpmsg_msg_queue_tail){
		return RL_ERR_NO_BUFF; //drop the message
	}

	// put the message to the queue
	rpmsg_msg_queue[head_next].payload = payload;
	rpmsg_msg_queue[head_next].payload_len = payload_len;
	rpmsg_msg_queue[head_next].src = src;
	rpmsg_msg_queue[head_next].priv = priv;
	rpmsg_msg_queue_head = head_next;

	// Holds the buffer for later process, zero copy.
	return RL_HOLD;
}

/*
 * This function is called from the idle loop, processes message received in echo_cap_call_back.
 * After message is processed the message buffer is released using rpmsg_lite_release_rx_buffer().
 */
int handle_echo_cap_messages(void){
	struct rpmsg_ep_info *_rpmsg_ep_info;
	sm_atomic_t tail_next;
	char* payload;
	uint32_t payload_len;
	uint32_t src;
	char append_msg[64];
	int32_t ret, i;

	//queue empty do nothing
	if(rpmsg_msg_queue_tail == rpmsg_msg_queue_head){
		return 0;
	}

	//Get a message from the queue
	tail_next = rpmsg_msg_queue_tail + 1;
	if(tail_next >= MAX_RPMSG_COUNT){
		tail_next = 0;
	}
	payload = (char*)rpmsg_msg_queue[tail_next].payload;
	payload_len = rpmsg_msg_queue[tail_next].payload_len;
	src = rpmsg_msg_queue[tail_next].src;
	_rpmsg_ep_info = rpmsg_msg_queue[tail_next].priv;

	// Process the message - capitalize letters
	for(i = 0; i < payload_len; i++){
		if('a' <= payload[i] && payload[i] <= 'z'){
			payload[i] = payload[i] - ('a' - 'A');
		}
	}

	// put in string ending for strcat_s
	if (payload_len < RL_BUFFER_PAYLOAD_SIZE)
		payload[payload_len] = '\0';

	// Attach sharc core info to the response
	snprintf(append_msg, sizeof(append_msg), " => capitalized echo from Core%d\n", adi_core_id());
	strcat_s(payload, RL_BUFFER_PAYLOAD_SIZE, append_msg);

	// Send modified message back to its origin endpoint
	ret = rpmsg_lite_send(
		_rpmsg_ep_info->rpmsg_instance,
		_rpmsg_ep_info->rpmsg_ept,
		src,
		payload,
		strlen(payload),
		100);
	if (ret < 0){
		//handle error
	}

	// Release the rpmsg buffer
	rpmsg_lite_release_rx_buffer(_rpmsg_ep_info->rpmsg_instance, rpmsg_msg_queue[tail_next].payload);
	rpmsg_msg_queue_tail = tail_next;
	return 1;
}

/*
 * Create seccond endpoint on the rpmsg channel and announce its existence.
 * Callback for this endpoint capitalizes letters in idle loop.
 * Core id is added to ECHO_EP_ADDRESS so the address is different for each core.
 */
int rpmsg_init_echo_cap_endpoint_to_ARM(int channel){
	struct rpmsg_lite_endpoint *rpmsg_ept;
	int ret;

	rpmsg_ept = rpmsg_lite_create_ept(
			&rpmsg_ARM_channel,
			ECHO_CAP_EP_ADDRESS + adi_core_id()*EP_CORE1_OFFSET+channel,
			&echo_cap_call_back,
			(void*)&rpmsg_echo_cap_ep_to_ARM[channel],
			&sharc_ARM_echo_cap_endpoint_context[channel]);
	if(rpmsg_ept == RL_NULL){
		return -1;
	}

	ret = rpmsg_ns_announce(
			&rpmsg_ARM_channel,
			rpmsg_ept,
			"sharc-echo-cap",
			RL_NS_CREATE);
	if(ret != RL_SUCCESS){
		return -1;
	}
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
	for (i=0;i<EP_NUM_PER_CHAN;i++)  {
		rpmsg_echo_cap_ep_to_ARM[i].rpmsg_instance = &rpmsg_ARM_channel;
		rpmsg_echo_cap_ep_to_ARM[i].rpmsg_ept = &sharc_ARM_echo_cap_endpoint_context[i].ept;
	}

	// Initialize rpmsg channel to the ARM core and create two endpoints
	rpmsg_init_channel_to_ARM();
	for (i=0;i<EP_NUM_PER_CHAN;i++) {
		rpmsg_init_echo_endpoint_to_ARM(i);
		rpmsg_init_echo_cap_endpoint_to_ARM(i);
	}

	/* Begin adding your custom code here */
	while(1) {
		handle_echo_cap_messages();
	}

	// Close notify system we are about to close endpoints.
	for (i=0;i<EP_NUM_PER_CHAN;i++) {
		rpmsg_ns_announce(
				rpmsg_echo_ep_to_ARM[i].rpmsg_instance,
				rpmsg_echo_ep_to_ARM[i].rpmsg_ept,
				"sharc-echo",
				RL_NS_DESTROY);

		rpmsg_ns_announce(
				rpmsg_echo_ep_to_ARM[i].rpmsg_instance,
				rpmsg_echo_cap_ep_to_ARM[i].rpmsg_ept,
				"sharc-echo-cap",
				RL_NS_DESTROY);
	}

	// Close endpoints and the rpmsg channel
	for (i=0;i<EP_NUM_PER_CHAN;i++) {
		rpmsg_lite_destroy_ept(rpmsg_echo_ep_to_ARM[i].rpmsg_instance, rpmsg_echo_ep_to_ARM[i].rpmsg_ept);
		//rpmsg_lite_destroy_ept(rpmsg_echo_cap_ep_to_ARM[i].rpmsg_instance, rpmsg_echo_cap_ep_to_ARM[i].rpmsg_ept);
		rpmsg_lite_deinit(&rpmsg_ARM_channel);
	}

	return 0;
}

