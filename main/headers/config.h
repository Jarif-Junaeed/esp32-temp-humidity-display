// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"

// Pin assignments
#define LED_GPIO    GPIO_NUM_2
#define DHT11_PIN   GPIO_NUM_22
#define DS18B20_PIN GPIO_NUM_21

// Timing constants
#define TIMEOUT      1000    // µs, for 1ms timeouts
#define LONG_TIMEOUT 1000000 // µs, for 1 second timeouts

// DS18B20 Constants
#define DS18B20_RESET_LOW_TIME            480   // Master reset pulse low time
#define DS18B20_RESET_RELEASE             5     // Small delay before releasing line
#define DS18B20_PRESENCE_WAIT             480   // Max wait for presence pulse
#define DS18B20_SLOT_RECOVERY             1     // Min recovery between slots
#define DS18B20_SLOT_TIME                 60    // µs, total slot time
#define DS18B20_WRITE1_LOW                6     // µs, low for writing '1'
#define DS18B20_WRITE0_LOW                60    // µs, low for writing '0'
#define DS18B20_READ_INIT                 1     // µs, low to start read slot
#define DS18B20_READ_SAMPLE               13    // µs, from start to sample
#define DS18B20_TIME_BETWEEN_SLOTS        2     // µs, time to wait before starting another slot
#define DS18B20_CONVERSION_POLL_INTERVAL  7500  // µs, delay between successive polls to check if the DS18B20 has finished temperature conversion
#define DS18B20_TOTAL_READ_SLOTS          72    // After sending a read scratchpad command, the DS18B20 will send a total of 9 bytes aka 72 bits  

#define DS18B20_CRC8_POLY 0x8C            // Based on the Polynomial for DS18B20 8-bit CRC (X^8 + X^5 + X^4 + 1)
                                          // Since i'm doing the crc check from the LSB, i'm using 0x8C instead of 0x31

#endif // CONFIG_H
