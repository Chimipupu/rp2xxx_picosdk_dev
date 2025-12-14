/**
 * @file dbd_com_app.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief デバッグモニタのアプリ
 * @version 0.1
 * @date 2025-08-02
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "dbg_com.h"

// C/C++ Std Include
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// SDK Include
#include "pico/version.h"
#include "pico/time.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "muc_rpxxx_util.h"
#include "pcb_def.h"
#include "hw_init.h"
#include "app_main.h"
#include "app_math.h"
#include "muc_rpxxx_util.h"
#include "cpu_com.h"
#include "drv_neopixel.h"

// --------------------------------------------------------------------------
// 期待値: tan(355/226)
#define TAN_355_226_EXPECTED    -7497258.18532

// [タイマー関連定義]
#define TIMER_MAX_SECONDS       3600   // 最大1時間
// タイマーの最大アラーム数
#define TIMER_MAX_ALARMS        PICO_TIME_DEFAULT_ALARM_POOL_MAX_TIMERS
// --------------------------------------------------------------------------
extern neopixel_t s_neopixel;

static uint32_t s_rng_buf[256];
static int get_neopixel_color_from_name(const char* name);
static int parse_hex_color(const char *p_str, uint8_t *p_r, uint8_t *p_g, uint8_t *p_b);
static void cmd_beep(dbg_cmd_args_t *p_args);

static void cmd_cls(dbg_cmd_args_t *p_args);
static void cmd_system(dbg_cmd_args_t *p_args);
static void cmd_mat_test(dbg_cmd_args_t *p_args);
static void cmd_mct_test(dbg_cmd_args_t *p_args);
static void cmd_pi_calc(dbg_cmd_args_t *p_args);
#if defined(MCU_RP2350)
static void cmd_rng(dbg_cmd_args_t *p_args);
static void cmd_sha(dbg_cmd_args_t *p_args);
#endif
static void cmd_rst(dbg_cmd_args_t *p_args);
static void cmd_timer(dbg_cmd_args_t *p_args);
static void cmd_rtc(dbg_cmd_args_t *p_args);
static void cmd_gpio(dbg_cmd_args_t *p_args);
static void cmd_i2c(dbg_cmd_args_t *p_args);
static void cmd_reg(dbg_cmd_args_t *p_args);
static void cmd_neopixel(dbg_cmd_args_t *p_args);
static void cmd_beep(dbg_cmd_args_t *p_args);

// コマンドテーブル
const dbg_cmd_info_t g_cmd_tbl[] = {
//  | 短縮/フルコマンド文字列 | コールバック関数 | 最小引数 | 最大引数 | コマンドの説明分 |
    // [システム機能関連]
    {"sys", "sysinfo"   ,&cmd_system,      0,    0,    "Show System Info"},
    {"rst", "reset"     ,&cmd_rst,         0,    0,    "S/W Reset"},

    // [マイコンのペリフェラル機能関連]
    {"reg", "register"  ,&cmd_reg,         3,    4,    "Register R/W. exp(reg #addr r|w bits #val)"},
    {"i2c", "iic"       ,&cmd_i2c,         2,    2,    "I2C Control. exp(i2c port, command)"},
    {"io",  "gpio"      ,&cmd_gpio,        2,    2,    "GPIO Control. exp(pin pin, value)"},
    {"px",  "neopixel"  ,&cmd_neopixel,    1,    2,    "NeoPixel Control. exp(px command, args)"},
    {"tm",  "timer"     ,&cmd_timer,       0,    1,    "Set timer alarm. exp(tm seconds)"},
    //  RTC ... (RP2040 = H/W RTC, RP2350 = AON Timer)
    {"rt", "rtc"        ,&cmd_rtc,         0,    1,    "RTC. exp(rtc g | rtc s YYYY/MM/DD HH:MM:SS)"},
    {"bp", "beep"       ,&cmd_beep,        1,    2,    "Beep Sound. exp(bp duration(us))"},
#if defined(MCU_RP2350)
    {"rng", "random"    ,&cmd_rng,         0,    1,    "Generate 32bit True Random Number, using H/W TRNG"},
    {"sha", "sha256"    ,&cmd_sha,         0,    1,    "Calc SHA-256 Hash, using H/W Accelerator"},
#endif

    // [マイコンの性能関連]
    {"mat", "mathtest"   ,&cmd_mat_test,   0,    0,    "Math Calc Test"},
    {"mct", "multicore"  ,&cmd_mct_test,   0,    0,    "Multi Core CPU Test"},
};
// コマンドテーブルのコマンド数(const)
const size_t g_tbl_cmd_num = sizeof(g_cmd_tbl) / sizeof(g_cmd_tbl[0]);

// --------------------------------------------------------------------------
static void cmd_system(dbg_cmd_args_t *p_args)
{
    uint8_t i;
    uint32_t sys_clock, usb_clock, adc_clock, ref_clock;
    float cpu_temp;

    printf("\n[System Information]\n");

    // Pico SDK
    pico_sdk_version_print();

#if defined(MCU_RP2350)
    // 時刻表示
    aon_current_time_print();
#endif

    // 基板
    printf("\n[PCB Info]\nPCB Name : %s\n", PCB_NAME);

    // マイコン
    cpu_temp = get_cpu_temp_from_adc();
    printf("MCU : %s\n", MCU_NAME);
#if defined(MCU_RP2040)
    printf("CPU : Arm Cortex-M0+ (DualCore)\n");
#elif defined(MCU_RP2350)
    printf("CPU : Arm Cortex-M33 (DualCore)\n");
    rp2xxx_chip_package_print();    // チップパッケージ
#endif
    rp2xxx_chip_rev_print();        // チップRev
    printf("CPU temp = %.02f℃\n", cpu_temp);

    // レジスタ表示
    rp2xxx_reg_info();

    // ROM/RAM
    printf("\n[Mem Info]\n");
    printf("Flash Size : %d MB\n", MCU_FLASH_SIZE);
    printf("RAM Size : %d KB\n", MCU_RAM_SIZE);
#if defined(PCB_PSRAM_ENABLE)
    printf("PSRAM Size : %d MB\n", MCU_PSRAM_SIZE);
#endif // MCU_PSRAM_SIZE

    // クロック関連
    printf("\n[Clock Info]\n");

#if 0
    uint32_t clk_gp0, clk_gp1, clk_gp2, clk_gp3;
    clk_gp0 = clock_get_hz(clk_gpout0) / 1000000;
    clk_gp1 = clock_get_hz(clk_gpout1) / 1000000;
    clk_gp2 = clock_get_hz(clk_gpout2) / 1000000;
    clk_gp3 = clock_get_hz(clk_gpout3) / 1000000;
    printf("CLK_GPOUT0 : %d MHz\n", clk_gp0);
    printf("CLK_GPOUT1 : %d MHz\n", clk_gp1);
    printf("CLK_GPOUT2 : %d MHz\n", clk_gp2);
    printf("CLK_GPOUT3 : %d MHz\n", clk_gp3);
#endif

    ref_clock = clock_get_hz(clk_ref) / 1000000;
    sys_clock = clock_get_hz(clk_sys) / 1000000;
    usb_clock = clock_get_hz(clk_usb) / 1000000;
    adc_clock = clock_get_hz(clk_adc) / 1000000;
    printf("CLK_REF : %d MHz\n", ref_clock);
    printf("CLK_SYS : %d MHz\n", sys_clock);
    printf("CLK_USB : %d MHz\n", usb_clock);
    printf("CLK_ADC : %d MHz\n", adc_clock);

    // GPIO
    printf("\n[GPIO Info]\n");
#if defined(PCB_WEACT_RP2350A_V10)
    printf("GPIO %d : On Board Green LED Pin\n", PCB_LED_PIN);
    printf("GPIO %d : On Board Blue LED Pin\n", PCB_LED_2_PIN);
#else
    printf("GPIO %d : On Board LED Pin\n", PCB_LED_PIN);
#endif

#if defined(PCB_BTN_PIN)
    printf("GPIO %d : On Board Button Pin\n", PCB_BTN_PIN);
#endif

#ifdef PCB_NEOPIXEL
    printf("GPIO %d : NeoPixel Data Pin\n", PCB_NEOPIXEL_PIN);
    printf("NeoPixel Count : %d\n", NEOPIXEL_LED_CNT);
#endif // PCB_NEOPIXEL

    // DMA
    printf("\n[DMA Info]\n");
    for(i = 0; i < DMA_CH_CNT; i++)
    {
        if(g_dma_ch_info[i].active != 0) {
            printf("DMA CH %d : Active\n", i);
        } else {
            printf("DMA CH %d : Not Use\n", i);
        }
    }

    // I2C
    printf("\n[I2C Info]\n");
    printf("I2C 0 : %d bps, SDA/SCL (GPIO %d/%d)\n",
            I2C_BIT_RATE, I2C_0_SDA, I2C_0_SCL);
    printf("I2C 1 : %d bps, SDA/SCL (GPIO %d/%d)\n",
            I2C_BIT_RATE, I2C_1_SDA, I2C_1_SCL);

    // SPI
    printf("\n[SPI Info]\n");
    printf("SPI 0 : %d bps, CS/SCK/MISO/MOSI(GPIO %d/%d/%d/%d)\n",
            SPI_BIT_RATE, SPI_0_CS, SPI_0_SCK, SPI_0_MISO, SPI_0_MOSI);
    printf("SPI 1 : %d bps, CS/SCK/MISO/MOSI(GPIO %d/%d/%d/%d)\n",
            SPI_BIT_RATE, SPI_1_CS, SPI_1_SCK, SPI_1_MISO, SPI_1_MOSI);
    // UART
    printf("\n[UART Info]\n");
    printf("UART 0 : %d bps 8N1, TX/RX (GPIO %d/%d)\n",
            UART_BAUD_RATE, UART_0_TX, UART_0_RX);
    printf("UART 1 : %d bps 8N1, TX/RX (GPIO %d/%d)\n",
            UART_BAUD_RATE, UART_1_TX, UART_1_RX);
}

static void cmd_mat_test(dbg_cmd_args_t *p_args)
{
    // 数学関連テスト
    app_math_math_test();

    // 四則演算テスト(inr,float,double)
    printf("\nInteger Arithmetic Test: @%d\n", TEST_LOOP_CNT);
    proc_exec_time(int_add_test, "int_add_test");
    proc_exec_time(int_sub_test, "int_sub_test");
    proc_exec_time(int_mul_test, "int_mul_test");
    proc_exec_time(int_div_test, "int_div_test");

    printf("\nFloat Arithmetic Tests: @%d\n", TEST_LOOP_CNT);
    proc_exec_time(float_add_test, "float_add_test");
    proc_exec_time(float_sub_test, "float_sub_test");
    proc_exec_time(float_mul_test, "float_mul_test");
    proc_exec_time(float_div_test, "float_div_test");

    printf("\nDouble Arithmetic Tests: @%d\n", TEST_LOOP_CNT);
    proc_exec_time(double_add_test, "double_add_test");
    proc_exec_time(double_sub_test, "double_sub_test");
    proc_exec_time(double_mul_test, "double_mul_test");
    proc_exec_time(double_div_test, "double_div_test");
}

static void  cmd_mct_test(dbg_cmd_args_t *p_args)
{
    cpu_com_flame_t flame;

    flame.BIT.CMD = CPU_COM_CMD_TEST;
    flame.BIT.CPUx = get_core_num();
    flame.BIT.DATA = CPU_COM_DATA_TEST;
    cpu_com_tx_flame(&flame);
}

static void cmd_pi_calc(dbg_cmd_args_t *p_args)
{
    int32_t iterations = 3;
    volatile double pi;

    if (p_args->argc > 1) {
        iterations = atoi(p_args->p_argv[1]);
        if (iterations <= 0) {
            printf("Error: Invalid iteration count. Must be positive.\n");
            return;
        }
    }

    printf("\nCalculating Pi using Gauss-Legendre algorithm (%d iterations):\n", iterations);
    for (uint32_t i = 1; i <= iterations; i++)
    {
        volatile uint32_t start_time = time_us_32();
        pi = app_math_pi_calc(i);
        volatile uint32_t end_time = time_us_32();
        printf("Iteration %d: π ≈ %.15f (proc time: %u us)\n", i, pi, end_time - start_time);
    }
}

#if defined(MCU_RP2350)
static void cmd_sha(dbg_cmd_args_t *p_args)
{
    uint8_t padding_buf[64 * 2];
    uint8_t hash_buf[64 * 2];
    size_t padding_len;

    if (p_args->argc < 2 || p_args->argc > 2) {
        printf("Usage: sha <data>\n");
        return;
    }

    memset(hash_buf, 0, sizeof(hash_buf));
    memset(padding_buf, 0, sizeof(padding_buf));

#if 1
    char *msg;
    msg = p_args->p_argv[1];
#else
    // (DEBUG)
    // const char msg[] = "RP2350 H/W SHA-256 TEST";    // SHA256期待値「40AEDCA712E82FD6128B6A9AFCA1350ABA27AC1259E92AE3BADE0C0C4FE892E2」
    const char msg[] = "ABC";                        // SHA256期待値「B5D4045C3F466FA91FE2CC6ABE79232A1A57CDF104F7A26E716E0A1E2789DF78」
#endif

    printf("\nSHA-256 Hash Calc(H/W)\n");
    printf("\nCalc str : %s\n", msg);
    // SHA-256のパディング処理
    sha256_padding((const uint8_t *)msg, strlen(msg), padding_buf, &padding_len);
    show_mem_dump((uint32_t)padding_buf, 64);

    // SHA-256のハッシュ値を計算
    hardware_calc_sha256((const uint8_t *)padding_buf, padding_len, hash_buf);
    printf("\nSHA-256 Hash : ");
    for (size_t i = 0; i < 32; ++i)
    {
        printf("%02X", hash_buf[i]);
    }
    printf("\n");
    show_mem_dump((uint32_t)hash_buf, 64);
}

static void cmd_rng(dbg_cmd_args_t *p_args)
{
    uint8_t i;
    int count;

    if (p_args->argc < 2 || p_args->argc > 2) {
        printf("Usage: rnd <count>\n");
        return;
    }

    count = atoi(p_args->p_argv[1]);
    if ((count < 0) || (count > 255)) {
        printf("Error: Invalid count. Must be [count ...  0 ~ 255]\n");
        return;
    }

    // TRNGで真性乱数を生成
    memset(&s_rng_buf[0], 0, sizeof(s_rng_buf));
    printf("\nGenerate 32bit Random Num = %d\n", count);
    trang_gen_rand_num_u32(&s_rng_buf[0], count);

    for (i = 0; i < count; i++)
    {
        printf("[DEBUG] 32bit RNG[%d] ... HEX : 0x%08X, DEC : %lu\n", i, s_rng_buf[i], s_rng_buf[i]);
    }

    printf("\n");
}
#endif

static void cmd_rst(dbg_cmd_args_t *p_args)
{
    printf("Resetting system...\n");
    watchdog_reboot(0, 0, 0);   // WDTで即時リセット
}

/**
 * @brief タイマーコマンド関数
 * 
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_timer(dbg_cmd_args_t *p_args)
{
    // TODO:
}

/**
 * @brief RTCコマンド（リソース ... RP2040 = H/W RTC, RP2350 = AON Timer）
 * 
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_rtc(dbg_cmd_args_t *p_args)
{
#if defined(MCU_RP2350)
    // 引数が2つ以上
    if (p_args->argc > 1) {
        // "g"なら現在時刻表示
        if (strcmp(p_args->p_argv[1], "g") == 0) {
            aon_current_time_print();
        }
        // "s"なら時刻設定
        else if (strcmp(p_args->p_argv[1], "s") == 0) {
            if (p_args->argc < 4) {
                printf("Usage: rtc s YYYY/MM/DD HH:MM:SS\n");
                printf("Example: rtc s 2025/08/02 15:30:00\n");
                return;
            }

            // 日付と時刻を結合して一つの文字列にする
            char time_str[32];
            snprintf(time_str, sizeof(time_str), "%s %s", p_args->p_argv[2], p_args->p_argv[3]);

            if (aon_set_time_from_string(time_str)) {
                printf("AON Timer set to: %s\n", time_str);
            } else {
                printf("Failed to set AON Timer (arg = %s). Format: YYYY/MM/DD HH:MM:SS\n", time_str);
            }
        } else {
            printf("Usage: rtc g | rtc s YYYY/MM/DD HH:MM:SS\n");
            printf("  g - Get current time\n");
            printf("  s - Set time (format: YYYY/MM/DD HH:MM:SS)\n");
            printf("Example: rtc s 2025/08/02 15:30:00\n");
        }
    } else {
        // 引数なし = 現在時刻表示
        aon_current_time_print();
    }
#endif // MCU_RP2350
}

/**
 * @brief GPIO制御コマンド関数
 * 
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_gpio(dbg_cmd_args_t *p_args)
{
    if (p_args->argc != 3) {
        printf("Error: Invalid number of arguments. Usage: gpio <pin> <value>\n\n");
        return;
    }

    // コマンドからGPIOのピン番号とピンの値を取得
    uint8_t pin = atoi(p_args->p_argv[1]);
    uint8_t value = atoi(p_args->p_argv[2]);

    // GPIOのピン番号チェック
    if (pin < 0 || pin > GPIO_MAX_PIN_NUM) {
        printf("Error: Invalid GPIO pin number. Must be between 0 and %d.\n\n", GPIO_MAX_PIN_NUM);
        return;
    }

    // Low/High以外は受け付けない
    if (value != 0 && value != 1) {
        printf("Error: Invalid GPIO value. Must be 0 or 1.\n\n");
        return;
    }

    // GPIOの初期化と設定
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);

#if 0
    // GPIO操作の処理時間を計測
    volatile uint32_t start_time = time_us_32();
    gpio_put(pin, value);
    volatile uint32_t end_time = time_us_32();
    printf("GPIO %d set to %d (proc time: %u us)\n\n", pin, value, end_time - start_time);
#endif
}

/**
 * @brief I2Cスキャンコマンド関数
 * 
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_i2c(dbg_cmd_args_t *p_args)
{
    if (p_args->argc != 3) {
        printf("Error: Invalid number of arguments. Usage: i2c <port> <command>\n");
        printf("Commands:\n");
        printf("  s - Scan I2C bus for devices\n");
        return;
    }

    // I2Cポート番号を取得
    uint8_t port = atoi(p_args->p_argv[1]);
    if (port != 0 && port != 1) {
        printf("Error: Only I2C ports 0 and 1 are supported.\n");
        return;
    }

    const char* cmd = p_args->p_argv[2];

    // I2Cのポートスキャン
    if (strcmp(cmd, "s") == 0) {
        i2c_slave_scan(port);
    } else {
        printf("Error: Unknown I2C command '%s'\n", cmd);
    }
}

/**
 * @brief レジスタR/Wコマンド関数
 * 
 * @param p_args コマンド引数の構造体ポインタ
 */
