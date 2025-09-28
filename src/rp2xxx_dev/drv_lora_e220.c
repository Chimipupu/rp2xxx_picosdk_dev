/**
 * @file drv_lora_e220.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief LoRaモジュール E220-900T22のドライバ
 * @version 0.1
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "drv_lora_e220.h"

// [Pico SDK Inc]
#include "hardware/gpio.h"
#include "hardware/uart.h"

// [My SRC Inc]
#include "pcb_def.h"

static uint8_t s_tx_buf[LORA_TX_BUF_SIZE] = {0};

#ifdef DEBUG_LORA_E220
const uint8_t g_test_data[] = {0xAB, 0xCD, 0xEF};
#endif

/**
 * @brief LoRaモジュール E220-900T22のブロードキャスト送信関数
 * 
 * @param ch 送信Ch
 * @param data_size データサイズ
 * @param p_data データ長
 * @return uint8_t 0x00...OK, 0xFF...データサイズ超過エラー
 */
uint8_t drv_lora_e220_broad_cast_tx(uint8_t ch, uint8_t data_size, const uint8_t *p_data)
{
    uint8_t *p;
    uint8_t i;
    uint8_t ret = 0xFF;

    p = (uint8_t *) p_data;

    if(data_size <= LORA_E220_PACKET_SIZE) {
        uart_putc(UART_1_PORT, 0xFF);
        uart_putc(UART_1_PORT, 0xFF);
        uart_putc(UART_1_PORT, ch);
        for(i = 0; i < data_size; i++)
        {
            uart_putc(UART_1_PORT, *p);
            p++;
        }
        uart_puts(UART_1_PORT, "\r\n");

        ret = 0x00;
    }

    return ret;
}

/**
 * @brief LoRaモジュール E220-900T22の初期化関数
 * 
 */
void drv_lora_e220_init(void)
{
    // [LoRa E220-900T22]
    // M0ピン
    gpio_set_function(LORA_E220_M0_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(LORA_E220_M0_PIN, GPIO_OUT);
    gpio_put(LORA_E220_M0_PIN, 0x00);

    // M1ピン
    gpio_set_function(LORA_E220_M1_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(LORA_E220_M1_PIN, GPIO_OUT);
    gpio_put(LORA_E220_M1_PIN, 0x00);

    memset(&s_tx_buf[0], 0, LORA_TX_BUF_SIZE);
}

/**
 * @brief LoRaモジュール E220-900T22のメイン関数
 * 
 */
void app_lora_e220_main(void)
{
#ifdef DEBUG_LORA_E220
    drv_lora_e220_broad_cast_tx(0x01, sizeof(g_test_data), &g_test_data[0]);
#endif // DEBUG_LORA_E220
}