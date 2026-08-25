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

#ifndef __UMM_MALLOC_HEAPS_H__
#define __UMM_MALLOC_HEAPS_H__

/*
 * When setting the heap size, be aware the UMM_MALLOC has a limitation
 * of 524287 blocks of UMM_BLOCK_SIZE bytes each.
 */

#define UMM_L2_CACHED_HEAP_SIZE      (       2*1024 * 1024 )
#define UMM_L2_SPORT_HEAP_SIZE       (        48 * 1024 )

#define UMM_HEAP_NAMES         \
{                              \
    "UMM_L2_CACHED_HEAP",      \
    "UMM_L2_SPORT_HEAP",    \
}                              \

typedef enum {
    UMM_L2_CACHED_HEAP = 0,
    UMM_L2_SPORT_HEAP,
    UMM_NUM_HEAPS
} umm_heap_t;

/*
 * The macro which sets the default heap used for standard
 * malloc/free calls
 */
#define UMM_DEFAULT_HEAP  UMM_L2_CACHED_HEAP

#endif