static void cmd_reg(dbg_cmd_args_t *p_args)
{
    uint32_t wval = 0;
    uint32_t val = 0;
    uint32_t addr = 0;

    if (p_args->argc != 4 && p_args->argc != 5) {
        printf("Error: Usage: reg #ADDR r|w BITS [#VAL]\n");
        printf("  e.g. reg #F000FF00 r 8\n");
        printf("  e.g. reg #F000FF00 w 32 #FFDC008F\n");
        return;
    }

    if (sscanf(p_args->p_argv[1], "#%x", &addr) != 1) {
        printf("Error: Invalid address format. Use #HEX (e.g. #F000FF00)\n");
        return;
    }
    char rw = p_args->p_argv[2][0];
    int bits = atoi(p_args->p_argv[3]);
    if (!(bits == 8 || bits == 16 || bits == 32)) {
        printf("Error: Bit width must be 8, 16, or 32\n");
        return;
    }
    if (rw == 'r') { // 読み取り
        if (p_args->argc != 4) {
            printf("Error: Read usage: reg #ADDR r BITS\n");
            return;
        }
        // app_main.hのマクロを使用
        if (bits == 8) val = REG_READ_BYTE(0, addr);
        else if (bits == 16) val = REG_READ_WORD(0, addr);
        else if (bits == 32) val = REG_READ_DWORD(0, addr);
        printf("[REG] Read %dbit @ 0x%08X = 0x%08X\n", bits, addr, val);
    } else if (rw == 'w') { // 書き込み
        sscanf(p_args->p_argv[4], "#%x", &wval);
        if (bits == 8) {
            REG_WRITE_BYTE(0, addr, (uint8_t)wval);
        } else if (bits == 16) {
            REG_WRITE_WORD(0, addr, (uint16_t)wval);
        } else if (bits == 32) {
            REG_WRITE_DWORD(0, addr, (uint32_t)wval);
        }
            printf("[REG] Write %dbit @ 0x%08X = 0x%08X\n", bits, addr, wval);
    } else {
            printf("Error: 2nd arg must be 'r' or 'w'\n");
    }
}

