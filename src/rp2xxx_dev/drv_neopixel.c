/**
 * @file drv_neopixel.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief NeoPixelドライバ
 * @version 0.1
 * @date 2025-08-22
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#include "drv_neopixel.h"

neopixel_t *s_p_neopixel;
static void set_neopixel_data_show(neopixel_t *p_neopixel);

#ifdef NEOPIXEL_CTRL_PIO
static void pio_neopixel_begin(neopixel_t *p_neopixel, PIO pio, uint sm, uint offset,  uint pin);
static PIO s_pio = pio1;
static uint s_sm = 0;
static uint s_offset = 0;
#endif // NEOPIXEL_CTRL_PIO

#ifdef NEOPIXEL_CTRL_SPI
static void neopixel_data_spi_tx(uint32_t neopixel_data);

/**
 * @brief SPIでNeoPixelのデータを送信
 * @param neopixel_data 送信するNeoPixelデータ（GRB形式、24bit）
 *
 * NeoPixelのタイミング:
 * - T0(0bit): T0H 220ns~380ns, T0L 580ns~1000ns
 * - T1(1bit): T1H 580ns~1000ns, T1L 580ns~1000ns
 * - Treset: Low 280μs以上
 *
 * T0H : 350ns
 * T0L : 900ns
 * T0 : = (350ns + 900ns) = 1.25us = 800KHz
 * T1H : 650ns
 * T1L : 600ns
 * T1 : = (650ns + 600ns) = 1.25us = 800KHz
 *
 * SPIが8MHz動作時1クロックは125ns
 * SPI 8MHz(125ns)なら8bitデータでT0,T1が800KHzになる
 */
static void neopixel_data_spi_tx(uint32_t neopixel_data)
{
    uint32_t mask = 0x800000; // MSBから開始（bit23）
    uint8_t spi_data_buf[NEOPIXEL_DATA_BIT];

    // 割り込み禁止
    // _DI();

    // 24ビット分のデータを送信
    for (uint8_t i = 0; i < NEOPIXEL_DATA_BIT; i++)
    {
        // T1 (1bitデータ)
        if (neopixel_data & mask) {
            spi_data_buf[i] = T1_BIT_DATA;
        // T0 (0bitデータ)
        } else {
            spi_data_buf[i] = T0_BIT_DATA;
        }
        mask >>= 1; // 次のビットへ
    }

    // 割り込み許可
    // _EI();

    // SPIで送信
    while(spi_is_writable(spi0) != true)
    {
        _WDT_CNT_RST();
    };
    spi_write_blocking(spi0, spi_data_buf, sizeof(spi_data_buf));
}
#endif // NEOPIXEL_CTRL_SPI

static void set_neopixel_data_show(neopixel_t *p_neopixel)
{
    uint8_t i;

    for ( i = 0; i < p_neopixel->led_cnt; i++)
    {
#ifdef NEOPIXEL_CTRL_PIO
        pio_sm_put_blocking(s_pio, s_sm, p_neopixel->p_pixel_grb_buf[i].grb_color.u32_grb << 8u);
#else
        neopixel_data_spi_tx(p_neopixel->p_pixel_grb_buf[i].grb_color.u32_grb);
#endif // NEOPIXEL_CTRL_PIO
    }

#ifdef NEOPIXEL_CTRL_SPI
    // Neopixelのリセット 定格@280us以上
    sleep_us(300);
#endif
}

// H/W(PIO)でNeoPixelをコントロール
#ifdef NEOPIXEL_CTRL_PIO
static void pio_neopixel_begin(neopixel_t *p_neopixel, PIO pio, uint sm, uint offset,  uint pin)
{
    pio_neopixel_init(pio, sm, offset, pin, 800000, false);
}
#endif // NEOPIXEL_CTRL_PIO

void drv_neopixel_init(neopixel_t *p_neopixel)
{
    s_p_neopixel = p_neopixel;

#ifdef NEOPIXEL_CTRL_PIO
    bool ret = false;
    s_offset = pio_add_program(s_pio, &neopixel_program);
    ret = pio_claim_free_sm_and_add_program_for_gpio_range(&neopixel_program, &s_pio, &s_sm, &s_offset, p_neopixel->data_pin, 1, true);
    hard_assert(ret);
    pio_neopixel_begin(p_neopixel, s_pio, s_sm, s_offset, p_neopixel->data_pin);
#endif // NEOPIXEL_CTRL_PIO

    drv_neopixel_clear(p_neopixel);
}

void drv_neopixel_set_pixel_rgb(neopixel_t *p_neopixel, uint8_t led, uint8_t red, uint8_t green, uint8_t blue)
{
    p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.reserved = 0;
    p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = blue;
    p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = red;
    p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = green;
    set_neopixel_data_show(p_neopixel);
}

