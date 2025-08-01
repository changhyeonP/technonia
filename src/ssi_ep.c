/***********************************************************************************************************************
 * File Name    : ssi_ep.c
 * Description  : Contains data structures and functions used in ssi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "ssi_ep.h"
#include "da7212.h"

/* Private functions */
static void ssi_example_calculate_samples(uint32_t buffer_index);
static void ssi_example_write();
static void deinit_gpt(void);
static void deinit_ssi(void);

/*******************************************************************************************************************//**
 * @addtogroup SSI_EP
 * @{
 **********************************************************************************************************************/

/* Global variables */
volatile i2s_event_t g_i2s_event       = I2S_EVENT_TX_EMPTY;  /* An actual event updates in callback */
volatile bool g_send_data_in_main_loop = true;
volatile bool g_data_ready             = false;
volatile uint8_t g_buffer_index        = 0;

/* Destination buffer to receive the sample audio data */
uint8_t g_dest_buff[BUFF_SIZE] = {RESET_VALUE};

/* Source buffer to transmit the sample audio data */
int32_t g_src_buff[2][SSI_STREAMING_EXAMPLE_SAMPLES_PER_CHUNK];

void da7212_write(uint8_t reg, uint8_t data);
void da7212_init(void);










/***********************************************************************************************************************
 * The RA Configuration tool generates main() and uses it to generate threads if an RTOS is used. This function is
 * called by main() when no RTOS is used.
 **********************************************************************************************************************/
uint8_t data_read_from_reg[8] = {0,};  // 읽은 값을 저장할 변수 (초기값은 0으로)
void ssi_entry(void)
{

    fsp_err_t err = FSP_SUCCESS;
        uint8_t reg[2];
        uint8_t read_value = 0;

        APP_PRINT("\r\n--- Audio Speaker Example Start ---\r\n");

        /* ========================================================================= */
        /* 1. I2C 드라이버 초기화                                                    */
        /* ========================================================================= */
        R_IIC_MASTER_CallbackSet(&g_i2c_master0_ctrl, i2c_callback, NULL, NULL);
        err = R_IIC_MASTER_Open(&g_i2c_master0_ctrl, &g_i2c_master0_cfg);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\nI2C Open Failed.\r\n");
            APP_ERR_TRAP(err);
        }
        APP_PRINT("I2C Open Success.\n");

        /* ========================================================================= */
        /* 2. DA7212 오디오 코덱 초기화 (I2C를 통해 레지스터 설정)                 */
        /* ========================================================================= */
        APP_PRINT("DA7212 Codec Configuration Start...\n");


        // 1. 시스템 활성화
        uint8_t target_reg_addr = 0xFD;  // 테스트할 레지스터 주소
        uint8_t value_to_write = 0x01;   // 쓰려는 값
//        uint8_t data_read_from_reg[8] = {0,};  // 읽은 값을 저장할 변수 (초기값은 0으로)

        uint8_t write_buffer[8];
        write_buffer[0] = target_reg_addr;
        write_buffer[1] = value_to_write;

        APP_PRINT("Testing Register 0x%X...\n", target_reg_addr);

        // Step 1: 먼저 레지스터에 값을 쓴다.
        err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, write_buffer, 2, false);
        if (FSP_SUCCESS != err)
        {
            APP_PRINT(" -> Write Failed.\n");
        }
        else
        {
            R_BSP_SoftwareDelay(40, BSP_DELAY_UNITS_MILLISECONDS); // 쓰기 후 안정화 대기

            // Step 2: 이제 해당 레지스터의 값을 읽어온다.
            // Step 2a: 읽을 레지스터의 주소를 먼저 알려준다.
//            err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &target_reg_addr, 1, true);

            err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, data_read_from_reg, 8, false);

//            for(uint8_t i=0; i<8; i++){
//                APP_PRINT(" -> Write/Read1 SUCCESS! Value: 0x%X\n", data_read_from_reg[i]);
//            }

//            // Step 2b: 주소를 보낸 직후, 데이터를 읽어온다.
//            if (FSP_SUCCESS == err)
//            {
//                err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, data_read_from_reg, 8, false);
//            }
//
//            // Step 3: 결과를 확인한다.
//            if (FSP_SUCCESS == err)
//            {
//
//                for(uint8_t i=0; i<8; i++){
//                    APP_PRINT(" -> Write/Read1 SUCCESS! Value: 0x%X\n", data_read_from_reg[i]);
//                }
//
//            }
//            else
//            {
//                APP_PRINT(" -> Read1 Transaction Failed.\n");
//            }
        }




