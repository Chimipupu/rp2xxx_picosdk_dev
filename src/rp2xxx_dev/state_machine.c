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
#include "ext_mcu_com.h"
#include "app_main.h"

EXT_COM_FLAME_T s_emc_flame;
uint8_t g_sm_state = SM_NONE;

static void sm_exec(EXT_COM_FLAME_T *p_flame, uint8_t state);

/**
 * @brief ステートマシーン実行関数
 * 
 * @param state 遷移するステート
 * @param p_flame フレーム構造体ポインタ
 */
static void sm_exec(EXT_COM_FLAME_T *p_flame, uint8_t state)
{
    switch (state)
    {
        case SM_TEST_PROC:
            if(p_flame->BIT.CMD == EMC_CMD_TEST_REQ) {
                p_flame->BIT.CMD = EMC_CMD_TEST_RES;
                p_flame->BIT.LEN = 1;
                p_flame->BIT.DATA = MULTI_CORE_TEST_DATA;
                emc_tx_flame(p_flame);
                printf("[DEBUG] SM_CMD_TEST_RES Flame TX. Data = 0x%04X\n", p_flame->BIT.DATA);
            }
            g_sm_state = SM_IDLE;
            break;

        case SM_NONE:
        case SM_IDLE:
        default:
            _NOP();
            break;
    }
}

/**
 * @brief スレーブ用ステートマシーン 初期化関数
 * 
 */
void sm_slave_init(void)
{
    g_sm_state = SM_NONE;
    s_emc_flame.DWORD = 0;
}

/**
 * @brief スレーブ用ステートマシーン メイン関数
 * 
 */
void sm_slave_main(void)
{
    // フレーム受信
    emc_rx_flame(&s_emc_flame);
    if(s_emc_flame.DWORD != 0) {
        // フレーム解析
        emc_parse_sm_flame(&s_emc_flame);

        // ステート実行
        sm_exec(&s_emc_flame, g_sm_state);
    }
}