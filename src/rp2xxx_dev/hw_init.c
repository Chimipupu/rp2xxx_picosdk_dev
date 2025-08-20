/**
 * @file hw_init.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief H/W初期化
 * @version 0.1
 * @date 2025-07-21
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "hw_init.h"

extern volatile uint32_t g_core_num_core_0;
extern volatile uint32_t g_core_num_core_1;

static uint32_t s_core_num = 0;

#if defined(MCU_RP2350)
static uint8_t s_chip_package_flg = CHIP_PACKAGE_QFN60_RP2350A;

static void hw_interp_init(void);
#endif

static void hw_clock_init(void);
static void hw_gpio_init(void);
static void hw_pio_init(void);
static void hw_pwm_init(void);
static void hw_uart_init(void);
static void hw_i2c_init(void);
static void hw_spi_init(void);
static void hw_timer_init(void);
static void hw_wdt_init(void);
static void hw_adc_init(void);

dma_info_t g_dma_ch_info[DMA_CH_CNT];
static int s_dma_ch = 0;
static bool s_is_dma_irq_0_use = false;
static bool s_is_dma_irq_1_use = false;
static bool s_dma_done_flg = false;
static void hw_dma_config(dma_info_t dma_info);
static void hw_dma_init(void);\

#if defined(PCB_PICO2W)
#include "pico/cyw43_arch.h"
static uint8_t s_led_val = 0;

void cyw43_led_tgl(void)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, s_led_val);
    s_led_val = !s_led_val;
}
#endif

const char s_test_str[] = "DMA Test (from DMA CH1)";
char s_dma_dst_buf[count_of(s_test_str)];

const char s_dma_test_src_buf[] = "DMA Test (from DMA CH1)";
char s_dma_test_dst_buf[count_of(s_dma_test_src_buf)];

#ifdef RPI_PIO_USE
#include "blink.pio.h"
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}
#endif

/**
 * @brief DMA転送完了割り込みハンドラ(IRQ0)
 * 
 */
void dma_irq_0_handler()
{
    // 割り込みフラグをクリア(IRQ0)
    dma_hw->ints0 = 1u << s_dma_ch;

    // メインにDMAの転送を通知
    s_dma_done_flg = true;
}

/**
 * @brief DMA転送完了割り込みハンドラ(IRQ1)
 * 
 */
void dma_irq_1_handler()
{
    // 割り込みフラグをクリア(IRQ1)
    dma_hw->ints1 = 1u << s_dma_ch;

    // メインにDMAの転送を通知
    s_dma_done_flg = true;
}

#if defined(PCB_BTN_PIN)
/**
 * @brief 基板ボタン外部割り込みハンドラ
 * 
 * @param gpio 
 * @param event_mask 
 */
