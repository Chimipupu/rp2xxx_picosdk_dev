/**
 * @file state_machine.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief ステートマシーン (RP2350 マルチコア制御用)
 * @version 0.1
 * @date 2025-09-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "state_machine.h"
#include "app_main.h"
#include "muc_rpxxx_util.h"

// マスター関連
static SM_FLAME_T s_sm_master_flame;

// スレーブ関連
static uint8_t s_state = SM_NONE;
static SM_FLAME_T s_sm_slave_flame;

static uint8_t s_read_idx = 0;
static uint8_t s_write_idx = 0;
static uint16_t s_flame_data_buf[SM_FLAME_DATA_BUF_SIZE] = {0};

static void parse_sm_flame(SM_FLAME_T *p_flame);
static void sm_exec(SM_FLAME_T *p_flame, uint8_t state);
static void sm_test(void);

/**
 * @brief フレーム解析関数(for スレーブ)
 * 
 * @param p_flame フレーム構造体ポインタ
 */
static void parse_sm_flame(SM_FLAME_T *p_flame)
{
    uint8_t i;

    if(p_flame->BIT.LEN > 0) {
        printf("[DEBUG] RX Flame : CMD=0x%02X, LEN=0x%02X\n", p_flame->BIT.CMD, p_flame->BIT.LEN);
    } else {
        printf("[DEBUG] RX Flame : CMD=0x%02X\n", p_flame->BIT.CMD);
    }

    switch (p_flame->BIT.CMD)
    {
        case SM_CMD_TEST_REQ:
            printf("[DEBUG] SM_CMD_TEST_REQ Flame RX\n");
            s_state = SM_TEST_PROC;
            break;

        default:
            for(i = 0; i < p_flame->BIT.LEN; i++)
            {
                printf("[DEBUG] RX Flame : DATA=0x%04X", p_flame->BIT.DATA);
                s_flame_data_buf[s_write_idx] = p_flame->BIT.DATA;
                s_write_idx = (s_write_idx + 1) % SM_FLAME_DATA_BUF_SIZE;
            }
            break;
    }
}

/**
 * @brief ステートマシーン実行関数
 * 
 * @param state 遷移するステート
 * @param p_flame フレーム構造体ポインタ
 */
static void sm_exec(SM_FLAME_T *p_flame, uint8_t state)
{
    switch (state)
    {
        case SM_TEST_PROC:
            if(p_flame->BIT.CMD == SM_CMD_TEST_REQ) {
                p_flame->BIT.CMD = SM_CMD_TEST_RES;
                p_flame->BIT.LEN = 1;
                p_flame->BIT.DATA = MULTI_CORE_TEST_DATA;
                sm_flame_tx(p_flame);
                printf("[DEBUG] SM_CMD_TEST_RES Flame TX. Data = 0x%04X\n", p_flame->BIT.DATA);
            }
            s_state = SM_IDLE;
            break;

        case SM_NONE:
        case SM_IDLE:
        default:
            _NOP();
            break;
    }
}

/**
 * @brief フレーム送信関数
 * 
 * @param p_flame フレーム構造体ポインタ
 */
void sm_flame_tx(SM_FLAME_T *p_flame)
{
    set_multicore_fifo(p_flame->DWORD);
}

/**
 * @brief フレーム受信関数
 * 
 * @param p_flame フレーム構造体ポインタ
 */
void sm_flame_rx(SM_FLAME_T *p_flame)
{
    p_flame->DWORD = get_multicore_fifo();
}

#if 1

static void sm_test(void)
{
    SM_FLAME_T flame;

    flame.BIT.CMD = SM_CMD_TEST_REQ;
    flame.BIT.LEN = 0;
    flame.BIT.DATA = 0;
    sm_flame_tx(&flame);
}

/**
 * @brief マスター用ステートマシーン 初期化関数
 * 
 */
void sm_master_init(void)
{
    s_sm_master_flame.DWORD = 0;
}

/**
 * @brief マスター用ステートマシーン メイン関数
 * 
 */
void sm_master_main(void)
{
    sm_test();

    sleep_ms(100);

    sm_flame_rx(&s_sm_master_flame);
    if(s_sm_master_flame.DWORD != 0) {
        if(s_sm_master_flame.BIT.CMD == SM_CMD_TEST_RES) {
            if(s_sm_master_flame.BIT.DATA == MULTI_CORE_TEST_DATA){
                printf("[INFO] SM TEST OK\n");
            } else {
                printf("[ERROR] SM TEST NG\n");
            }
        }
    }
}
#endif

#if 1
/**
 * @brief スレーブ用ステートマシーン 初期化関数
 * 
 */
void sm_slave_init(void)
{
    s_state = SM_NONE;
    s_sm_slave_flame.DWORD = 0;
}

/**
 * @brief スレーブ用ステートマシーン メイン関数
 * 
 */
void sm_slave_main(void)
{
    // フレーム受信
    sm_flame_rx(&s_sm_slave_flame);
    if(s_sm_slave_flame.DWORD != 0) {
        // フレーム解析
        parse_sm_flame(&s_sm_slave_flame);

        // ステート実行
        sm_exec(&s_sm_slave_flame, s_state);
    }
}
#endif