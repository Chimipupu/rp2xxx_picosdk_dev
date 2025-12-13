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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "muc_rpxxx_util.h"

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_EXT_MCU_COM_USE

// --------------------------------------------------------------------------
// [CPU番号]
#define CPU_CORE_0               0
#define CPU_CORE_1               1

// [CPU間データ]
#define CPU_COM_DATA_PROC_RGB    0x1234
#define CPU_COM_DATA_TEST        0xABCD

// [CPU FIFOサイズ]
#define CPU_FIFO_SIZE            32 // RP2040, RP2350のCPU FIFOサイズ = 32Bit

// [CPU FIFO数]
#ifdef MCU_RP2040
#define CPU_FIFO_NUM             8 // RP2040 CPU FIFOの数: 8段
#else
#define CPU_FIFO_NUM             4 // RP2350 CPU FIFOの数: 4段
#endif

// --------------------------------------------------------------------------

// CPU間通信フレームコマンド
typedef enum {
    CPU_COM_CMD_NONE      = 0x00, // CPU間通信 コマンドなし
    CPU_COM_CMD_REQ       = 0x70, // CPU間通信 要求コマンド
    CPU_COM_CMD_RES       = 0x71, // CPU間通信 要求応答コマンド
    CPU_COM_CMD_TEST      = 0x72, // CPU間通信 テストコマンド
} e_cpu_com_cmd;

// CPU間通信フレーム構造体
typedef union {
    uint32_t DWORD;
    struct {
        uint32_t DATA:24;   // bit[23:0]:  データ   24it
        uint32_t CMD:7;     // bit[30:24]: コマンド 7bit
        uint32_t CPUx:1;    // bit31:      CPU番号  1bit
    }BIT;
} cpu_com_flame_t;

void cpu_com_tx_flame(cpu_com_flame_t *p_flame);
void cpu_com_rx_flame(cpu_com_flame_t *p_flame);
void cpu_com_init(void);
void cpu_com_main(uint8_t cpu_no);

#endif // EXT_MCU_COM_H