void btn_ex_irq_handler(uint gpio, uint32_t event_mask)
{
    // 割り込みマスク
    gpio_set_irq_enabled(PCB_BTN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);

    // チャタリング除去(@30ms待機)
    sleep_ms(30);

    if (gpio_get(PCB_BTN_PIN) != PORT_OFF) {
        printf("[Core%d] Button OFF!\n", s_core_num);
    } else {
        printf("[Core%d] Button ON!\n", s_core_num);
    }

    // 割り込み有効
    gpio_set_irq_enabled(PCB_BTN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}
#endif

#ifdef TIMER_ALARM_IRQ_ENABLE
/**
 * @brief タイマー割り込みハンドラ(2000ms)
 * 
 * @param id 
 * @param p_user_data 
 * @return int64_t 
 */
int64_t alarm_callback(alarm_id_t id, void *p_user_data)
{
    _NOP;

    return 0;
}
#endif // TIMER_ALARM_IRQ_ENABLE

static void hw_clock_init(void)
{
    _NOP;
}

static void hw_gpio_init(void)
{
    // [基板LED]
    gpio_set_function(PCB_LED_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(PCB_LED_PIN, GPIO_OUT);
    gpio_put(PCB_LED_PIN, PORT_OFF);

#if defined(PCB_WEACT_RP2350A_V10)
    // [基板LED(2個目)]
    gpio_set_function(PCB_LED_2_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(PCB_LED_2_PIN, GPIO_OUT);
    gpio_put(PCB_LED_2_PIN, PORT_OFF);
#endif

    // [基板ボタン]
#if defined(PCB_BTN_PIN)
    gpio_init(PCB_BTN_PIN);
    gpio_set_dir(PCB_BTN_PIN, GPIO_IN);

    // 外部割り込み
    gpio_set_irq_enabled_with_callback(PCB_BTN_PIN,   // GPIOピン
                                        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, // 立ち上がり or 立ち下がりエッジでIRQ
                                        true,              // 割り込み 有効 / 無効
                                        &btn_ex_irq_handler // 割り込みハンドラ
                                        );
#endif
}

static void hw_pio_init(void)
{
#ifdef RPI_PIO_USE
    // PIO初期化
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    printf("Loaded program at %d\n", offset);
#endif //RPI_PIO_USE

#if defined(PCB_PICO2) || defined(PCB_WEACT_RP2350B)
    #ifdef RPI_PIO_USE
        // マイコンのPIOが並列でLチカ
        blink_pin_forever(pio, 0, offset, PCB_LED_PIN, 5);
    #endif // RPI_PIO_USE
#elif defined(PCB_PICO2W)
    // RFモジュールでLED制御
    cyw43_arch_init();
#endif
}

#if defined(MCU_RP2350)
/**
 * @brief // 補間器(インターポレータ)初期化
 * 
 */
static void hw_interp_init(void)
{
    interp_config cfg = interp_default_config();
    interp_config_set_cross_result(&cfg, true);
    // ACCUM0 gets lane 1 result:
    interp_set_config(interp0, 0, &cfg);
    // ACCUM1 gets lane 0 result:
    interp_set_config(interp1, 0, &cfg);
}
#endif

static void hw_pwm_init(void)
{
    _NOP;
}

static void hw_i2c_init(void)
{
    // I2C0初期化
    i2c_init(I2C_0_PORT, I2C_BIT_RATE);
    gpio_set_function(I2C_0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_0_SDA);
    gpio_pull_up(I2C_0_SCL);

    // I2C1初期化
    i2c_init(I2C_1_PORT, I2C_BIT_RATE);
    gpio_set_function(I2C_1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_1_SDA);
    gpio_pull_up(I2C_1_SCL);
}

static void hw_spi_init(void)
{
    // SPI0初期化
    spi_init(SPI_0_PORT, SPI_BIT_RATE);
    gpio_set_function(SPI_0_CS,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_0_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_0_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_0_MOSI, GPIO_FUNC_SPI);
    gpio_set_dir(SPI_0_CS, GPIO_OUT);
    gpio_put(SPI_0_CS, 1);

    // SPI1初期化
    spi_init(SPI_1_PORT, SPI_BIT_RATE);
    gpio_set_function(SPI_1_CS,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_1_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_1_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_1_MOSI, GPIO_FUNC_SPI);
    gpio_set_dir(SPI_1_CS, GPIO_OUT);
    gpio_put(SPI_1_CS, 1);
}

#if defined(MCU_RP2350)
// AONタイマー割り込みハンドラ
void aon_alarm_handler()
{
    // TODO :
    // printf("AON timer Alarm\n");
}
#endif // MCU_RP2350

static void hw_timer_init(void)
{
#ifdef TIMER_ALARM_IRQ_ENABLE
    // タイマー割り込み @2000ms
    add_alarm_in_ms(2000, alarm_callback, NULL, false);
#endif // TIMER_ALARM_IRQ_ENABLE

#if defined(MCU_RP2350)
    // 64bit AONタイマー スタート
    struct timespec now;
    uint8_t ret = 0x00;

    // AONタイマー起動
    aon_timer_start(&now);
    aon_set_time_from_string("2025/01/01 00:00:00");

    if (!aon_timer_get_time(&now)) {
        printf("Failed to get AON timer time\n");
        ret = 0xFF;
    }

#if 0
    // アラーム設定
    if (ret != 0xFF) {
        struct timespec alarm_time = now;
        alarm_time.tv_sec += 10;
        aon_timer_enable_alarm(&alarm_time, aon_alarm_handler, true);
    }
#endif
#endif // MCU_RP2350
}

static void hw_uart_init(void)
{
    // UART0初期化(115200bps 8N1)
    uart_init(UART_0_PORT, UART_BAUD_RATE);
    gpio_set_function(UART_0_TX, GPIO_FUNC_UART);
    gpio_set_function(UART_0_RX, GPIO_FUNC_UART);
    uart_puts(UART_0_PORT, "Hello, from UART0!\n");

    // UART1初期化(115200bps 8N1)
    uart_init(UART_1_PORT, UART_BAUD_RATE);
    gpio_set_function(UART_1_TX, GPIO_FUNC_UART);
    gpio_set_function(UART_1_RX, GPIO_FUNC_UART);
    uart_puts(UART_1_PORT, "Hello, from UART1!\n");
}

static void hw_adc_init(void)
{
    adc_init();

    adc_set_temp_sensor_enabled(true);

#if defined(MCU_RP2040)
    // RP2040の内蔵CPU温度センサ = ADC Ch4
    adc_select_input(ADC_CH_4);
#elif defined(MCU_RP2350)
    if(s_chip_package_flg == CHIP_PACKAGE_QFN60_RP2350A) {
        // RP2040とRP2350Aの内蔵CPU温度センサ = ADC Ch4
        adc_select_input(ADC_CH_4);
    } else {
        // RP2350Bの内蔵CPU温度センサ = ADC Ch8
        adc_select_input(ADC_CH_8);
    }
#endif
}

static void hw_wdt_init(void)
{
#ifdef _WDT_ENABLE_
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
    }

    watchdog_enable(_WDT_OVF_TIME_MS_, 1);
    _WDT_CNT_RST();
#endif
}

/**
 * @brief DMA設定API
 * 
 * @param dma_info DMA情報構造体
 */
static void hw_dma_config(dma_info_t dma_info)
{
    // DMAの未使用CH取得
    s_dma_ch = dma_claim_unused_channel(true);

    // DMAのコンフィグ
    dma_channel_config dma_config = dma_channel_get_default_config(s_dma_ch);

    // DMAの構造体に情報を叩いておく
    g_dma_ch_info[s_dma_ch].p_config = &dma_config;
    memset(&s_dma_dst_buf, 0x00, sizeof(s_dma_dst_buf));

    // DMA転送サイズ
    channel_config_set_transfer_data_size(&dma_config, dma_info.tx_size);

    // DMAでRead/Writeのアドレスを自動インクリメント
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, true);

    if(s_is_dma_irq_0_use != true) {
        // DMA転送完了割り込みをIRQ0で取るよう設定
        s_is_dma_irq_0_use = true;
        dma_channel_set_irq0_enabled(s_dma_ch, true);
        irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_0_handler);
        irq_set_enabled(DMA_IRQ_0, true);
    } else {
        // DMA転送完了割り込みをIRQ1で取るよう設定
        s_is_dma_irq_1_use = true;
        dma_channel_set_irq0_enabled(s_dma_ch, true);
        irq_set_exclusive_handler(DMA_IRQ_1, dma_irq_1_handler);
        irq_set_enabled(DMA_IRQ_1, true);
    }

    // DMAで転送開始
    dma_channel_configure(
        s_dma_ch,
        &dma_config,
        dma_info.p_dst_addr,   // 転送先
        dma_info.p_src_addr,   // 転送元
        dma_info.tx_cnt,       // 転送回数
        false                  // True=すぐ開始
    );
}

