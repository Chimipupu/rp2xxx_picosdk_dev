/**
 * @file state_machine.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief ステートマシーン (RP2350 マルチコア制御用)
 * @version 0.1
 * @date 2025-09-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// #include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SM_FLAME_DATA_BUF_SIZE              0xFF

#define CORE_1_WUP_RESULT_DATA              0x1234
#define MULTI_CORE_TEST_DATA                0xABCD
#define PROC_NEOPIXEL_FADE                  0xAAAA

typedef enum {
    // SM_ERROR        = 0xFF, // エラー状態
    SM_NONE         = 0x00, // 状態なし
    SM_IDLE         = 0x10, // アイドル状態
    // SM_BUSY         = 0x20, // ビジー状態
    SM_PROC_END     = 0x30, // 処理完了状態
    // SM_MATH_CALC    = 0x40, // 円周率計算状態
    SM_TEST_PROC    = 0xE0,
} e_state_machine;

// ステートマシンフレームコマンド
typedef enum {
    SM_CMD_NONE         = 0x00, // コマンドなし
    SM_CMD_TEST_REQ     = 0xE0, // テスト要求コマンド
    SM_CMD_TEST_RES     = 0xE2, // テスト要求応答コマンド
} e_sm_cmd;

// ステートマシンフレーム構造体
typedef union {
    uint32_t DWORD;
    struct {
        uint32_t DATA:16;  // データ         ... bit[15:0]
        uint32_t LEN:8;    // データ長(Word) ... bit[23:16]
        uint32_t CMD:8;    // コマンド       ... bit[31:24]
    }BIT;
} SM_FLAME_T;

void sm_flame_tx(SM_FLAME_T *p_flame);
void sm_flame_rx(SM_FLAME_T *p_flame);
void sm_master_init(void);
void sm_master_main(void);
void sm_slave_init(void);
void sm_slave_main(void);

#endif // STATE_MACHINE_H