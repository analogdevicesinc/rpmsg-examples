/*********************************************************************************
Copyright(c) 2019 Analog Devices, Inc. All Rights Reserved.
This software is proprietary. By using this software you agree
to the terms of the associated Analog Devices License Agreement.
*********************************************************************************/

/*****************************************************************************
 * FIR_Multi_Channel_Processing.h
 *****************************************************************************/

#ifndef __FIR_MULTI_CHANNEL_PROCESSING_H__
#define __FIR_MULTI_CHANNEL_PROCESSING_H__

#include <drivers/fir/adi_fir.h>

/* Number of Taps 1. Note that Tap length can be varied across channels in a task.*/
#define TAPS1    64

/* Number of Taps 2. Note that Tap length can be varied across channels in a task.*/
#define TAPS2    4096

/* Window Size. Note that Window size can be varied across channels in a task */
#define FIR_WINDOW_SIZE    1024

/* Number of FIR Tasks */
#define FIR_NUMBER_OF_TASKS 1

/* Number of Channels in FIR task 1 */
#define FIR_NUMBER_OF_CHANNELS_TASK1 2

/* Number of Channels in FIR task 2 */
#define FIR_NUMBER_OF_CHANNELS_TASK2 1

/* Total Number of Channels */
#define FIR_NUMBER_OF_CHANNELS    (FIR_NUMBER_OF_CHANNELS_TASK1 + FIR_NUMBER_OF_CHANNELS_TASK2)

/* FIR Task 1 Memory Size */
#define FIR_MEM_SIZE_TASK1    (FIR_MEM_SIZE(FIR_NUMBER_OF_CHANNELS_TASK1))

/* FIR Task 2 Memory Size */
#define FIR_MEM_SIZE_TASK2    (FIR_MEM_SIZE(FIR_NUMBER_OF_CHANNELS_TASK2))




/* Macro for checking Result returned by the driver after API call */
#define CHECK_FIR_RESULT(eResult) \
        if(eResult != ADI_FIR_RESULT_SUCCESS)\
		{\
            printf("CHECK_RESULT failed at line %d of file %s \n",__LINE__,__FILE__);\
            exit(-2);\
        }

#define MAX_DIFF_LIMIT 0.001

/* Macro for checking Maximum Diff exceeding the limit */
#define CHECK_MAX_DIFF(max_diff) \
        if(max_diff > MAX_DIFF_LIMIT)\
		{\
            printf("CHECK_MAX_DIFF failed at line %d of file %s \n",__LINE__,__FILE__);\
            exit(-2);\
        }

ADI_FIR_RESULT execute_single_channel(float *data_buf);


#endif /* __FIR_MULTI_CHANNEL_PROCESSING_H__ */
