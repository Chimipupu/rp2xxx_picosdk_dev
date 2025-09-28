/**
 * @file app_cpu_core_1.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief RP2350のCPU Core1のアプリ
 * @version 0.1
 * @date 2025-06-13
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#include "app_cpu_core_1.h"
#include "app_main.h"
#include "muc_rpxxx_util.h"
#include "state_machine.h"
#include "ext_mcu_com.h"

#define DEBUG_DBG_COM_USE

#ifdef DEBUG_DBG_COM_USE
#include "dbg_com.h"
#endif // DEBUG_DBG_COM_USE

#if 1
#include "drv_neopixel.h"
neopixel_t s_neopixel;
static rgb_color_t s_rgb_buf[NEOPIXEL_LED_CNT] = {0};
#endif


volatile uint32_t g_core_num_core_1 = 0xFF;

/**
 * @brief CPU Core1のアプリメイン関数
 * 
 */
void app_core_1_main(void)
{
    g_core_num_core_1 = get_core_num();

    // 外部マイコン間通信 初期化
    emc_init();

#if 1
    // NeoPixel初期化
    s_neopixel.led_cnt = NEOPIXEL_LED_CNT;
    s_neopixel.data_pin = PCB_NEOPIXEL_PIN;
    memset(s_rgb_buf, 0, sizeof(s_rgb_buf));
    s_neopixel.p_pixel_grb_buf = &s_rgb_buf[0];
    drv_neopixel_init(&s_neopixel);
#endif

    // printf("MCU:\tRP2350\n");
    // printf("System Clock:\t%d MHz\n", clock_get_hz(clk_sys) / 1000000);
    // printf("USB Clock:\t%d MHz\n", clock_get_hz(clk_usb) / 1000000);

#ifdef DEBUG_DBG_COM_USE
    // デバッグモニタ初期化
    dbg_com_init();
#endif // DEBUG_DBG_COM_USE

    while(1)
    {
        // 外部マイコン間通信 メイン
        emc_main();

#ifdef DEBUG_DBG_COM_USE
        // デバッグモニタ メイン
        dbg_com_main();
#endif // DEBUG_DBG_COM_USE

        _WDT_CNT_RST();
    }
}