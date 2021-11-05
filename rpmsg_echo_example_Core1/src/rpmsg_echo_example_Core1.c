/*****************************************************************************
 * rpmsg_echo_example_Core1.c
 *****************************************************************************/
#include <string.h>

#include <sys/platform.h>
#include <sys/adi_core.h>

#include <rpmsg_platform.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>

#include "adi_initialize.h"
#include "rpmsg_echo_example_Core1.h"

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

struct sharc_resource_table{
	struct resource_table table_hdr;
	unsigned int offset[1];
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring vring[2];
};

extern "asm" struct sharc_resource_table ___MCAPI_arm_start[2];

/* Static variables for rpmsg */
#define ECHO_EP_ADDRESS 150
struct rpmsg_lite_instance rpmsg_ARM_channel;
struct rpmsg_lite_ept_static_context sharc_ARM_echo_endpoint_context;

struct rpmsg_ep_info{
	struct rpmsg_lite_instance *rpmsg_instance;
	struct rpmsg_lite_endpoint *rpmsg_ept;
};

const struct rpmsg_ep_info rpmsg_echo_ep_to_ARM = {
		.rpmsg_instance = &rpmsg_ARM_channel,
		.rpmsg_ept = &sharc_ARM_echo_endpoint_context.ept,
};

struct vring_mem_info{
	uint64_t desc_start;
	uint64_t desc_end;
	uint64_t buffer_start;
	uint64_t buffer_end;
};

void vring_get_mem_alloc_info(struct fw_rsc_vdev_vring *vring, struct vring_mem_info *info){
	struct vring_desc *desc = (struct vring_desc *)vring->da;
	info->desc_start = (uint64_t)desc;
	info->desc_end = (uint64_t)desc + vring_size(vring->num, vring->align);
	info->buffer_start = desc->addr;
	info->buffer_end = desc->addr + vring->num * (RL_BUFFER_PAYLOAD_SIZE +16);
}

int rpmsg_init_channel_to_ARM(void){
	struct sharc_resource_table *resource_table;
	struct rpmsg_lite_instance *rpmsg_instance;
	adiCacheStatus status;
	uint32_t addr_start, addr_end;
	struct vring_mem_info vring0_info;
	struct vring_mem_info vring1_info;

	// The delay is required to read addresses correctly
	platform_time_delay(15);

    switch(adi_core_id()){
	case ADI_CORE_ARM:
		return -1;
		//break;
    case ADI_CORE_SHARC0:
		resource_table = &___MCAPI_arm_start[0];
		break;
    case ADI_CORE_SHARC1:
		resource_table = &___MCAPI_arm_start[1];
		break;
    default:
		// should never happen
		break;
    }

    // Get memory range which needs cache disabled
    // Read vring memory info
	vring_get_mem_alloc_info(&resource_table->vring[0], &vring0_info);
	vring_get_mem_alloc_info(&resource_table->vring[1], &vring1_info);

#define _min_u64_to_u32(a, b) ((uint32_t)((a)<(b)?(a):(b)))
#define _max_u64_to_u32(a, b) ((uint32_t)((a)>(b)?(a):(b)))
	// vring1 has its own descriptors but share buffers with vring0
	// vring1.buffer_start and vring1_info.buffer_end has invalid addresses, ignore them
    addr_start = _min_u64_to_u32(vring0_info.desc_start, vring1_info.desc_start);
    addr_start = _min_u64_to_u32(addr_start, vring0_info.buffer_start);
    addr_end = _max_u64_to_u32(vring0_info.desc_end, vring1_info.desc_end);
    addr_end = _max_u64_to_u32(addr_end, vring0_info.buffer_end);

    // Disable cache for the vring memory range
	status = adi_cache_set_range ((void *)addr_start,
                         (void *)(addr_end),
						 adi_cache_rr7,
                         adi_cache_noncacheable_range);

	rpmsg_instance = rpmsg_lite_remote_init(
			&resource_table->rpmsg_vdev,
			RL_PLATFORM_SHARC_ARM_LINK_ID,
			RL_SHM_VDEV,
			&rpmsg_ARM_channel);
	if(rpmsg_instance == RL_NULL){
		return -1;
	}

	while(!rpmsg_lite_is_link_up(rpmsg_instance));
	return 0;
}

int32_t echo_call_back(void *payload, uint32_t payload_len, uint32_t src, void *priv){
	struct rpmsg_ep_info *_rpmsg_ep_info = (struct rpmsg_ep_info *)priv;
	char* data=payload;
	char append_msg[32];
	int32_t ret;

	// put in string ending for strcat_s
	if (payload_len < RL_BUFFER_PAYLOAD_SIZE)
		data[payload_len] = '\0';

	snprintf(append_msg, sizeof(append_msg), " => echo from Core%d\n", adi_core_id());
	strcat_s(payload, RL_BUFFER_PAYLOAD_SIZE, append_msg);

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

int rpmsg_init_echo_endpoint_to_ARM(void){
	struct rpmsg_lite_endpoint *rpmsg_ept;
	int ret;

	rpmsg_ept = rpmsg_lite_create_ept(
			&rpmsg_ARM_channel,
			ECHO_EP_ADDRESS + adi_core_id(),
			&echo_call_back,
			(void*)&rpmsg_echo_ep_to_ARM,
			&sharc_ARM_echo_endpoint_context);
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

int main(int argc, char *argv[])
{
	int run=1;

	adi_initComponents();
	rpmsg_init_channel_to_ARM();
	rpmsg_init_echo_endpoint_to_ARM();

	while(run){
	}


	rpmsg_ns_announce(
			rpmsg_echo_ep_to_ARM.rpmsg_instance,
			rpmsg_echo_ep_to_ARM.rpmsg_ept,
			"sharc-echo",
			RL_NS_DESTROY);
	rpmsg_lite_destroy_ept(rpmsg_echo_ep_to_ARM.rpmsg_instance, rpmsg_echo_ep_to_ARM.rpmsg_ept);
	rpmsg_lite_deinit(&rpmsg_ARM_channel);
	return 0;
}

