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

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* CCES includes */
#include <sys/adi_core.h>
#include <services/int/adi_sec.h>

/* Simple driver includes */
#include "twi_simple.h"
#include "sport_simple.h"


/* Project includes */
#include "context.h"
#include "init.h"
#include "clocks.h"
#include "rpmsg_initialize.h"
#include "rpmsg_arm_ep.h"
#include "icap.h"
#include "version.h"

/*This macro enables or disables the channel routing in DAC*/
//#define ROUTE_ENABLE

#define syslog_print(...)
#define syslog_printf(...)


typedef enum _CHANNEL_ROUTE {
    CHANNEL_ROUTE_DISABLE = 0,  // channel 1 to 12 observe in the 1-12 DAC output
	CHANNEL_ROUTE_ENABLE,  // channel 4 to 16 observe in the 1-12 DAC output
	CHANNEL_ROUTE_INVALID
} CHANNEL_ROUTE;


#ifdef ROUTE_ENABLE
unsigned route_setting = CHANNEL_ROUTE_ENABLE;
#else
unsigned route_setting = CHANNEL_ROUTE_DISABLE;
#endif


extern struct icap_instance icap_sharc_alsa_playback;
#ifdef ICAP_RECORD_EN
extern struct icap_instance icap_sharc_alsa_record;
#endif

/* Application context */
APP_CONTEXT mainAppContext;

/* prototype */
void Switch_Configurator(void);
void ConfigSoftSwitches_ADC_DAC(void);
void ConfigSoftSwitches_ADAU_Reset(void);

/* Idle function for performance measurement purposes */
#pragma never_inline
void idle_asm(void) {
   asm volatile("idle;");
}

/***********************************************************************
 * Main
 **********************************************************************/
int main(int argc, char **argv)
{

    APP_CONTEXT *context = &mainAppContext;
    TWI_SIMPLE_RESULT twiResult;
    SPORT_SIMPLE_RESULT sportResult;

    /* Initialize the SEC */
    adi_sec_Init();

    /* Initialize the application context */
    memset(context, 0, sizeof(*context));

    /* Switch Configuration */
	Switch_Configurator();

    /* Initialize GPIO */
    gpio_init();

    /* Init the system heaps */
    umm_heap_init();

    syslog_printf("twi_init\n");
    /* Initialize the simple TWI driver */
    twiResult = twi_init();

    /* Initialize the simple SPORT driver */
    syslog_printf("sport_init\n");
    sportResult = sport_init();


    /* Open up a global device handle for TWI2 @ 400KHz */
    syslog_printf("twi_setSpeed\n");
    twiResult = twi_open(TWI2, &context->twi2Handle);
    twi_setSpeed(context->twi2Handle, TWI_SIMPLE_SPEED_400);

    /* Set the adau1962 and soft switch device handles to TWI2 */
    context->adau1962TwiHandle = context->twi2Handle;

    syslog_printf("rpmsg_initialize\n");

    /* Disable main MCLK/BCLK */
    syslog_printf("disable mclk\n");
    disable_mclk(context);

    /* Initialize main MCLK/BCLK */
    mclk_init(context);

    /* Initialize the ADAU1962 DAC */
    adau1962_init(context);
    twiResult = twi_close(&context->twi2Handle);
    /* Enable main MCLK/BCLK for a synchronous start */
    enable_mclk(context);

    /* Initialize rpmsg and icap channels to linux */
    rpmsg_initialize();

    /* Initialize the audio routing table */
    syslog_printf("audio_routing_init\n");
    audio_routing_init(context);

    syslog_printf("Started\n");

    extern void apply_playback_settings(char **argv);

    const char* inputargs[] = { "00", "linux", "codec", "16", NULL};

    apply_playback_settings((char**)inputargs);

    /* Drop into the shell */
    while (1) {
        icap_loop(&icap_sharc_alsa_playback);
#ifdef ICAP_RECORD_EN
        icap_loop(&icap_sharc_alsa_record);
#endif
        idle_asm();
     }
}


void Switch_Configurator()
{
	int delay11=0xffff;

	/* Software Switch Configuration for Enabling ADC-DAC */
	ConfigSoftSwitches_ADC_DAC();

	while(delay11--)
	{
		asm("nop;");
	}

	/* Software Switch Configuration for Re-Setting ADC-DAC  */
	ConfigSoftSwitches_ADAU_Reset();

	/* wait for Codec to up */
	delay11=0xffff;
	while(delay11--)
	{
		asm("nop;");
	}
}
