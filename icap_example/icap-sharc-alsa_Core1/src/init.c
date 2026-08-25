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

/* Standard library includes */
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#include <assert.h>

/* ADI service includes */
#include <services/gpio/adi_gpio.h>
#include <services/pwr/adi_pwr.h>

/* ADI processor includes */
#include <sruSC598.h>
#include <umm_malloc_heaps.h>

/* Simple driver includes */
#include "twi_simple.h"
#include "sport_simple.h"

/* Simple service includes */
#include "adau1962.h"

/* OSS service includes */
#include "umm_malloc.h"
#include "context.h"
#include "init.h"
#include "gpio_pins.h"
#include "codec_audio.h"
#include "route.h"

#define syslog_print(...)
#define syslog_printf(...)

/***********************************************************************
 * GPIO / Pin MUX / SRU Initialization
 **********************************************************************/

/* TWI2 GPIO FER bit positions */
#define TWI2_SCL_PORTA_FER (1 << BITP_PORT_DATA_PX14)
#define TWI2_SDA_PORTA_FER (1 << BITP_PORT_DATA_PX15)

/* TWI2 GPIO MUX bit positions (two bits per MUX entry */
#define TWI2_SCL_PORTA_MUX (0 << (BITP_PORT_DATA_PX14 << 1))
#define TWI2_SDA_PORTA_MUX (0 << (BITP_PORT_DATA_PX15 << 1))

/* DAI IE Bit definitions (not in any ADI header files) */
#define BITP_PADS0_DAI0_IE_PB03 (2)
#define BITP_PADS0_DAI0_IE_PB04 (3)
#define BITP_PADS0_DAI0_IE_PB05 (4)

/*
 * WARNING: Order must match the GPIO_PIN_ID enum in gpio_pins.h!
 */
GPIO_CONFIG gpioPins[GPIO_PIN_MAX] = {
	{ ADI_GPIO_PORT_D, ADI_GPIO_PIN_0, ADI_GPIO_DIRECTION_INPUT,
	  0 }, // GPIO_PIN_SOMCRR_PB1
	{ ADI_GPIO_PORT_H, ADI_GPIO_PIN_0, ADI_GPIO_DIRECTION_INPUT,
	  0 }, // GPIO_PIN_SOMCRR_PB2
	{ ADI_GPIO_PORT_C, ADI_GPIO_PIN_1, ADI_GPIO_DIRECTION_OUTPUT,
	  0 }, // GPIO_PIN_SOMCRR_LED7
	{ ADI_GPIO_PORT_C, ADI_GPIO_PIN_2, ADI_GPIO_DIRECTION_OUTPUT,
	  0 }, // GPIO_PIN_SOMCRR_LED10
	{ ADI_GPIO_PORT_C, ADI_GPIO_PIN_3, ADI_GPIO_DIRECTION_OUTPUT,
	  0 }, // GPIO_PIN_SOMCRR_LED9
	{ ADI_GPIO_PORT_C, ADI_GPIO_PIN_5, ADI_GPIO_DIRECTION_INPUT,
	  0 }, // GPIO_PIN_SOMCRR_A2B1_IRQ
	{ ADI_GPIO_PORT_I, ADI_GPIO_PIN_4, ADI_GPIO_DIRECTION_OUTPUT,
	  0 }, // GPIO_PIN_SOMCRR_PTPPPS0
#ifdef ENABLE_GPIO_DEBUG
	{ ADI_GPIO_PORT_A, ADI_GPIO_PIN_13, ADI_GPIO_DIRECTION_OUTPUT,
	  0 }, // GPIO_PIN_DEBUG_0
#endif
};

bool gpio_get_pin(GPIO_CONFIG *pinConfig, GPIO_PIN_ID pinId)
{
	ADI_GPIO_RESULT result;
	GPIO_CONFIG *pin;
	uint32_t value;

	pin = &pinConfig[pinId];

	result = adi_gpio_GetData(pin->port, &value);

	pin->state = value & pin->pinNum;

	return (pin->state);
}

void gpio_set_pin(GPIO_CONFIG *pinConfig, GPIO_PIN_ID pinId, bool state)
{
	ADI_GPIO_RESULT result;
	GPIO_CONFIG *pin;

	pin = &pinConfig[pinId];

	if (state) {
		result = adi_gpio_Set(pin->port, pin->pinNum);
	} else {
		result = adi_gpio_Clear(pin->port, pin->pinNum);
	}

	pin->state = state;
}

void gpio_toggle_pin(GPIO_CONFIG *pinConfig, GPIO_PIN_ID pinId)
{
	ADI_GPIO_RESULT result;
	GPIO_CONFIG *pin;

	pin = &pinConfig[pinId];

	result = adi_gpio_Toggle(pin->port, pin->pinNum);

	gpio_get_pin(pinConfig, pinId);
}

