// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_timer.h"

// Pin assignments
#define LED_GPIO    GPIO_NUM_2
#define DHT11_PIN   GPIO_NUM_22
#define DS18B20_PIN GPIO_NUM_21

//ISR Context Struct
// Forward declaration of the struct type
typedef struct isr_device_context_t isr_device_context_t; //Allows the ISR to differentiate between which device aka GPIO is activating it

// Function pointer type that uses the forward-declared struct
typedef void (*device_func_t)(isr_device_context_t *ctx);

struct isr_device_context_t {
    const uint8_t id;
    const char* name;
    const uint16_t data_pin;
    uint32_t *active_edge_buffer;
    uint32_t *processing_edge_buffer;
    uint16_t edge_count;
    const uint16_t max_edge;
    uint64_t last_timestamp;
    bool data_transmitting;
    esp_timer_handle_t start_periodic_timer;
    esp_timer_handle_t finish_periodic_timer;
    const device_func_t action;
};

// Types for queue
typedef struct {
    uint32_t *buffer;   // pointer to completed buffer (processing pulse buffer)
    uint16_t length;    // number of recorded edge intervals in that buffer
} pulse_packet_t;

// Queue declaration
extern QueueHandle_t edge_duration_queue;

// ISR Context Devices declaration
extern isr_device_context_t DS18B20_ctx;

// ISR handler declaration
void gpio_isr_handler(void* arg);

// Timing constants
#define TIMEOUT      1000    // µs, for 1ms timeouts
#define LONG_TIMEOUT 1000000 // µs, for 1 second timeouts

// DS18B20 Constants
#define DS18B20_RESET_LOW_TIME 480  // Master reset pulse low time
#define DS18B20_RESET_RELEASE   5   // Small delay before releasing line
#define DS18B20_PRESENCE_WAIT  480  // Max wait for presence pulse
#define DS18B20_SLOT_RECOVERY   1   // Min recovery between slots
#define DS18B20_SLOT_TIME           60    // µs, total slot time
#define DS18B20_WRITE1_LOW          6     // µs, low for writing '1'
#define DS18B20_WRITE0_LOW          60    // µs, low for writing '0'
#define DS18B20_READ_INIT           1     // µs, low to start read slot
#define DS18B20_READ_SAMPLE         13    // µs, from start to sample
#define DS18B20_TIME_BETWEEN_SLOTS  2     // µs, time to wait before starting another slot
#define DS18B20_TOTAL_READ_SLOTS    72    // After sending a read scratchpad command, the DS18B20 will send a total of 9 bytes aka 72 bits  
#define DS18B20_CRC8_POLY 0x8C      // Based on the Polynomial for DS18B20 8-bit CRC (X^8 + X^5 + X^4 + 1)
                                    // Since i'm doing the crc check from the LSB, i'm using 0x8C instead of 0x31

#endif // CONFIG_H