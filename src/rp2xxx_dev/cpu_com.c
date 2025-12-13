/**
 * @file cpu_com.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CPU間通信
 * @version 0.1
 * @date 2025-12-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "cpu_com.h"

// --------------------------------------------------------------------------
// [CPU Core0用]
static cpu_com_flame_t s_cpu_core0_fifo_buf[CPU_FIFO_NUM] = {0};
static uint8_t s_idx_cpu_core0_fifo_buf = 0;

// [CPU Core0用]
static cpu_com_flame_t s_cpu_core1_fifo_buf[CPU_FIFO_NUM] = {0};
static uint8_t s_idx_cpu_core1_fifo_buf = 0;
// --------------------------------------------------------------------------
static bool s_is_test_end = false;
static void cpu_com_statemachine(uint8_t cpu_no, cpu_com_flame_t *p_flame);
// --------------------------------------------------------------------------
// [Static関数]
// --------------------------------------------------------------------------

/**
 * @brief CPU間通信 ステートマシーン
 * @param cpu_no CPU番号
 * @param p_flame CPU間通信フレーム構造体ポインタ
 */
static void cpu_com_statemachine(uint8_t cpu_no, cpu_com_flame_t *p_flame)
{
    uint32_t tmp = 0;
    cpu_com_flame_t tx_flame;

    switch (p_flame->BIT.CMD) {
        // 受信処理 ... CPU間通信 要求コマンド
        case CPU_COM_CMD_REQ:
            printf("[DEBUG] RX! REQ Flame: CPUx = %d, DATA = 0x%04X\n",
                p_flame->BIT.CPUx,
                p_flame->BIT.DATA
                );
            break;

        // 受信処理 ... CPU間通信 要求応答コマンド
        case CPU_COM_CMD_RES:
            printf("[DEBUG] RX! REQ Flame: CPUx = %d, DATA = 0x%04X\n", p_flame->BIT.CPUx, p_flame->BIT.DATA);
            break;

        // 受信処理 ... CPU間通信 テストコマンド
        case CPU_COM_CMD_TEST:
            printf("[DEBUG] RX! CPU COM Test Flame: CPUx = %d, DATA = 0x%04X\n", p_flame->BIT.CPUx, p_flame->BIT.DATA);

            if(p_flame->BIT.DATA == CPU_COM_DATA_TEST)
            {
                tx_flame.BIT.CMD = CPU_COM_CMD_RES;
                tx_flame.BIT.CPUx = cpu_no;
                tmp = p_flame->BIT.DATA;
                tmp = ~tmp;
                tx_flame.BIT.DATA = tmp;
                cpu_com_tx_flame(&tx_flame);
            }
            break;

        default:
            // NOP
            break;
    }
}

// --------------------------------------------------------------------------
// [API]
// --------------------------------------------------------------------------

/**
 * @brief CPU間通信フレーム送信関数
 * 
 * @param p_flame CPU間通信フレーム構造体ポインタ
 */
void cpu_com_tx_flame(cpu_com_flame_t *p_flame)
{
    set_multicore_fifo(p_flame->DWORD);
}

/**
 * @brief CPU間通信フレーム受信関数
 * 
 * @param p_flame CPU間通信フレーム構造体ポインタ
 */
void cpu_com_rx_flame(cpu_com_flame_t *p_flame)
{
    p_flame->DWORD = get_multicore_fifo();
}

/**
 * @brief CPU間通信 初期化関数
 * 
 */
void cpu_com_init(void)
{
    uint8_t i;

    s_idx_cpu_core0_fifo_buf = 0;
    s_idx_cpu_core1_fifo_buf = 0;

    for(i = 0; i < CPU_FIFO_NUM; i++)
    {
        s_cpu_core0_fifo_buf[i].DWORD = 0;
        s_cpu_core1_fifo_buf[i].DWORD = 0;
    }
}

/**
 * @brief CPU間通信 メイン
 * 
 * @param cpu_no CPU番号
 */
void cpu_com_main(uint8_t cpu_no)
{
    if(cpu_no == CPU_CORE_0) {
        s_cpu_core0_fifo_buf[s_idx_cpu_core0_fifo_buf].DWORD = 0;
        cpu_com_rx_flame(&s_cpu_core0_fifo_buf[s_idx_cpu_core0_fifo_buf]);
        s_idx_cpu_core0_fifo_buf = (s_idx_cpu_core0_fifo_buf + 1) % CPU_FIFO_NUM;
        cpu_com_statemachine(cpu_no, &s_cpu_core0_fifo_buf[s_idx_cpu_core0_fifo_buf]);
    } else if(cpu_no == CPU_CORE_1) {
        s_cpu_core0_fifo_buf[s_idx_cpu_core0_fifo_buf].DWORD = 0;
        cpu_com_rx_flame(&s_cpu_core1_fifo_buf[s_idx_cpu_core1_fifo_buf]);
        s_idx_cpu_core1_fifo_buf = (s_idx_cpu_core1_fifo_buf + 1) % CPU_FIFO_NUM;
        cpu_com_statemachine(cpu_no, &s_cpu_core1_fifo_buf[s_idx_cpu_core1_fifo_buf]);
    } else {
        // NOP
    }
}