static int get_neopixel_color_from_name(const char* name)
{
    if (strcasecmp(name, "black") == 0) return NEOPIXEL_COLOR_BLACK;
    if (strcasecmp(name, "red") == 0) return NEOPIXEL_COLOR_RED;
    if (strcasecmp(name, "green") == 0) return NEOPIXEL_COLOR_GREEN;
    if (strcasecmp(name, "blue") == 0) return NEOPIXEL_COLOR_BLUE;
    if (strcasecmp(name, "yellow") == 0) return NEOPIXEL_COLOR_YELLOW;
    if (strcasecmp(name, "cyan") == 0) return NEOPIXEL_COLOR_CYAN;
    if (strcasecmp(name, "magenta") == 0) return NEOPIXEL_COLOR_MAGENTA;
    if (strcasecmp(name, "orange") == 0) return NEOPIXEL_COLOR_ORANGE;
    if (strcasecmp(name, "purple") == 0) return NEOPIXEL_COLOR_PURPLE;
    if (strcasecmp(name, "pink") == 0) return NEOPIXEL_COLOR_PINK;
    if (strcasecmp(name, "white") == 0) return NEOPIXEL_COLOR_WHITE;
    return -1;
}

static int parse_hex_color(const char *p_str, uint8_t *p_r, uint8_t *p_g, uint8_t *p_b)
{
    int ret;
    unsigned int rv, gv, bv = 0;
    char *p_tmp;

    p_tmp = (char *)p_str;

    if ((p_str == NULL) || (strlen(p_str) != 7) || (p_str[0] != '#')) {
        return -1;
    }

    sscanf(p_tmp, "#%02X%02X%02X", &rv, &gv, &bv);
    *p_r = (uint8_t)rv;
    *p_g = (uint8_t)gv;
    *p_b = (uint8_t)bv;

    return 0;
}

