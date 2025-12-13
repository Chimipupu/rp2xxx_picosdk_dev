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
#include "app_main.h"
#include "muc_rpxxx_util.h"

// --------------------------------------------------------------------------
static uint8_t s_read_idx = 0;
static uint8_t s_write_idx = 0;
static uint16_t s_flame_data_buf[FLAME_DATA_BUF_SIZE] = {0};
static bool s_is_test_end = false;
// --------------------------------------------------------------------------
/**
 * @brief CPU間通信フレーム解析関数
 * 
 * @param p_flame CPU間通信フレーム構造体ポインタ
 */
void cpu_com_parse_sm_flame(CPU_COM_FLAME_T *p_flame)
{
    uint8_t i;

    switch (p_flame->BIT.CMD) {
        case CPU_COM_CMD_TEST_REQ:
            printf("[DEBUG] SM_CMD_TEST_REQ Flame RX\n");
            break;

        case CPU_COM_CMD_WAIT_RES:
            if(p_flame->BIT.LEN > 0) {
                printf("[DEBUG] RX Flame : CMD=0x%02X, LEN=0x%02X\n", p_flame->BIT.CMD, p_flame->BIT.LEN);
            } else {
                printf("[DEBUG] RX Flame : CMD=0x%02X\n", p_flame->BIT.CMD);
            }
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
 * @brief CPU間通信フレーム送信関数
 * 
 * @param p_flame CPU間通信フレーム構造体ポインタ
 */
void cpu_com_tx_flame(CPU_COM_FLAME_T *p_flame)
{
    set_multicore_fifo(p_flame->DWORD);
}

/**
 * @brief CPU間通信フレーム受信関数
 * 
 * @param p_flame CPU間通信フレーム構造体ポインタ
 */
void cpu_com_rx_flame(CPU_COM_FLAME_T *p_flame)
{
    p_flame->DWORD = get_multicore_fifo();
}

/**
 * @brief CPU間通信 初期化関数
 * 
 */
void cpu_com_init(void)
{
    // TODO
}

/**
 * @brief CPU間通信 メイン
 * 
 * @param cpu_no CPU番号
 */
void cpu_com_main(uint8_t cpu_no)
{
    CPU_COM_FLAME_T flame;

    if(s_is_test_end != true) {
        flame.BIT.CMD = CPU_COM_CMD_TEST_REQ;
        flame.BIT.LEN = 0;
        flame.BIT.DATA = 0;
        flame.BIT.CPUx = cpu_no;
        cpu_com_tx_flame(&flame);

        if(flame.DWORD != 0) {
            if(flame.BIT.CMD == CPU_COM_CMD_TEST_RES) {
                if(flame.BIT.DATA == CPU_COM_DATA_TEST){
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