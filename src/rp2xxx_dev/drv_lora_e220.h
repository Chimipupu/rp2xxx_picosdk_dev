/**
 * @file drv_lora_e220.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief LoRaモジュール E220-900T22のドライバ
 * @version 0.1
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef DRV_LORA_E220_H
#define DRV_LORA_E220_H

// #include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define LORA_E220_USE

#define LORA_E220_M0_PIN        20    // E220-900T22のM0ピン
#define LORA_E220_M1_PIN        21    // E220-900T22のM1ピン

#define LORA_E220_PACKET_SIZE   200   // E220-900T22のパケットサイズ
#define LORA_TX_BUF_SIZE        LORA_E220_PACKET_SIZE

uint8_t drv_lora_e220_broad_cast_tx(uint8_t ch, uint8_t data_size, const uint8_t *p_data);
void drv_lora_e220_init(void);
void app_lora_e220_main(void);

#define DEBUG_LORA_E220

#endif // DRV_LORA_E220_H