void gpio_init_pins(GPIO_CONFIG *pinConfig, int numPins)
{
	ADI_GPIO_RESULT result;
	GPIO_CONFIG *pin;
	int i;

	for (i = 0; i < GPIO_PIN_MAX; i++) {
		/* Select the pin */
		pin = &pinConfig[i];

		/* Set the initial pin state */
		if (pin->state) {
			result = adi_gpio_Set(pin->port, pin->pinNum);
		} else {
			result = adi_gpio_Clear(pin->port, pin->pinNum);
		}

		/* Set the direction */
		result =
			adi_gpio_SetDirection(pin->port, pin->pinNum, pin->dir);
	}
}

void gpio_init(void)
{
	static uint8_t gpioMemory[ADI_GPIO_CALLBACK_MEM_SIZE * 16];
	uint32_t numCallbacks;
	ADI_GPIO_RESULT result;

	/* Init the GPIO system service */
	result = adi_gpio_Init(gpioMemory, sizeof(gpioMemory), &numCallbacks);

	/* Configure TWI2 Alternate Function GPIO */
	*pREG_PORTA_FER |= (TWI2_SCL_PORTA_FER | TWI2_SDA_PORTA_FER);
	*pREG_PORTA_MUX |= (TWI2_SCL_PORTA_MUX | TWI2_SDA_PORTA_MUX);

	/* Configure straight GPIO */
	gpio_init_pins(gpioPins, GPIO_PIN_MAX);

	/* PADS0 DAI0/1 Port Input Enable Control Register */
	*pREG_PADS0_DAI0_IE = BITM_PADS_DAI0_IE_VALUE;
	*pREG_PADS0_DAI1_IE = BITM_PADS_DAI1_IE_VALUE;
}

/***********************************************************************
 * UMM_MALLOC heap initialization
 **********************************************************************/
#pragma section("seg_l2_noinit_data", NO_INIT)
static uint8_t umm_l2_sport_heap[UMM_L2_SPORT_HEAP_SIZE];

#pragma section("seg_sdram_noinit_data", NO_INIT)
static uint8_t umm_l2_cached_heap[UMM_L2_CACHED_HEAP_SIZE];

void umm_heap_init(void)
{
	/* Initialize the L2 SPORT buffer heap. */
	umm_init(UMM_L2_SPORT_HEAP, umm_l2_sport_heap, UMM_L2_SPORT_HEAP_SIZE);

	/* Initialize the L2 cached heap. */
	umm_init(UMM_L2_CACHED_HEAP, umm_l2_cached_heap,
		 UMM_L2_CACHED_HEAP_SIZE);
}

/***********************************************************************
 * This function allocates audio buffers in L2 cached memory and
 * initializes a single SPORT using the simple SPORT driver.
 *
 * NOTE: OUT buffers are generally marked as not cached since outbound
 *       buffers are flushed in process_audio().  This keeps the driver
 *       from prematurely (and redundantly) flushing the buffer when the
 *       SPORT callback returns.
 **********************************************************************/
static sSPORT *single_sport_init(SPORT_SIMPLE_PORT sport,
				 SPORT_SIMPLE_CONFIG *cfg,
				 SPORT_SIMPLE_AUDIO_CALLBACK cb,
				 void **pingPongPtrs, unsigned *pingPongLen,
				 void *usrPtr, bool cached,
				 SPORT_SIMPLE_RESULT *result)
{
	sSPORT *sportHandle;
	SPORT_SIMPLE_RESULT sportResult;
	uint32_t dataBufferSize;
	int i;

	/* Open a handle to the SPORT */
	sportResult = sport_open(sport, &sportHandle);
	if (sportResult != SPORT_SIMPLE_SUCCESS) {
		if (result) {
			*result = sportResult;
		}
		return (NULL);
	}

	/* Copy application callback info */
	cfg->callBack = cb;
	cfg->usrPtr = usrPtr;

	/* Allocate audio buffers if not already allocated */
	dataBufferSize = sport_buffer_size(cfg);
	for (i = 0; i < 2; i++) {
		if (!cfg->dataBuffers[i]) {
			cfg->dataBuffers[i] = umm_malloc_heap_aligned(
				UMM_L2_SPORT_HEAP, dataBufferSize,
				ADI_CACHE_LINE_LENGTH);
			if (cfg->dataBuffers[i] == NULL) {
				syslog_printf(
					"Failed to allocate SPORT buffer of size %#x\n",
					dataBufferSize);
				sportResult = SPORT_SIMPLE_ERROR;
				break;
			}
			memset(cfg->dataBuffers[i], 0, dataBufferSize);
		}
	}

	if (sportResult != SPORT_SIMPLE_SUCCESS) {
		if (result) {
			*result = sportResult;
		}
		return (NULL);
	}

	cfg->dataBuffersCached = cached;
	cfg->syncDMA = true;

	/* Configure the SPORT */
	sportResult = sport_configure(sportHandle, cfg);

	/* Save ping pong data pointers */
	if (pingPongPtrs) {
		pingPongPtrs[0] = cfg->dataBuffers[0];
		pingPongPtrs[1] = cfg->dataBuffers[1];
	}
	if (pingPongLen) {
		*pingPongLen = dataBufferSize;
	}
	if (result) {
		*result = sportResult;
	}

	return (sportHandle);
}

