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

void sm_slave_init(void);
void sm_slave_main(void);

#endif // STATE_MACHINE_H