/**
 * @file ext_mcu_com.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief マイコン間通信
 * @version 0.1
 * @date 2025-09-23
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

#define DEBUG_EXT_MCU_COM_USE

// マイコン間通信フレームコマンド
typedef enum {
    EMC_CMD_NONE         = 0x00, // コマンドなし
#ifdef DEBUG_EXT_MCU_COM_USE
    EMC_CMD_TEST_REQ     = 0xE0, // テスト要求コマンド
    EMC_CMD_TEST_RES     = 0xE2, // テスト要求応答コマンド
#endif // DEBUG_EXT_MCU_COM_USE
} e_emc_cmd;

// マイコン間通信フレーム構造体
typedef union {
    uint32_t DWORD;
    struct {
        uint32_t DATA:16;  // データ         ... bit[15:0]
        uint32_t LEN:8;    // データ長(Word) ... bit[23:16]
        uint32_t CMD:8;    // コマンド       ... bit[31:24]
    }BIT;
} EXT_COM_FLAME_T;

void emc_tx_flame(EXT_COM_FLAME_T *p_flame);
void emc_rx_flame(EXT_COM_FLAME_T *p_flame);
void emc_parse_sm_flame(EXT_COM_FLAME_T *p_flame);
void emc_init(void);
void emc_main(void);

#ifdef DEBUG_EXT_MCU_COM_USE
void emc_test(void);
#endif // DEBUG_EXT_MCU_COM_USE

#endif // EXT_MCU_COM_H