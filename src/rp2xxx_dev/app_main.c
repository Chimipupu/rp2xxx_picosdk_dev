/**
 * @file app_main.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2025-11-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#include "app_main.h"
#include "pcb_def.h"
#include "muc_rpxxx_util.h"
#include "drv_neopixel.h"
#include "cpu_com.h"

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#ifdef DEBUG_DBG_COM_USE
#include "dbg_com.h"
extern const dbg_cmd_info_t g_cmd_tbl[];
extern const size_t g_tbl_cmd_num;
dbg_com_config_t g_dbgc_config;
#endif // DEBUG_DBG_COM_USE

#if 1
#include "drv_neopixel.h"
neopixel_t s_neopixel;
static rgb_color_t s_rgb_buf[NEOPIXEL_LED_CNT] = {0};
#endif
// --------------------------------------------------------------------------

volatile uint32_t g_core_num_core_0 = 0xFF;
volatile uint32_t g_core_num_core_1 = 0xFF;

static void cpu_core_0_app_main(void);
static void cpu_core_1_app_main(void);
// --------------------------------------------------------------------------
// [APP]

/**
 * @brief メモリダンプ(16進HEX & Ascii)
 * 
 * @param dump_addr ダンプするメモリの32bitアドレス
 * @param dump_size ダンプするサイズ(Byte)
 */
void show_mem_dump(uint32_t dump_addr, uint32_t dump_size)
{
    printf("\n[Memory Dump '(addr:0x%04X)]\n", dump_addr);

    // ヘッダー行を表示
    printf("Address  ");
    for (int i = 0; i < 16; i++)
    {
        printf("%02X ", i);
    }
    printf("| ASCII\n");
    printf("-------- ");
    for (int i = 0; i < 16; i++)
    {
        printf("---");
    }
    printf("| ------\n");

    // 16バイトずつダンプ
    for (uint32_t offset = 0; offset < dump_size; offset += 16)
    {
        printf("%08X: ", dump_addr + offset);

        // 16バイト分のデータを表示
        for (int i = 0; i < 16; i++)
        {
            if (offset + i < dump_size) {
                uint8_t data = *((volatile uint8_t*)(dump_addr + offset + i));
                printf("%02X ", data);
            } else {
                printf("   ");
            }
        }

        // ASCII表示
        printf("| ");
        for (int i = 0; i < 16; i++)
        {
            if (offset + i < dump_size) {
                uint8_t data = *((volatile uint8_t*)(dump_addr + offset + i));
                // 表示可能なASCII文字のみ表示
                printf("%c", (data >= 32 && data <= 126) ? data : '.');
            } else {
                printf(" ");  // データがない場合は空白を表示
            }
        }
        printf("\n");
    }
}

/**
 * @brief i2Cスレーブデバイスのスキャン関数
 * 
 * @param port I2Cポート番号 (0 or 1)
 */
void i2c_slave_scan(uint8_t port)
{
    int32_t ret = 0xFF;
    uint8_t addr, dummy = 0x00;
    uint8_t slave_count = 0;
    uint8_t slave_addr_buf[128] = {0};

    memset(&slave_addr_buf[0], 0x00, sizeof(slave_addr_buf));
    i2c_inst_t *i2c_port = (port == 0) ? I2C_0_PORT : I2C_1_PORT;

    // 7bitのI2Cスレーブアドレス(0x00～0x7F)をスキャン
    printf("Scanning I2C%d bus...\n", port);
    printf("       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    for (addr = 0; addr <= 0x7F; addr++)
    {
        if ((addr & 0x0F) == 0) {
            printf("0x%02X: ", addr & 0xF0);
        }

        ret = i2c_write_blocking(i2c_port, addr, &dummy, 1, false);
        if (ret >= 0) {
            printf(" * ");
            slave_addr_buf[slave_count] = addr;
            slave_count++;
        } else {
            printf(" - ");
        }

        if ((addr & 0x0F) == 0x0F) {
            printf("\n");
        }
    }

    printf("\nI2C Scan complete! (Slave:%d", slave_count);
    for (uint8_t i = 0; i < slave_count; i++) {
        printf(", 0x%02X", slave_addr_buf[i]);
    }
    printf(")\n");
}

/**
 * @brief 関数の実行時間を計測する
 * 
 * @param func 計測対象の関数ポインタ
 * @param func_name 関数名（表示用）
 * @param ... 関数に渡す引数（可変長）
 */
void proc_exec_time(void (*func)(void), const char* func_name, ...)
{
    volatile uint32_t start_time = time_us_32();
    func();
    volatile uint32_t end_time = time_us_32();

    printf("proc time %s: %u us\n", func_name, end_time - start_time);
}

/**
 * @brief CPU Core0のメイン関数
 * 
 */
void core_0_main(void)
{
    cpu_core_0_app_main();
}

/**
 * @brief CPU Core1のメイン関数
 * 
 */
void core_1_main(void)
{
    cpu_core_1_app_main();
}

/**
 * @brief CPU Core0のアプリメイン関数
 * 
 */
static void cpu_core_0_app_main(void)
{
    uint32_t cpu_fifo_data = 0;
    g_core_num_core_0 = get_core_num();

    while(1)
    {
        // CPU間通信 メイン
        cpu_com_main(g_core_num_core_0);
        _WDT_CNT_RST();
    }
}

/**
 * @brief CPU Core1のアプリメイン関数
 * 
 */
static void cpu_core_1_app_main(void)
{
    g_core_num_core_1 = get_core_num();

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
    g_dbgc_config.p_cmd_tbl = &g_cmd_tbl[0];
    g_dbgc_config.total_cmd = g_tbl_cmd_num;
    dbg_com_init(&g_dbgc_config);
#endif // DEBUG_DBG_COM_USE

    while(1)
    {
        // CPU間通信 メイン
        cpu_com_main(g_core_num_core_1);

#ifdef DEBUG_DBG_COM_USE
        // デバッグモニタ メイン
        dbg_com_main();
#endif // DEBUG_DBG_COM_USE

        _WDT_CNT_RST();
    }
}