//        reg[0] = 0xFD; reg[1] = 0x01; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false); R_BSP_SoftwareDelay(40, BSP_DELAY_UNITS_MILLISECONDS);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                        // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                        // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                        // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                        err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                        // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                        if (FSP_SUCCESS == err)
//                                        {
//                                            err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                        }
//
//                                        // Step 3: 쓴 값과 읽은 값 비교하기
//                                        if (FSP_SUCCESS == err)
//                                        {
//                                            if (reg[1] == read_value)
//                                            {
//                                                APP_PRINT("\r\n num1 Write/Read Success! Value: 0x%X\n", read_value);
//                                            }
//                                            else
//                                            {
//                                                APP_PRINT("\r\n num1 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                            }
//                                        }
//                                        else
//                                        {
//                                            APP_PRINT("\r\n num1 Read transaction failed.\r\n");
//                                        }

        // 2. 마스터 바이어스 활성화
//        reg[0] = 0x23; reg[1] = 0x08; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false); R_BSP_SoftwareDelay(25, BSP_DELAY_UNITS_MILLISECONDS);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                        // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                        // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                        // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                        err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                        // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                        if (FSP_SUCCESS == err)
//                                        {
//                                            err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                        }
//
//                                        // Step 3: 쓴 값과 읽은 값 비교하기
//                                        if (FSP_SUCCESS == err)
//                                        {
//                                            if (reg[1] == read_value)
//                                            {
//                                                APP_PRINT("\r\n num2 Write/Read Success! Value: 0x%X\n", read_value);
//                                            }
//                                            else
//                                            {
//                                                APP_PRINT("\r\n num2 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                            }
//                                        }
//                                        else
//                                        {
//                                            APP_PRINT("\r\n num2 Read transaction failed.\r\n");
//                                        }
//
//
//        // 3. 샘플레이트 설정 (48kHz)
//        reg[0] = 0x22; reg[1] = 0x0B; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                        // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                        // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                        // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                        err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                        // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                        if (FSP_SUCCESS == err)
//                                        {
//                                            err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                        }
//
//                                        // Step 3: 쓴 값과 읽은 값 비교하기
//                                        if (FSP_SUCCESS == err)
//                                        {
//                                            if (reg[1] == read_value)
//                                            {
//                                                APP_PRINT("\r\n num3 Write/Read Success! Value: 0x%X\n", read_value);
//                                            }
//                                            else
//                                            {
//                                                APP_PRINT("\r\n num3 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                            }
//                                        }
//                                        else
//                                        {
//                                            APP_PRINT("\r\n num3 Read transaction failed.\r\n");
//                                        }
//
//
//        // 4. PLL 설정
//        reg[0] = 0x27; reg[1] = 0x84; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num4-1 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num4-1 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num4-1 Read transaction failed.\r\n");
//                                }
//
//        reg[0] = 0x24; reg[1] = 0x07; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num4-2 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num4-2 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num4-2 Read transaction failed.\r\n");
//                                }
//
//        reg[0] = 0x25; reg[1] = 0xEA; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num4-3 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num4-3 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num4-3 Read transaction failed.\r\n");
//                                }
//
//        reg[0] = 0x26; reg[1] = 0x1E; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num4-4 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num4-4 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num4-4 Read transaction failed.\r\n");
//                                }
//
//
//        // 5. DAI 클록 모드 설정
//        reg[0] = 0x28; reg[1] = 0x01; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num5 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num5 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num5 Read transaction failed.\r\n");
//                                }
//
//
//        // 6. DAI 제어 설정
//        reg[0] = 0x29; reg[1] = 0xC4; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num6 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num6 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num6 Read transaction failed.\r\n");
//                                }
//
//
//        // 7. DAC 라우팅 설정
//        reg[0] = 0x2A; reg[1] = 0x22; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num7 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num7 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num7 Read transaction failed.\r\n");
//                                }
//
//
//        // 8. 차지펌프 설정
//        reg[0] = 0x47; reg[1] = 0xA1; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num8 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num8 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num8 Read transaction failed.\r\n");
//                                }
//
//
//        // 9. 스피커 출력 믹서 설정
//        reg[0] = 0x4B; reg[1] = 0x08; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num9-1 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num9-1 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num9-1 Read transaction failed.\r\n");
//                                }
//
//        reg[0] = 0x4C; reg[1] = 0x08; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num9-2 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num9-2 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num9-2 Read transaction failed.\r\n");
//                                }
//
//
//        // 10. 라인 앰프 (스피커) 게인 설정 -- 여기가 볼륨 조절 부분입니다.
//        reg[0] = 0x4A; reg[1] = 0x30; // 현재 +6dB, 최댓값은 0x3F(+21dB)
//        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//        // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//        // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                        // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                        err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                        // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                        if (FSP_SUCCESS == err)
//                        {
//                            err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                        }
//
//                        // Step 3: 쓴 값과 읽은 값 비교하기
//                        if (FSP_SUCCESS == err)
//                        {
//                            if (reg[1] == read_value)
//                            {
//                                APP_PRINT("\r\n num10 Write/Read Success! Value: 0x%X\n", read_value);
//                            }
//                            else
//                            {
//                                APP_PRINT("\r\n num10 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                            }
//                        }
//                        else
//                        {
//                            APP_PRINT("\r\n num10 Read transaction failed.\r\n");
//                        }
//
//
//        // 11. 라인 앰프 활성화
//        reg[0] = 0x6D; reg[1] = 0x80; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                                if (FSP_SUCCESS == err)
//                                {
//                                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                                }
//
//                                // Step 3: 쓴 값과 읽은 값 비교하기
//                                if (FSP_SUCCESS == err)
//                                {
//                                    if (reg[1] == read_value)
//                                    {
//                                        APP_PRINT("\r\n num11 Write/Read Success! Value: 0x%X\n", read_value);
//                                    }
//                                    else
//                                    {
//                                        APP_PRINT("\r\n num11 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                                    }
//                                }
//                                else
//                                {
//                                    APP_PRINT("\r\n num11 Read transaction failed.\r\n");
//                                }
//
//
//
//        // 12. 시스템 컨트롤러로 전체 활성화
//        reg[0] = 0x51; reg[1] = 0xD9; R_IIC_MASTER_Write(&g_i2c_master0_ctrl, reg, 2, false);
//
//        // 🚨 방금 쓴 레지스터의 값을 다시 읽어오기
//                // 먼저 읽을 주소를 알려주고(Write), 그 다음 값을 읽어옵니다(Read).
//                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//                // --- 🚨 여기서부터가 수정된 부분입니다 ---
//
//                // Step 1: 읽고 싶은 레지스터의 주소를 먼저 알려준다 (Write).
//                // 마지막 파라미터 'true'는 통신을 끝내지 않고 유지하라는 의미(restart).
//                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg[0], 1, true);
//
//                // Step 2: 주소를 알려준 직후, 해당 레지스터에서 데이터를 읽어온다 (Read).
//                if (FSP_SUCCESS == err)
//                {
//                    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_value, 1, false);
//                }
//
//                // Step 3: 쓴 값과 읽은 값 비교하기
//                if (FSP_SUCCESS == err)
//                {
//                    if (reg[1] == read_value)
//                    {
//                        APP_PRINT("\r\n num12 Write/Read Success! Value: 0x%X\n", read_value);
//                    }
//                    else
//                    {
//                        APP_PRINT("\r\n num12 Write/Read FAILED! Wrote: 0x%X, Read: 0x%X\n", reg[1], read_value);
//                    }
//                }
//                else
//                {
//                    APP_PRINT("\r\n num12 Read transaction failed.\r\n");
//                }
//
//                target_reg_addr = 0x03;
//                err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &target_reg_addr, 1, true);
//
//                err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, data_read_from_reg, 8, false);
//
//                for(uint8_t i=0; i<8; i++){
//                    APP_PRINT(" -> Write/Read1 SUCCESS! Value: 0x%X\n", data_read_from_reg[i]);
//                }
//
//        // 13. 안정화 대기
//        R_BSP_SoftwareDelay(250, BSP_DELAY_UNITS_MILLISECONDS);
//        APP_PRINT("DA7212 Codec Configuration Complete.\n");
//
//
        /* ========================================================================= */
        /* 3. SSI(I2S) 및 GPT(타이머) 초기화 및 시작                             */
        /* ========================================================================= */
        err = R_SSI_Open(&g_i2s_ctrl, &g_i2s_cfg);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\nSSI Open Failed.\r\n");
            APP_ERR_TRAP(err);
        }

        err = R_GPT_Open(&g_timer_ctrl, &g_timer_cfg);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\nGPT Open Failed.\r\n");
            APP_ERR_TRAP(err);
        }

        err = R_GPT_Start(&g_timer_ctrl);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\nGPT Start Failed.\r\n");
            APP_ERR_TRAP(err);
        }
        APP_PRINT("SSI and GPT Started.\n");


        /* ========================================================================= */
        /* 4. 오디오 데이터 준비 및 첫 전송 시작                                   */
        /* ======================================================================*/
        ssi_example_calculate_samples(!g_buffer_index);

        ssi_example_write(); // 첫 데이터 전송 시작!
        APP_PRINT("\r\nFirst audio frame sent. Entering main loop.\n");

        /* ========================================================================= */
        /* 5. 메인 루프 진입 (필수!!!)                                             */
        /* ========================================================================= */
        while (1)
        {
            // 백그라운드에서 인터럽트가 오디오 출력을 처리하는 동안
            // 메인 루프는 여기서 안정적으로 대기합니다.
            __WFI();
        }






}

