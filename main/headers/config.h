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
#define TIMEOUT     1000 // µs

//DS18B20 Constants
#define ONEWIRE_RESET_LOW_TIME 480  // Master reset pulse low time
#define ONEWIRE_RESET_RELEASE   5   // Small delay before releasing line
#define ONEWIRE_PRESENCE_WAIT  480  // Max wait for presence pulse
#define ONEWIRE_SLOT_RECOVERY   1   // Min recovery between slots

#define ONEWIRE_SLOT_TIME     60   // µs, total slot time
#define ONEWIRE_WRITE1_LOW    6    // µs low for writing '1'
#define ONEWIRE_WRITE0_LOW    60   // µs low for writing '0'
#define ONEWIRE_READ_INIT     2    // µs low to start read slot
#define ONEWIRE_READ_SAMPLE   15   // µs from start to sample

#endif // CONFIG_H
