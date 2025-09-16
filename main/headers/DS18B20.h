// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#ifndef DS18B20_H
#define DS18B20_H

#include "esp_err.h"

// Reads the temperature from a DS18B20 sensor.
// Returns ESP_OK if the sensor responds correctly to the reset and presence sequence,
// or ESP_FAIL if there is a communication error (e.g., timeout waiting for presence pulse).
esp_err_t read_DS18B20(void);

void DS18B20_init_pin(void);

#endif