/***********************************************************************************************************************
 *  @brief      This function generates stereo audio samples in the form of a sine wave and stores them into 2 buffers
 *              for later transmission.
 *  @param[IN]  buffer_index
 *  @retval     None
 **********************************************************************************************************************/
static void ssi_example_calculate_samples(uint32_t buffer_index)
{
    uint32_t pclkd_get_hz = R_FSP_SystemClockHzGet (FSP_PRIV_CLOCK_PCLKD); /* PCLKD clock (GPT clock) */
    uint32_t period = pclkd_get_hz / g_timer_cfg.period_counts;

    /* Audio sample frequency (bit_clock (Hz) = sampling_frequency (Hz) * channels * system_word_bits
     * (the bit clock for transmitting 2 channels of 16-bit data),
     * audio_clock (Hz) = desired_bit_clock (Hz) * bit_clock_divider) */
    uint32_t audio_sample = (period / (2 * 2 * 16));

    static uint32_t t = 0U;
    /* Create a stereo sine wave. Using formula sample = sin(2 * pi * tone_frequency * t / sampling_frequency) */
    uint32_t freq = SSI_STREAMING_EXAMPLE_TONE_FREQUENCY_HZ;
    for (uint32_t i = 0; i < SSI_STREAMING_EXAMPLE_SAMPLES_PER_CHUNK / 2; i += 1)
    {
        float input = (float) ((2.0f * M_PI * freq * t) / (audio_sample));
        t++;
        /* Store sample twice, once for left channel and once for right channel */
        int16_t sample = (int16_t) ((INT16_MAX * sinf (input)));
        int32_t sample_32bit = (int32_t)(sample << 8);

        g_src_buff[buffer_index][2 * i] = sample_32bit;
        g_src_buff[buffer_index][2 * i + 1] = sample_32bit;
    }
    /* Data is ready to be sent in the interrupt */
    g_data_ready = true;
}

