/***********************************************************************************************************************
 * File Name    : ssi_ep.c
 * Description  : Contains data structures and functions used in ssi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************/

#include "hal_data.h"
#include "common_utils.h"
#include "ssi_ep.h"
#include "da7212.h"
#include <math.h>

/* --- Private Function Prototypes --- */
void da7212_init(void);
void da7212_write(uint8_t reg, uint8_t data);
static uint8_t da7212_read(uint8_t reg);
static bool da7212_verify_registers(void); // ⭐️ 레지스터 검증 함수 선언
static void audio_data_transfer(void);
static void audio_data_calculate(uint32_t buffer_index);

/* --- Global Variables --- */
static volatile i2c_master_event_t g_i2c_callback_event;
//static volatile i2s_event_t g_i2s_event = I2S_EVENT_TX_EMPTY;
static volatile uint8_t g_buffer_index = 0;
static int32_t g_src_buff[2][256];
static volatile bool g_play_button_pressed = false;
static volatile bool g_stop_button_pressed = false;

// 1. 재생할 음들의 주파수 (Hz) 정의
#define HIGH_NOTE_FREQ 880  // 높은 '라'
#define LOW_NOTE_FREQ  440  // 중간 '라'

// 2. 멜로디 순서 정의 (높은 라 -> 낮은 라 -> 높은 라 -> 낮은 라)
const uint32_t g_song_melody[] = {HIGH_NOTE_FREQ, LOW_NOTE_FREQ, HIGH_NOTE_FREQ, LOW_NOTE_FREQ};

// 3. 멜로디에 총 몇 개의 음이 있는지 자동으로 계산
const uint32_t g_total_notes_in_song = sizeof(g_song_melody) / sizeof(g_song_melody[0]);

// 4. 하나의 음을 몇 초 동안 연주할지, 샘플레이트는 몇인지 정의
#define NOTE_DURATION_SECONDS 0.5f
#define SAMPLE_RATE 48000
const uint32_t g_samples_per_note = (uint32_t)(NOTE_DURATION_SECONDS * SAMPLE_RATE);

// =========================================================================
// 현재 연주 상태 저장 (Performance State)
// =========================================================================
static uint32_t g_current_note_index = 0;      // 현재 멜로디의 몇 번째 음을 연주 중인가?
static uint32_t g_samples_played_for_note = 0; // 현재 음을 몇 개의 샘플 동안 연주했는가?

bsp_io_level_t led_level = BSP_IO_LEVEL_LOW; // LED 상태를 기억할 변수

void switch_callback1(external_irq_callback_args_t *p_args); // 1번 스위치 콜백 함수 선언
void switch_callback2(external_irq_callback_args_t *p_args); // 2번 스위치 콜백 함수 선언


/*******************************************************************************************************************//**
 * Main entry function.
 **********************************************************************************************************************/
