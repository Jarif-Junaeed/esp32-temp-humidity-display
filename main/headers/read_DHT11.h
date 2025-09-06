// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#ifndef DHT11_H
#define DHT11_H

#include "esp_err.h"   // For esp_err_t

// Reads temperature and humidity from a DHT11 sensor.
// Returns ESP_OK if reading was successful, ESP_FAIL if there was an error (timeout, checksum failure, etc.) 
esp_err_t read_DHT11(void);

#endif 