/***********************************************************************
 * Simple SPORT driver 8/16-ch packed I2S settings
 * Compatible A2B I2S Register settings:
 * 8 ch
 *    I2SGCFG: 0xE2
 *     I2SCFG: 0x7F
 * 16 ch
 *    I2SGCFG: 0xE4
 *     I2SCFG: 0x7F
 **********************************************************************/

SPORT_SIMPLE_CONFIG cfg16chPackedI2S = {
	.clkDir = SPORT_SIMPLE_CLK_DIR_SLAVE,
	.fsDir = SPORT_SIMPLE_FS_DIR_SLAVE,
	.bitClkOptions = SPORT_SIMPLE_CLK_FALLING,
	.fsOptions = SPORT_SIMPLE_FS_OPTION_INV | SPORT_SIMPLE_FS_OPTION_EARLY |
		     SPORT_SIMPLE_FS_OPTION_50,
	.tdmSlots = SPORT_SIMPLE_TDM_16,
	.wordSize = SPORT_SIMPLE_WORD_SIZE_32BIT,
	.dataEnable = SPORT_SIMPLE_ENABLE_BOTH,
	.frames = SYSTEM_BLOCK_SIZE,
	.syncDMA = true
};

void disable_mclk(APP_CONTEXT *context)
{
	*pREG_PADS0_DAI1_IE &= ~((1 << BITP_PADS0_DAI0_IE_PB05));
}

void enable_mclk(APP_CONTEXT *context)
{
	*pREG_PADS0_DAI1_IE |= ((1 << BITP_PADS0_DAI0_IE_PB05));
}

static void sru_config_mclk(APP_CONTEXT *context)
{
	/* 24.576Mhz MCLK in from DAC 512Fs BCLK on DAI1.05 */
	SRU2(LOW, DAI1_PBEN05_I);

	/* Cross-route DAI1.05 MCLK to DAI0.11 as an output */
	SRU(HIGH, DAI0_PBEN11_I);
	SRU(DAI1_PB05_O, DAI0_PB11_I);
}

void mclk_init(APP_CONTEXT *context)
{
	sru_config_mclk(context);
}

/***********************************************************************
 * ADAU1962 DAC / SPORT4 / SRU initialization (TDM16 clock slave)
 **********************************************************************/
#define ADAU1962_I2C_ADDR (0x04)

static void sru_config_adau1962_master(void)
{
	/* Setup DAI Pin I/O */
	SRU2(HIGH, DAI1_PBEN01_I); // ADAU1962 DAC data1 is an output
	SRU2(HIGH, DAI1_PBEN02_I); // ADAU1962 DAC data2 is an output
	SRU2(LOW, DAI1_PBEN04_I); // ADAU1962 FS is an input
	//SRU2(LOW, DAI1_PBEN05_I);       // ADAU1962 CLK is an input

	/* Route clocks */
	SRU2(DAI1_PB04_O, SPT4_AFS_I); // route FS
	SRU2(DAI1_PB05_O, SPT4_ACLK_I); // route BCLK

	/* Route to SPORT4A */
	SRU2(SPT4_AD0_O,
	     DAI1_PB01_I); // SPORT4A-D0 output to ADAU1962 data1 pin
}

static void adau1962_sport_init(APP_CONTEXT *context)
{
	SPORT_SIMPLE_CONFIG sportCfg;
	SPORT_SIMPLE_RESULT sportResult;

	/* SPORT4A: DAC 16-ch packed I2S data out */
	sportCfg = cfg16chPackedI2S;
	sportCfg.dataDir = SPORT_SIMPLE_DATA_DIR_TX;
	sportCfg.dataEnable = SPORT_SIMPLE_ENABLE_PRIMARY;
	context->dacSportOutHandle = single_sport_init(SPORT4A, &sportCfg,
						       dacAudioOut, NULL, NULL,
						       context, false, NULL);

	if (context->dacSportOutHandle) {
		sportResult = sport_start(context->dacSportOutHandle, true);
		assert(sportResult == SPORT_SIMPLE_SUCCESS);
	}
}

void adau1962_init(APP_CONTEXT *context)
{
	/* Configure the DAI routing */
	sru_config_adau1962_master();

	/* Configure the SPORT */
	adau1962_sport_init(context);

	/* Initialize the DAC */
	init_adau1962(context->adau1962TwiHandle, ADAU1962_I2C_ADDR);
}

/***********************************************************************
 * Audio routing
 **********************************************************************/
void audio_routing_init(APP_CONTEXT *context)
{
	context->routingTable =
		umm_calloc(MAX_AUDIO_ROUTES, sizeof(ROUTE_INFO));
}