void ssi_entry(void)
{
    fsp_err_t err;

    R_BSP_PinAccessEnable();

    APP_PRINT("\r\n--- DA7212 Audio Codec Speaker Example ---\r\n");

    /* 1. I2C 드라이버 초기화 */
    R_IIC_MASTER_CallbackSet(&g_i2c_master0_ctrl, i2c_callback, NULL, NULL);
    err = R_IIC_MASTER_Open(&g_i2c_master0_ctrl, &g_i2c_master0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("I2C Open Failed.\r\n");
        APP_ERR_TRAP(err);
    }
    APP_PRINT("I2C Bus Opened Successfully.\n");

    /* 2. DA7212 코덱 초기화 */
    da7212_init();
    APP_PRINT("DA7212 Codec Initialized.\n");

    /* ⭐️ ========================================================================= */
    /* ⭐️ 3. 레지스터 값 읽기 및 검증 (새로 추가된 부분)                         */
    /* ⭐️ ========================================================================= */
    if (da7212_verify_registers())
    {
        APP_PRINT(" -> All registers verified successfully!\n");
    }
    else
    {
        APP_PRINT(" -> Register verification FAILED. Halting.\n");
//        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* 4. SSI(I2S) 및 GPT(타이머) 초기화 */
    err = R_SSI_Open(&g_i2s_ctrl, &g_i2s_cfg);
    if (FSP_SUCCESS != err) { APP_PRINT("SSI Open Failed.\n"); APP_ERR_TRAP(err); }

    err = R_GPT_Open(&g_timer_ctrl, &g_timer_cfg);
    if (FSP_SUCCESS != err) { APP_PRINT("GPT Open Failed.\n"); APP_ERR_TRAP(err); }

    err = R_GPT_Start(&g_timer_ctrl);
    if (FSP_SUCCESS != err) { APP_PRINT("GPT Start Failed.\n"); APP_ERR_TRAP(err); }
    APP_PRINT("SSI and GPT Started.\n");


    APP_PRINT("System Initialized. Waiting for button press...\n");
    APP_PRINT("Press SW1 to Play, SW2 to Stop.\n");

    err = R_ICU_ExternalIrqOpen(&g_external_irq0_ctrl, &g_external_irq0_cfg);
    if (FSP_SUCCESS != err) { APP_ERR_TRAP(err); }
    err = R_ICU_ExternalIrqEnable(&g_external_irq0_ctrl);
    if (FSP_SUCCESS != err) { APP_ERR_TRAP(err); }

    err = R_ICU_ExternalIrqOpen(&g_external_irq1_ctrl, &g_external_irq1_cfg);
    if (FSP_SUCCESS != err) { APP_ERR_TRAP(err); }
    err = R_ICU_ExternalIrqEnable(&g_external_irq1_ctrl);
    if (FSP_SUCCESS != err) { APP_ERR_TRAP(err); }

    while (1){
        // 1번 스위치(재생)가 눌렸는지 확인
        if (g_play_button_pressed)
        {
            g_play_button_pressed = false; // 깃발을 내린다

            APP_PRINT("SW1 Pressed: Starting Playback...\n");

            // 오디오 버퍼를 채우고 첫 전송을 시작
            audio_data_calculate(0);
            audio_data_calculate(1);
            audio_data_transfer();

            // I2S 콜백(자동화 파이프라인)을 활성화!
            R_SSI_CallbackSet(&g_i2s_ctrl, i2s_callback, NULL, NULL);
        }

        // 2번 스위치(정지)가 눌렸는지 확인
        if (g_stop_button_pressed)
        {
            g_stop_button_pressed = false; // 깃발을 내린다

            APP_PRINT("SW2 Pressed: Stopping Playback...\n");

            // I2S 콜백을 비활성화하여 자동화 파이프라인을 멈춘다!
            R_SSI_CallbackSet(&g_i2s_ctrl, NULL, NULL, NULL);

            // (선택사항) 더 확실한 정지를 위해 코덱 앰프를 음소거(Mute)
            // da7212_write(0x6F, 0x58); // Mute, Gain -9dB
        }
    }








//    /* 5. 오디오 데이터 준비 및 첫 전송 시작 */
//    audio_data_calculate(0);
//    audio_data_calculate(1);
//    audio_data_transfer();
//    APP_PRINT("Audio streaming started. Entering main loop...\n");

}

/*******************************************************************************************************************//**
 * 1번 스위치 (재생) 콜백 함수
 **********************************************************************************************************************/
void switch_callback1(external_irq_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    g_play_button_pressed = true; // '재생' 깃발을 올린다!
}

/*******************************************************************************************************************//**
 * 2번 스위치 (정지) 콜백 함수
 **********************************************************************************************************************/
void switch_callback2(external_irq_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    g_stop_button_pressed = true; // '정지' 깃발을 올린다!
}

/* (i2c_callback, i2s_callback 함수는 이전과 동일) */
void i2c_callback(i2c_master_callback_args_t *p_args){ g_i2c_callback_event = p_args->event; }
void i2s_callback(i2s_callback_args_t *p_args)
{
    if (I2S_EVENT_TX_EMPTY == p_args->event) {
//        APP_PRINT("\nI2S_EVENT_TX_EMPTY audio_data_transfer()");
        audio_data_transfer();
    }
}


/*******************************************************************************************************************//**
 * DA7212 코덱 초기화 (올바른 순서 적용)
 **********************************************************************************************************************/
void da7212_init(void)
{
    da7212_write(0x1D, 0x80);
    da7212_write(0xFD, 0x01); R_BSP_SoftwareDelay(40, BSP_DELAY_UNITS_MILLISECONDS);
    da7212_write(0x90, 0x90);
    da7212_write(0x23, 0x08); R_BSP_SoftwareDelay(25, BSP_DELAY_UNITS_MILLISECONDS);
    da7212_write(0x24, 0x00);
    da7212_write(0x25, 0x00);
    da7212_write(0x26, 0x20);
    da7212_write(0x27, 0x84);

    da7212_write(0x22, 0x0B);
    da7212_write(0x28, 0x02);
    da7212_write(0x29, 0xC8);
    da7212_write(0x2A, 0x22);
    da7212_write(0x47, 0xA1);
    da7212_write(0x4B, 0x08);
    da7212_write(0x4C, 0x08);
    da7212_write(0x6E, 0x98);
    da7212_write(0x6F, 0x98);
    da7212_write(0x4A, 0xB8);   //음향조절? 0X98 ~ 0xBF
    da7212_write(0x51, 0xF1);
//    da7212_write(0x4A, 0x30); 해드폰 잭 경로의 출력 조절?
//    da7212_write(0x6D, 0xA8); 스피커가 아닌 해드폰 앰프?

    R_BSP_SoftwareDelay(250, BSP_DELAY_UNITS_MILLISECONDS);
}

/*******************************************************************************************************************//**
 * ⭐️ 레지스터 검증 함수 (새로 추가)
 **********************************************************************************************************************/
static bool da7212_verify_registers(void)
{
    bool success = true;
    uint8_t read_val;

    // 검증할 레지스터와 기대값 목록
    const struct { uint8_t reg; uint8_t expected_val; } reg_map[] = {
        {0xFD, 0x01}, {0x23, 0x08}, {0x24, 0x00}, {0x25, 0x00}, {0x26, 0x20}, {0x27, 0x84}, {0x22, 0x0B}, {0x28, 0x02},
        {0x29, 0xC8}, {0x2A, 0x22}, {0x47, 0xA1},  {0x4B, 0x08}, {0x4C, 0x08}, {0x6E, 0x98}, {0x6F, 0x98}, {0x51, 0xF1}
    };

    APP_PRINT("\nVerifying DA7212 Registers...\n");
    APP_PRINT("=================================================\n");
    APP_PRINT("| Register | Expected Value | Read Value | Result |\n");
    APP_PRINT("-------------------------------------------------\n");

    for(uint32_t i = 0; i < sizeof(reg_map)/sizeof(reg_map[0]); i++)
    {
        read_val = da7212_read(reg_map[i].reg);
        bool match = (read_val == reg_map[i].expected_val);
        APP_PRINT("|   0x%02X   |      0x%02X      |    0x%02X    |  %s  |\n",
                  reg_map[i].reg, reg_map[i].expected_val, read_val, match ? "PASS" : "FAIL");
        if (!match)
        {
            success = false;
        }
    }
    APP_PRINT("=================================================\n");
    return success;
}


/*******************************************************************************************************************//**
 * DA7212 레지스터 쓰기 헬퍼 함수
 **********************************************************************************************************************/
void da7212_write(uint8_t reg, uint8_t data)
{
    fsp_err_t err;
    uint32_t timeout_ms;
    uint8_t buf[2] = {reg, data};

    g_i2c_callback_event = (i2c_master_event_t)0;
    err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, buf, 2, false);
    if (FSP_SUCCESS != err) { APP_PRINT("I2C Write Cmd Err for Reg 0x%02X\n", reg); return; }

    timeout_ms = 100;
    while ((I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) && (I2C_MASTER_EVENT_ABORTED != g_i2c_callback_event) && timeout_ms)
    {
        R_BSP_SoftwareDelay(40, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }
    if(I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) {APP_PRINT("I2C Write Failed for Reg 0x%02X\n", reg);}
}

/*******************************************************************************************************************//**
 * DA7212 레지스터 읽기 헬퍼 함수
 **********************************************************************************************************************/
uint8_t da7212_read(uint8_t reg)
{
    fsp_err_t err;
    uint32_t  timeout_ms;
    uint8_t   read_val = 0xFF;

    g_i2c_callback_event = (i2c_master_event_t)0;
    err = R_IIC_MASTER_Write(&g_i2c_master0_ctrl, &reg, 1, true);
    if(FSP_SUCCESS != err) { return read_val; }
    timeout_ms = 100;
    while ((I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) && (I2C_MASTER_EVENT_ABORTED != g_i2c_callback_event) && timeout_ms)
    {
         R_BSP_SoftwareDelay(40, BSP_DELAY_UNITS_MILLISECONDS);
         timeout_ms--;
    }
    if (I2C_MASTER_EVENT_TX_COMPLETE != g_i2c_callback_event) { return read_val; }

    g_i2c_callback_event = (i2c_master_event_t)0;
    err = R_IIC_MASTER_Read(&g_i2c_master0_ctrl, &read_val, 1, false);
    if(FSP_SUCCESS != err) { return 0xFF; }
    timeout_ms = 100;
    while ((I2C_MASTER_EVENT_RX_COMPLETE != g_i2c_callback_event) && (I2C_MASTER_EVENT_ABORTED != g_i2c_callback_event) && timeout_ms)
    {
        R_BSP_SoftwareDelay(40, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }
    if (I2C_MASTER_EVENT_RX_COMPLETE != g_i2c_callback_event) { return 0xFF;}

    return read_val;
}

/* (audio_data_transfer, audio_data_calculate 함수는 이전과 동일) */
static void audio_data_transfer(void)
{
    // 1. R_SSI_Write 함수의 반환값을 저장할 fsp_err_t 타입의 변수를 선언합니다.
    fsp_err_t err;

    // 2. R_SSI_Write 함수를 호출하고, 그 결과를 err 변수에 저장합니다.
    err = R_SSI_Write(&g_i2s_ctrl, (uint8_t *)g_src_buff[g_buffer_index], sizeof(g_src_buff[0]));

    // 3. err 변수의 값을 확인하여 성공/실패 여부를 RTT로 출력합니다.
    if (FSP_SUCCESS != err)
    {
        // R_SSI_Write가 실패했을 경우에만 메시지를 출력합니다.
        APP_PRINT("--> R_SSI_Write FAILED with error code: %u\r\n", err);
    }
    else
    {
        // 성공했을 경우에는 아무것도 출력하지 않습니다. (아래 '주의사항' 참고)
        APP_PRINT("\r\nSUCCESS");
    }

    // 4. 다음 전송을 위해 버퍼 인덱스를 교체하는 로직은 그대로 유지합니다.
    g_buffer_index = !g_buffer_index;

    audio_data_calculate(g_buffer_index);
}

static void audio_data_calculate(uint32_t buffer_index)
{

    static uint32_t t = 0U;

    uint32_t samples_per_chunk = sizeof(g_src_buff[0]) / sizeof(int32_t);

    for (uint32_t i = 0; i < samples_per_chunk / 2; i++)
    {
        // 1. 멜로디의 다음 음으로 넘어갈지 결정하는 부분 (이전과 동일)
        if (g_samples_played_for_note >= g_samples_per_note)
        {
            g_samples_played_for_note = 0;
            g_current_note_index++;
            if (g_current_note_index >= g_total_notes_in_song)
            {
                g_current_note_index = 0;
            }
            // 다음 음으로 변경되었음을 RTT 로그로 출력
            APP_PRINT("--> Note Changed to: %u Hz\r\n", g_song_melody[g_current_note_index]);

            led_level = !led_level;
            R_BSP_PinWrite(BSP_IO_PORT_06_PIN_00, led_level);
        }
    // 2. 현재 악보 순서에 맞는 주파수를 가져옵니다.
            uint32_t current_freq = g_song_melody[g_current_note_index];

            // 3. ✨ 사인(Sine) 함수를 이용해 해당 주파수의 샘플을 생성합니다. ✨
            float input = (float)((2.0f * M_PI * current_freq * t) / (SAMPLE_RATE));
            int16_t sample_16bit = (int16_t)(INT16_MAX * sinf(input));

            // 4. 시간을 계속 흘려보냅니다.
            t++;

            // 5. 생성된 16비트 샘플을 32비트로 변환하여 버퍼에 저장합니다.
            int32_t sample_32bit = (int32_t)(sample_16bit << 8);
            g_src_buff[buffer_index][2 * i]     = sample_32bit;
            g_src_buff[buffer_index][2 * i + 1] = sample_32bit;

            // 6. 현재 음을 연주한 샘플 개수를 1 증가시킵니다.
            g_samples_played_for_note++;
        }
//
//        // 2. 현재 연주할 음의 주파수를 가져옵니다. (이전과 동일)
//        uint32_t current_freq = g_song_melody[g_current_note_index];
//
//
//        // 3. 현재 주파수의 한 주기(period)가 몇 개의 샘플로 이루어지는지 계산합니다.
//        uint32_t period_in_samples = SAMPLE_RATE / current_freq;
//
//        // 4. 현재 시간(t)이 주기의 앞부분인지 뒷부분인지 확인합니다.
//        //    (t를 주기로 나눈 나머지를 이용)
//        if ((t % period_in_samples) < (period_in_samples / 2))
//        {
//            // 주기의 앞 절반 동안은 -> 최댓값
//            sample_16bit = INT16_MAX;
//        }
//        else
//        {
//            // 주기의 뒤 절반 동안은 -> 최솟값
//            sample_16bit = INT16_MIN;
//        }
//
//
//        // 5. 시간을 계속 흘려보냅니다.
//        t++;
//
//        // 6. 생성된 16비트 샘플을 32비트로 변환하여 버퍼에 저장합니다. (이전과 동일)
//        int32_t sample_32bit = (int32_t)(sample_16bit << 8);
//        g_src_buff[buffer_index][2 * i]     = sample_32bit;
//        g_src_buff[buffer_index][2 * i + 1] = sample_32bit;
//
//        // 7. 현재 음을 연주한 샘플 개수를 1 증가시킵니다. (이전과 동일)
//        g_samples_played_for_note++;
//    }







//
//
//
//        static uint32_t t = 0U;
//    uint32_t freq = 440;
//    uint32_t samples_per_chunk = sizeof(g_src_buff[0]) / sizeof(int32_t);
//    for (uint32_t i = 0; i < samples_per_chunk / 2; i++){
//        float input = (float)((2.0f * M_PI * freq * t) / (48000.0f));
//        t++;
//        int16_t sample_16bit = (int16_t)(INT16_MAX * sinf(input));
//        int32_t sample_32bit = (int32_t)(sample_16bit << 8);
//        g_src_buff[buffer_index][2 * i] = sample_32bit;
//        g_src_buff[buffer_index][2 * i + 1] = sample_32bit;
//    }

}