/***********************************************************************************************************************
 *  @brief      This function is responsible for transferring data between two buffers
 *              (source buffer and destination buffer).
 *  @param[IN]  None
 *  @retval     None
 **********************************************************************************************************************/
static void ssi_example_write()
{
    /* Setting g_dest_buff to zero */
    memset(g_dest_buff, 0, sizeof(g_dest_buff));

    /* Transfer data. This call is non-blocking */
    fsp_err_t err = R_SSI_Write(&g_i2s_ctrl,\
                                    (uint8_t *)g_src_buff[g_buffer_index],\
                                    SSI_STREAMING_EXAMPLE_SAMPLES_PER_CHUNK * sizeof(int32_t));
    if (FSP_SUCCESS == err)
    {
        /* Switch the buffer after data is sent */
        g_buffer_index = !g_buffer_index;
        /* Allow loop to calculate next buffer only if transmission was successful. Clear flag. */
        g_data_ready = false;
        APP_PRINT("\r\nSEND FINISH");
    }
    else
    {
        /* Handle error */
        APP_ERR_PRINT("\r\nR_SSI_WriteRead API failed, Closing SSI and GPT\r\n");
        deinit_ssi();
        deinit_gpt();
        /* Trap here */
        APP_ERR_TRAP(err);
        /* Getting here most likely means a transmit overflow occurred before the  transmit buffer could be reloaded
         * The application must wait until the SSI is idle, then restart transmission
         * In this example, the idle callback transmits data or resets the flag g_send_data_in_main_loop */
    }
  }

