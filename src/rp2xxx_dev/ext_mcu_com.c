/**
 * @file ext_mcu_com.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief マイコン間通信
 * @version 0.1
 * @date 2025-09-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "ext_mcu_com.h"
#include "app_main.h"
#include "muc_rpxxx_util.h"
#include "state_machine.h"

extern uint8_t g_sm_state;

static uint8_t s_read_idx = 0;
static uint8_t s_write_idx = 0;
static uint16_t s_flame_data_buf[FLAME_DATA_BUF_SIZE] = {0};

static bool s_is_test_end = false;

/**
 * @brief マイコン間通信フレーム解析関数
 * 
 * @param p_flame マイコン間通信フレーム構造体ポインタ
 */
void emc_parse_sm_flame(EXT_COM_FLAME_T *p_flame)
{
    uint8_t i;

    if(p_flame->BIT.LEN > 0) {
        printf("[DEBUG] RX Flame : CMD=0x%02X, LEN=0x%02X\n", p_flame->BIT.CMD, p_flame->BIT.LEN);
    } else {
        printf("[DEBUG] RX Flame : CMD=0x%02X\n", p_flame->BIT.CMD);
    }

    switch (p_flame->BIT.CMD) {
        case EMC_CMD_TEST_REQ:
            printf("[DEBUG] SM_CMD_TEST_REQ Flame RX\n");
            g_sm_state = SM_TEST_PROC;
            break;

        default:
            for(i = 0; i < p_flame->BIT.LEN; i++)
            {
                printf("[DEBUG] RX Flame : DATA=0x%04X", p_flame->BIT.DATA);
                s_flame_data_buf[s_write_idx] = p_flame->BIT.DATA;
                s_write_idx = (s_write_idx + 1) % FLAME_DATA_BUF_SIZE;
            }
            break;
    }
}

/**
 * @brief マイコン間通信フレーム送信関数
 * 
 * @param p_flame マイコン間通信フレーム構造体ポインタ
 */
void emc_tx_flame(EXT_COM_FLAME_T *p_flame)
{
    set_multicore_fifo(p_flame->DWORD);
}

/**
 * @brief マイコン間通信フレーム受信関数
 * 
 * @param p_flame マイコン間通信フレーム構造体ポインタ
 */
void emc_rx_flame(EXT_COM_FLAME_T *p_flame)
{
    p_flame->DWORD = get_multicore_fifo();
}

/**
 * @brief マイコン間通信 初期化関数
 * 
 */
void emc_init(void)
{
    // TODO
}

/**
 * @brief マイコン間通信 メイン関数
 * 
 */
void emc_main(void)
{
    // TODO
}

// (DEBUG)テスト関連
#ifdef DEBUG_EXT_MCU_COM_USE
void emc_test(void)
{
    EXT_COM_FLAME_T flame;

    if(s_is_test_end != true) {
        flame.BIT.CMD = EMC_CMD_TEST_REQ;
        flame.BIT.LEN = 0;
        flame.BIT.DATA = 0;
        emc_tx_flame(&flame);

        if(flame.DWORD != 0) {
            if(flame.BIT.CMD == EMC_CMD_TEST_RES) {
                if(flame.BIT.DATA == MULTI_CORE_TEST_DATA){
                    printf("[INFO] EMC TEST OK\n");
                    s_is_test_end = true;
                } else {
                    printf("[ERROR] EMC TEST NG\n");
                    s_is_test_end = false;
                }
            }
        }
    }
}
#endif // DEBUG_EXT_MCU_COM_USE