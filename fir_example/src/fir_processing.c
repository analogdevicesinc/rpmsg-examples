/*********************************************************************************
Copyright(c) 2021-2022 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.
 *********************************************************************************/

/*****************************************************************************
 * FIR_Processing.c
 *****************************************************************************/

#include <sys/platform.h>
#include <sys/adi_core.h>
#include "adi_initialize.h"
#include "fir_processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>




//float FirInputBuff3[TAPS2+FIR_WINDOW_SIZE-1]=
//{
//	#include "fir_input_taps_4096_window_1024.dat"
//};

extern float *FirInputBuff3;
float FirOutputBuff3[FIR_WINDOW_SIZE];

float CoeffBuff3[TAPS2]=
{
	#include "fir_coeffs_4096.dat"
};

#pragma align 32
uint8_t FirTask1Memory[FIR_MEM_SIZE_TASK1];

#pragma align 32
uint8_t FirTask2Memory[FIR_MEM_SIZE_TASK2];

ADI_FIR_DEV_HANDLE hFir;

ADI_FIR_TASK_HANDLE hFirTasks[FIR_NUMBER_OF_TASKS] = {0};

volatile uint32_t FIRTaskDoneCount = 0;
volatile void *FIRCompletedTaskHandles[20] = {0};
void FIRTaskDoneCallback( void * pCBParam, ADI_FIR_EVENT Event, void * pArg);

void FIRTaskDoneCallback( void * pCBParam, ADI_FIR_EVENT Event, void * pArg)
{
	switch(Event)
	{
#if  ADI_FIR_CFG_ACCELERATOR_MODE == ADI_FIR_ACCELERATOR_MODE_ACM
	    case ADI_FIR_EVENT_CHANNEL_DONE:
#else
	    case ADI_FIR_EVENT_ALL_CHANNEL_DONE:
#endif
	    	FIRCompletedTaskHandles[FIRTaskDoneCount++] = pArg;
	}
}

volatile int count = 0;

/* This function creates 2 types of demo tasks, which can then be executed */
ADI_FIR_RESULT execute_single_channel(float *data_buf)
{
	ADI_FIR_RESULT eFirResult = ADI_FIR_RESULT_SUCCESS;

	ADI_FIR_CHANNEL_INFO FirTask2_Channels[FIR_NUMBER_OF_CHANNELS_TASK2] = {

			{
				TAPS2,                                                      /* Tap Length */
				FIR_WINDOW_SIZE,                                            /* Window Size */
				ADI_FIR_SAMPLING_SINGLE_RATE,                               /* Sampling */
				1,                                                          /* Sampling Ratio */
#if  ADI_FIR_CFG_ACCELERATOR_MODE == ADI_FIR_ACCELERATOR_MODE_ACM
				true,                                                       /* Callback Enable */
				false,                                                      /* Generate Trigger on completion */
				false,                                                      /* Wait for Trigger */
				ADI_FIR_FLOAT_ROUNDING_MODE_IEEE_ROUND_TO_NEAREST_EVEN,     /* Rounding Mode */
				false,                                                      /* Fixed point enable */
				ADI_FIR_FIXED_INPUT_FORMAT_UNSIGNED_INTEGER,                /* Fixed Point format */
#endif /* MODE == ACM */
				TAPS2,                                                      /* Coefficient Count */
				1,                                                          /* Coefficient Modify */
				(void *)CoeffBuff3,                                         /* Coefficient Index */
		        (void *)FirOutputBuff3,                                     /* Output Base */
				FIR_WINDOW_SIZE,                                            /* Output Count */
				1,                                                          /* Output Modify */
				(void *)FirOutputBuff3,                                     /* Output Index */
		        (void *)FirInputBuff3,                                      /* Input Base */
				TAPS2+FIR_WINDOW_SIZE-1,                                    /* Input Count */
				1,                                                          /* Input Modify */
				(void *)FirInputBuff3                                       /* Input Count */

			},

	};

	/* Open the FIR Device */
	eFirResult  = adi_fir_Open(0u,&hFir);
	CHECK_FIR_RESULT(eFirResult)
	
	/* Register the Callback */
	eFirResult = adi_fir_RegisterCallback(hFir,FIRTaskDoneCallback,0);
	CHECK_FIR_RESULT(eFirResult)

    eFirResult = adi_fir_CreateTask(hFir, /* Device Handle*/
    		FirTask2_Channels,            /* Pointer to Channel List */
			FIR_NUMBER_OF_CHANNELS_TASK2, /* Number of Channels  */
			&FirTask2Memory,              /* Pointer to Memory */
			FIR_MEM_SIZE_TASK2,           /*Memory Size */
			&hFirTasks[0]);               /* Address to store the handle */
    CHECK_FIR_RESULT(eFirResult)
	
	/* Queue the Tasks Created */
	for(count = 0; count<FIR_NUMBER_OF_TASKS; count++)
	{
	    eFirResult = adi_fir_QueueTask(hFirTasks[count]);
	    CHECK_FIR_RESULT(eFirResult)
	}

    /* 
	   In Legacy Mode,the callbacks are received after completion of all the channels of the task. Hence wait untill we get 
	   as many callbacks as the number of tasks queued(i.e 2).
	   In Auto Configuration Mode(ACM),the callbacks are customisable with each channel of a task.
       In this example, since all the channels of both tasks are configured to generate callback after their completion, 
	   wait untill we get as many callbacks as the total number of channels in both the tasks.(i.e 4) 
	*/
#if (ADI_FIR_CFG_ACCELERATOR_MODE == ADI_FIR_ACCELERATOR_MODE_LEGACY)
	while(FIRTaskDoneCount<FIR_NUMBER_OF_TASKS);
#elif (ADI_FIR_CFG_ACCELERATOR_MODE == ADI_FIR_ACCELERATOR_MODE_ACM)
	while(FIRTaskDoneCount<FIR_NUMBER_OF_CHANNELS_TASK);
#endif

	for(int i=0; i<5; i++) {
		printf("in[%d]: %9.6f\n", i, FirInputBuff3[i]);
		printf("out[%d]: %9.6f\n", i, FirOutputBuff3[i]);
	}

	for(int i=0; i<FIR_WINDOW_SIZE; i++)
		data_buf[i] = FirOutputBuff3[i];

	return eFirResult;
}

