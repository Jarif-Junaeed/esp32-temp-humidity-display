// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include "DS18B20_ISR.h"
#include "config.h"

#define MAX_DS18B20_EDGES 320//Well, there are actually 288 edges in the worst possible case(all 0), as there are 4 edges for each
                              //bit if the bit is a 0, of which there are 72 of them(aka 9 bytes), but I chose a bigger size to be safe


//DS18B20
static uint32_t DS18B20_active_edge_buffer[MAX_DS18B20_EDGES];
static uint32_t DS18B20_processing_edge_buffer[MAX_DS18B20_EDGES];

isr_device_context_t DS18B20_ctx = { 
    .id = 1,
    .name = "DS18B20",
    .data_pin = DS18B20_PIN,
    .active_edge_buffer = DS18B20_active_edge_buffer,
    .processing_edge_buffer = DS18B20_processing_edge_buffer,
    .edge_count = 0,
    .max_edge = MAX_DS18B20_EDGES,
    .last_timestamp = 0,
    .data_transmitting = false,
    .start_periodic_timer = NULL,
    .finish_periodic_timer = NULL,
    .action = DS18B20_start_read_slot
};