/***********************************************************************************************************************
 *  @brief      This function gets SSI events
 *  @param[IN]  p_args
 *  @retval     None
 **********************************************************************************************************************/
void i2s_callback(i2s_callback_args_t *p_args)
{
    if( NULL != p_args)
    {
        /* Capture callback event for validating the i2s transfer event */
        g_i2s_event = p_args->event;
    }
    /* Reload the transmit buffer if we hit the transmit water mark or restart transmission if the SSI is idle
     * because it was stopped after a transmit buffer overflow */
        if ((I2S_EVENT_TX_EMPTY == p_args->event) || (I2S_EVENT_IDLE == p_args->event))
    {
            if (g_data_ready)
            {
                /* Reload the transmit buffer and handle errors */
                ssi_example_write();
            }
            else
            {
                /* Data was not ready yet, send it in the main loop */
                g_send_data_in_main_loop = true;
            }
     }

}

//typedef enum e_i2c_master_event
//{
//    I2C_MASTER_EVENT_ABORTED     = 1,  ///< A transfer was aborted
//    I2C_MASTER_EVENT_RX_COMPLETE = 2,  ///< A receive operation was completed successfully
//    I2C_MASTER_EVENT_TX_COMPLETE = 3,  ///< A transmit operation was completed successfully
//    I2C_MASTER_EVENT_START       = 4,  ///< I2C sent a start condition
//    I2C_MASTER_EVENT_BYTE_ACK    = 5,  ///< I2C finished sending/receiving 1 data byte
//} i2c_master_event_t;


void i2c_callback(i2c_master_callback_args_t *p_args)
{
//    FSP_PARAMETER_NOT_USED(p_args);
    i2c_master_event_t g_i2c_callback_event = p_args->event;

    switch(g_i2c_callback_event){
        case I2C_MASTER_EVENT_ABORTED:
            APP_PRINT("\r\n I2C_MASTER_EVENT_ABORTED");
            break;
        case I2C_MASTER_EVENT_RX_COMPLETE:
            APP_PRINT("\r\n I2C_MASTER_EVENT_RX_COMPLETE");
            for(uint8_t i=0; i<8; i++){
                  APP_PRINT(" -> Write/Read1 SUCCESS! Value: 0x%X\n", data_read_from_reg[i]);
            }
            break;
        case I2C_MASTER_EVENT_TX_COMPLETE:
            APP_PRINT("\r\n I2C_MASTER_EVENT_TX_COMPLETE");
            break;
        case I2C_MASTER_EVENT_START:
            APP_PRINT("\r\n I2C_MASTER_EVENT_START");
            break;
        case I2C_MASTER_EVENT_BYTE_ACK:
            APP_PRINT("\r\n I2C_MASTER_EVENT_BYTE_ACK");
            break;
        default:
            APP_PRINT("\r\n I2C_MASTER_EVENT_unknown");
            break;

    }
}




/***********************************************************************************************************************
 *  @brief      This function is used to close SSI module.
 *  @param[IN]  None
 *  @retval     None
 **********************************************************************************************************************/
static void deinit_ssi(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Close SSI Module */
    err = R_SSI_Close(&g_i2s_ctrl);
    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nR_SSI_Close API Failed\r\n");
    }
}

/***********************************************************************************************************************
 *  @brief      This function is used to close GPT module.
 *  @param[IN]  None
 *  @retval     None
 **********************************************************************************************************************/
static void deinit_gpt(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Close GPT module */
    err = R_GPT_Close(&g_timer_ctrl);
    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nR_GPT_Close API Failed\r\n");
    }
}