/**
 * @brief NeoPixel制御コマンド関数
 * 
 * @param p_args 
 */
static void cmd_neopixel(dbg_cmd_args_t *p_args)
{
    uint8_t r, g, b = 0;
    int idx = 0;
    uint8_t all_set_flag = 0;
    int color_enum = 0;

    char *p_mode_str = p_args->p_argv[1];
    // 第二引数がcls
    if(strcasecmp(p_mode_str, "cls") == 0) {
        drv_neopixel_init(&s_neopixel);
        printf("All NeoPixel Cleared!\n");
        return;
    }

    if(strcasecmp(p_mode_str, "fade") == 0) {
        // Core 0にLEDフェードを実行要求
        set_multicore_fifo(CPU_COM_DATA_PROC_RGB);
        printf("All NeoPixel Color Fade! at Core 0\n");
        return;
    }

    // コマンド引数チェック
    if (p_args->argc < 3) {
        printf("Usage: neopixel <index|mode> <color|#RRGGBB>\n");
        return;
    }

    // 第二引数がall
    if(strcasecmp(p_mode_str, "all") == 0) {
        all_set_flag = 1;
    } else {
        idx = atoi(p_args->p_argv[1]);
        // 指定indexチェック
        if ((idx < 1) || idx > s_neopixel.led_cnt) {
            printf("Error: indexは1～%d\n", s_neopixel.led_cnt);
            return;
        }
    }

    // 第三引数が文字
    uint8_t led_idx = (uint8_t)idx - 1;
    char *p_color_str = p_args->p_argv[2];
    color_enum = get_neopixel_color_from_name(p_color_str);
    if (color_enum >= 0) {
        if (all_set_flag != 0) {
            // 全てのNeoPixelに同じ色を設定
            for (uint8_t i = 0; i < s_neopixel.led_cnt; i++)
            {
                drv_neopixel_get_pixel_color(&s_neopixel, i, color_enum);
            }
            drv_neopixel_show(&s_neopixel);
            printf("All NeoPixels set to %s\n", p_color_str);
            return;
        } else {
            // 指定されたNeoPixelに色を設定
            drv_neopixel_set_pixel_color(&s_neopixel, led_idx, color_enum);
            printf("NeoPixel[%d] = %s\n", led_idx, p_color_str);
            return;
        }
    }

    // 第三引数が#RRGGBB形式
    if (parse_hex_color(p_color_str, &r, &g, &b) == 0) {
        if (all_set_flag != 0) {
            // 全てのNeoPixelに同じ色を設定
            drv_neopixel_set_all_led_color(&s_neopixel, r, g, b);
            printf("All NeoPixels set to #%02X%02X%02X\n", r, g, b);
            return;
        } else {
            drv_neopixel_set_pixel_rgb(&s_neopixel, led_idx, r, g, b);
            printf("NeoPixel[%d] = #%02X%02X%02X\n", led_idx, r, g, b);
            return;
        }
    }
}

