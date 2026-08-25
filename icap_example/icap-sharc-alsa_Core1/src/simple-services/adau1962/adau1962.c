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

#include <stdint.h>

#include "adau1962.h"
#include "twi_simple.h"

#define ADAU1962_PLL_CTL_CTRL0          0x00
#define ADAU1962_PLL_CTL_CTRL1          0x01
#define ADAU1962_PDN_CTRL_1             0x02
#define ADAU1962_PDN_CTRL_2             0x03
#define ADAU1962_PDN_CTRL_3             0x04
#define ADAU1962_TEMP_STAT              0x05
#define ADAU1962_DAC_CTRL0              0x06
#define ADAU1962_DAC_CTRL1              0x07
#define ADAU1962_DAC_CTRL2              0x08
#define ADAU1962_DAC_MUTE1              0x09
#define ADAU1962_DAC_MUTE2              0x0a
#define ADAU1962_MSTR_VOL               0x0b
#define ADAU1962_DAC1_VOL               0x0c
#define ADAU1962_DAC2_VOL               0x0d
#define ADAU1962_DAC3_VOL               0x0e
#define ADAU1962_DAC4_VOL               0x0f
#define ADAU1962_DAC5_VOL               0x10
#define ADAU1962_DAC6_VOL               0x11
#define ADAU1962_DAC7_VOL               0x12
#define ADAU1962_DAC8_VOL               0x13
#define ADAU1962_DAC9_VOL               0x14
#define ADAU1962_DAC10_VOL              0x15
#define ADAU1962_DAC11_VOL              0x16
#define ADAU1962_DAC12_VOL              0x17
#define ADAU1962_PAD_STRGTH             0x1C
#define ADAU1962_DAC_PWR1               0x1D
#define ADAU1962_DAC_PWR2               0x1E
#define ADAU1962_DAC_PWR3               0x1F

typedef struct {
    uint8_t reg;
    uint8_t value;
} REG_CONFIG;

REG_CONFIG adau1962Config[] = {
    { ADAU1962_PAD_STRGTH, 0x22 }, // 8mA pad drive strength
//    { ADAU1962_DAC_CTRL0,  0x19 }, // 48KHz, TDM8, 1-Bit Delay, Muted
    { ADAU1962_DAC_CTRL0,  0x21 }, // 48KHz, TDM16, 1-Bit Delay, Muted
//    { ADAU1962_DAC_CTRL1,  0x00 }, // I2S framing, Clock Slave
    { ADAU1962_DAC_CTRL1,  0x01 }, // I2S framing, Clock Master
    { ADAU1962_DAC_CTRL2,  0x00 }, // 256x OSR, Automute Disable
//    { ADAU1962_DAC_CTRL0,  0x18 }, // 48KHz, TDM8, 1-Bit delay, Unmuted
    { ADAU1962_DAC_CTRL0,  0x20 }, // 48KHz, TDM16, 1-Bit delay, Unmuted
};

static ADAU1962_RESULT init_adau1962_pll(sTWI *twi, uint8_t adau_address)
{
    ADAU1962_RESULT result;
    TWI_SIMPLE_RESULT twiResult;
    uint8_t rx[2];
    uint8_t tx[2];

    result = ADAU1962_SUCCESS;

    /* Set PLL PUP (power up) bit */
    tx[0] = ADAU1962_PLL_CTL_CTRL0; tx[1] = 0x01;
    twiResult = twi_write(twi, adau_address, tx, 2);
    if (twiResult != TWI_SIMPLE_SUCCESS) {
        return(ADAU1962_ERROR);
    }

    /* Set PLL PUP (power up) + MCS=2 (512fs) bits */
    tx[0] = ADAU1962_PLL_CTL_CTRL0; tx[1] = 0x05;
    twiResult = twi_write(twi, adau_address, tx, 2);
    if (twiResult != TWI_SIMPLE_SUCCESS) {
        return(ADAU1962_ERROR);
    }

    /* Set PLL VREF_EN + CCLKO_SEL=2 bits */
    tx[0] = ADAU1962_PLL_CTL_CTRL1; tx[1] = 0x22;
    twiResult = twi_write(twi, adau_address, tx, 2);
    if (twiResult != TWI_SIMPLE_SUCCESS) {
        return(ADAU1962_ERROR);
    }

    /* Poll for PLL_LOCK bit */
    tx[0] = ADAU1962_PLL_CTL_CTRL1;
    do {
        twiResult = twi_writeRead(twi, adau_address, tx, 1, rx, 1);
        if (twiResult != TWI_SIMPLE_SUCCESS) {
            return(ADAU1962_ERROR);
        }
    } while ((rx[0] & 0x04) == 0);

    return(result);
}


ADAU1962_RESULT init_adau1962(sTWI *twi, uint8_t adau_address)
{
    ADAU1962_RESULT result;
    TWI_SIMPLE_RESULT twiResult;
    uint8_t configLen;
    uint8_t i;
    uint8_t buf[2];

    /* Initialize the ADAU1962 PLL */
    result = init_adau1962_pll(twi, adau_address);
    if (result != ADAU1962_SUCCESS) {
        return(result);
    }

    /* Program ADAU1962 registers */
    configLen = sizeof(adau1962Config) / sizeof(adau1962Config[0]);
    for (i = 0; i < configLen; i++) {
        buf[0] = adau1962Config[i].reg; buf[1] = adau1962Config[i].value;
        twiResult = twi_write(twi, adau_address, buf, sizeof(buf));
        if (twiResult != TWI_SIMPLE_SUCCESS) {
            return(ADAU1962_ERROR);
        }
    }

    return(result);
}
