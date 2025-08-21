/**
 * @file hw_init.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief H/W初期化
 * @version 0.1
 * @date 2025-08-21
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef HW_INIT_H
#define HW_INIT_H

#include "muc_rpxxx_util.h"
#include "app_main.h"

#include "pico/multicore.h"
#include "hardware/adc.h"

#if defined(MCU_RP2350)
#include "pico/aon_timer.h"
#include <time.h>
#endif // MCU_RP2350

// DMA情報構造体
typedef struct {
    uint8_t active;                 // 使用状況
    dma_channel_config *p_config;   // コンフィグのポインタ
    uint8_t tx_size;                // 転送サイズ
    uint32_t tx_cnt;
    void *p_src_addr;               // 転送元のアドレスポインタ
    void *p_dst_addr;               // 転送先のアドレスポインタ
    uint8_t use_irq_ch;             // 使用中のIRQ(0xFFで無効)
} dma_info_t;

#define DMA_CH_CNT          16      // DMA CHの総数

extern dma_info_t g_dma_ch_info[DMA_CH_CNT];

#endif // HW_INIT_H