/**
 * @brief タイマー割り込みハンドラ
 * 
 * @param id アラームID
 * @param p_user_data ユーザーデータポインタ
 * @return int64_t 次のアラームまでの時間(マイクロ秒)
 */
int64_t tim_irq_handler(alarm_id_t id, void *p_user_data)
{
    // GPIOトグル
    gpio_put(PCB_LED_PIN, !gpio_get(PCB_LED_PIN));   // 基板LEDピン
    gpio_put(PCB_BEEP_PIN, !gpio_get(PCB_BEEP_PIN)); // 基板BEEP音用ピン
    return 0;
}

/**
 * @brief Beep音初期化関数
 * @note Beep音 ... タイマー割り込みでGPIOをトグルして音を鳴らす
 * @param duration_us 周期(us)
 */
static void hw_init_beep_sound(uint32_t duration_us)
{
    static bool is_beep_configured = false;

    if(is_beep_configured == false) {
        // BEEP音を鳴らすGPIOの初期化
        gpio_init(PCB_BEEP_PIN);
        gpio_set_dir(PCB_BEEP_PIN, GPIO_OUT);

#if 1
        // (DEBUG) デバッグ用に基板のLEDを使う
        gpio_init(PCB_LED_PIN);
        gpio_set_dir(PCB_LED_PIN, GPIO_OUT);
#endif
        is_beep_configured = true;
    } else {
       // 引数の周期(us)からタイマー割り込みを設定
        add_alarm_in_us(duration_us, tim_irq_handler, NULL, false);
    }
}

/**
 * @brief Beep音コマンド
 * @param p_args 周期(us)
 */
static void cmd_beep(dbg_cmd_args_t *p_args)
{
    uint32_t duration_us;

    if (p_args->argc == 2) {
        duration_us = atoi(p_args->p_argv[1]);
        // 人の可聴域の20Hz~20KHzではない場合はエラー
        if (duration_us < 50 || duration_us > 50000) {
            printf("[ERROR] Beep duration must be between 50000us (20Hz) and 50us (20KHz) !!!\n");
            return;
        }
    }

    printf("[DEBUG] Beep Sound: T=%d us\n", duration_us);
    hw_init_beep_sound(duration_us);
}