static void hw_dma_init(void)
{
    uint8_t i;

    // DMA CH0
    g_dma_ch_info[0].active = 1;
    g_dma_ch_info[0].tx_size = DMA_SIZE_8;
    g_dma_ch_info[0].tx_cnt = count_of(s_test_str);
    g_dma_ch_info[0].p_src_addr = s_test_str;
    g_dma_ch_info[0].p_dst_addr = s_dma_dst_buf;
    hw_dma_config(g_dma_ch_info[0]);

    // DNA CH1
    g_dma_ch_info[1].active = 1;
    g_dma_ch_info[1].tx_size = DMA_SIZE_32;
    g_dma_ch_info[1].tx_cnt = count_of(s_test_str);
    g_dma_ch_info[1].p_src_addr = s_dma_test_src_buf;
    g_dma_ch_info[1].p_dst_addr = s_dma_test_dst_buf;
    hw_dma_config(g_dma_ch_info[1]);

    // DMAの転送開始
    for(i = 0; i < DMA_CH_CNT; i++)
    {
        if(g_dma_ch_info[i].active != 0) {
            dma_channel_start(i);
        }
    }
}

int main()
{
    // Pico SDKの初期化
    stdio_init_all();

    // クロック初期化
    hw_clock_init();
    sleep_ms(100);

    // DMA初期化
    hw_dma_init();

    // UART初期化
    hw_uart_init();

#if defined(MCU_RP2350)
    // チップのパッケージ取得
    s_chip_package_flg = rp2xxx_get_chip_package();

    // 補間器(インターポレータ)初期化
    // ※2Dのテクスチャマッピング用
    hw_interp_init();
#endif

    // PIO初期化
    hw_pio_init();

    // ADC初期化
    hw_adc_init();

    // GPIO初期化
    hw_gpio_init();

    // PWM初期化
    hw_pwm_init();

    // I2C初期化
    hw_i2c_init();

    // SPI初期化
    hw_spi_init();

    // タイマー初期化
    hw_timer_init();

    // WDT初期化
    hw_wdt_init();

    // printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    // printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));

    s_core_num = get_core_num();

    // CPU Core1を起動
    multicore_launch_core1(core_1_main);

    // CPU Core0 アプリメイン
    core_0_main();
}