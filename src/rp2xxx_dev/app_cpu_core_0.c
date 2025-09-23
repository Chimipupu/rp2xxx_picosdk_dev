/**
 * @file app_cpu_core_0.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief RP2350のCPU Core0のアプリ
 * @version 0.1
 * @date 2025-06-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#include "app_cpu_core_0.h"
#include "pcb_def.h"
#include "muc_rpxxx_util.h"
#include "drv_neopixel.h"
#include "state_machine.h"

volatile uint32_t g_core_num_core_0 = 0xFF;

/**
 * @brief CPU Core0のアプリメイン関数
 * 
 */
void app_core_0_main(void)
{
    g_core_num_core_0 = get_core_num();

    // ステートマシン初期化
    sm_slave_init();

    while(1)
    {
        // ステートマシーン
        sm_slave_main();

        _WDT_CNT_RST();
    }
}