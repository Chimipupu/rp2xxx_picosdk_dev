/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2025-11-23
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

// C Std Lib
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// PicoSDK Lib
#include "pico/version.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"

// MyApp Lib
#include "muc_rpxxx_util.h"
#include "pcb_def.h"

// --------------------------------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_DBG_COM_USE
// --------------------------------------------------------------------------

void show_mem_dump(uint32_t dump_addr, uint32_t dump_size);
void i2c_slave_scan(uint8_t i2c_port);
void proc_exec_time(void (*p_func)(void), const char* p_func_name, ...);
void core_0_main(void);
void core_1_main(void);

#endif // APP_MAIN_H