void drv_neopixel_set_pixel_color(neopixel_t *p_neopixel, uint8_t led, uint8_t color)
{
    // 色指定の状態遷移
    switch (color) {
        case NEOPIXEL_COLOR_RED:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0xFF, 0, 0);
            break;
        case NEOPIXEL_COLOR_GREEN:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0, 0xFF, 0);
            break;

        case NEOPIXEL_COLOR_BLUE:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0, 0, 0xFF);
            break;

        case NEOPIXEL_COLOR_YELLOW:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0xFF, 0xFF, 0);
            break;

        case NEOPIXEL_COLOR_CYAN:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0, 0xFF, 0xFF);
            break;

        case NEOPIXEL_COLOR_MAGENTA:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0xFF, 0, 0xFF);
            break;

        case NEOPIXEL_COLOR_ORANGE:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0xFF, 0xA5, 0);
            break;

        case NEOPIXEL_COLOR_PURPLE:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0x80, 0, 0x80);
            break;

        case NEOPIXEL_COLOR_PINK:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0xFF, 0xC0, 0xCB);
            break;

        case NEOPIXEL_COLOR_WHITE:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0xFF, 0xFF, 0xFF);
            break;

        default:
            drv_neopixel_set_pixel_rgb(p_neopixel, led, 0, 0, 0); // OFF
            break;
    }
}

void drv_neopixel_get_pixel_color(neopixel_t *p_neopixel, uint8_t led, uint8_t color)
{
    // 色指定の状態遷移
    switch (color) {
        case NEOPIXEL_COLOR_RED:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0;
            break;
        case NEOPIXEL_COLOR_GREEN:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0;
            break;

        case NEOPIXEL_COLOR_BLUE:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0xFF;
            break;

        case NEOPIXEL_COLOR_YELLOW:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0;
            break;

        case NEOPIXEL_COLOR_CYAN:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0xFF;
            break;

        case NEOPIXEL_COLOR_MAGENTA:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0xFF;
            break;

        case NEOPIXEL_COLOR_ORANGE:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xA5;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0;
            break;

        case NEOPIXEL_COLOR_PURPLE:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0x80;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xA5;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0x80;
            break;

        case NEOPIXEL_COLOR_PINK:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xC0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0xCB;
            break;

        case NEOPIXEL_COLOR_WHITE:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0xFF;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0xFF;
            break;

        default:
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.red = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.green = 0;
            p_neopixel->p_pixel_grb_buf[led].grb_color.grb_bit.blue = 0;
            break;
    }
}

void drv_neopixel_clear(neopixel_t *p_neopixel)
{
    for (uint8_t i = 0; i < p_neopixel->led_cnt; i++)
    {
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.reserved = 0;
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.blue = 0;
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.red = 0;
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.green = 0;
        set_neopixel_data_show(p_neopixel);
    }
}

void drv_neopixel_show(neopixel_t *p_neopixel)
{
    set_neopixel_data_show(p_neopixel);
}

void drv_neopixel_set_all_led_color(neopixel_t *p_neopixel, uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint8_t i = 0; i < p_neopixel->led_cnt; i++)
    {
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.reserved = 0;
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.blue = blue;
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.red = red;
        p_neopixel->p_pixel_grb_buf[i].grb_color.grb_bit.green = green;
        set_neopixel_data_show(p_neopixel);
    }
}

uint32_t drv_neopixel_get_color(neopixel_t *p_neopixel, uint8_t led)
{
    uint32_t color = 0;
    color = p_neopixel->p_pixel_grb_buf->grb_color.u32_grb;
    return color;
}

void drv_neopixel_pixel_color_fade(void)
{
    static uint8_t s_idx = 0, s_r = 0, s_g = 0, s_b = 0;

    if (s_r < 0xFF) {
        s_r++;
    } else if (s_g < 0xFF) {
        s_g++;
    } else if (s_b < 0xFF) {
        s_b++;
    } else {
        s_r = 0;
        s_g = 0;
        s_b = 0;

        if (s_idx >= (s_p_neopixel->led_cnt - 1)) {
            s_idx = 0;
            drv_neopixel_clear(s_p_neopixel);
        } else {
            s_idx++;
        }
    }

    s_p_neopixel->p_pixel_grb_buf[s_idx].grb_color.grb_bit.red = s_r;
    s_p_neopixel->p_pixel_grb_buf[s_idx].grb_color.grb_bit.green = s_g;
    s_p_neopixel->p_pixel_grb_buf[s_idx].grb_color.grb_bit.blue = s_b;
    drv_neopixel_set_pixel_rgb(s_p_neopixel, s_idx, s_r, s_g, s_b);
}