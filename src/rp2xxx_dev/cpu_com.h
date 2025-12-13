/**
 * @file cpu_com.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief CPU間通信
 * @version 0.1
 * @date 2025-12-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef EXT_MCU_COM_H
#define EXT_MCU_COM_H

#define FLAME_DATA_BUF_SIZE    0xFF

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_EXT_MCU_COM_USE

// --------------------------------------------------------------------------
// [CPU番号]
#define CPU_CORE_0               0
#define CPU_CORE_1               1

// [CPU間データ]
#define CPU_COM_DATA_PROC_RGB     0x1234
#define CPU_COM_DATA_TEST         0xABCD

// --------------------------------------------------------------------------

// CPU間通信フレームコマンド
typedef enum {
    CPU_COM_CMD_NONE         = 0x00, // コマンドなし
    CPU_COM_CMD_TEST_REQ     = 0x70, // テスト要求コマンド
    CPU_COM_CMD_WAIT_RES     = 0x71, // 応答待ち
    CPU_COM_CMD_TEST_RES     = 0x72, // テスト要求応答コマンド
} e_cpu_com_cmd;

// CPU間通信フレーム構造体
typedef union {
    uint32_t DWORD;
    struct {
        uint32_t DATA:16;   // bit[15:0]:  データ 16bit
        uint32_t LEN:8;     // bit[23:16]: データワード長
        uint32_t CMD:7;     // bit[30:24]: コマンド 7bit
        uint32_t CPUx:1;    // bit31:      CPU番号 1bit
    }BIT;
} CPU_COM_FLAME_T;

void cpu_com_tx_flame(CPU_COM_FLAME_T *p_flame);
void cpu_com_rx_flame(CPU_COM_FLAME_T *p_flame);
void cpu_com_parse_sm_flame(CPU_COM_FLAME_T *p_flame);
void cpu_com_init(void);
void cpu_com_main(uint8_t cpu_no);

#endif // EXT_MCU_COM_H