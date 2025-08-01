/*
 * da7212.c
 *
 *  Created on: 2025. 7. 30.
 *      Author: qnfmd
 */
#include "hal_data.h"

#define DA7212_I2C_ADDR    (0x1A << 1)  // 7-bit address shifted for FSP I2C API

void da7212_write(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    R_IIC_MASTER_Write(&g_i2c_master0_ctrl, buf, 2, false);
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
}

void da7212_init(void)
{
    // Power supplies and reset
    da7212_write(0x92, 0x01); // Disable master bias (for full reset)
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
    da7212_write(0x92, 0x00); // Enable master bias

    // PLL & Clock
    da7212_write(0x23, 0x1A); // PLL enabled, input = MCLK, output = 98.304MHz
    da7212_write(0x24, 0x80); // PLL_REF_DIV = 2
    da7212_write(0x25, 0x18); // PLL integer part
    da7212_write(0x26, 0x00); // PLL fractional part
    da7212_write(0x28, 0x81); // Use PLL as SYSCLK source, SYSCLK enabled

    // Digital audio interface (I2S)
    da7212_write(0x2A, 0x00); // Slave mode
    da7212_write(0x2B, 0x18); // I2S mode, 16-bit word, normal polarity

    // Audio path: input → DAC → mixer → headphone
    da7212_write(0x2E, 0x01); // Enable DAC filter
    da7212_write(0x2F, 0xA0); // Mix input from DAC L+R
    da7212_write(0x30, 0x80); // Headphone Left input = mixer
    da7212_write(0x31, 0x80); // Headphone Right input = mixer

    // Gain
    da7212_write(0x32, 0x39); // HP L gain
    da7212_write(0x33, 0x39); // HP R gain

    // Output path power-up
    da7212_write(0x34, 0x03); // Enable headphone outputs
    da7212_write(0x2D, 0x01); // Enable DAC

    // Master bias ON
    da7212_write(0x92, 0x01);
}

