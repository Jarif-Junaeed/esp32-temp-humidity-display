// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#ifndef DS18B20_ISR_H
#define DS18B20_ISR_H

#include <stdint.h>

#include "esp_err.h"

#include "config.h"

esp_err_t DS18B20_start_single_slave_tconv(void);
esp_err_t DS18B20_start_single_slave_read(void);
void DS18B20_start_read_slot(isr_device_context_t* ctx);
void DS18B20_create_timer(void);
void DS18B20_start_read_sequence(void);
void DS18B20_stop_read_sequence(void);
uint8_t DS18B20_crc_check(const uint8_t *data, uint8_t len);
void DS18B20_init_pin(void);

#endif