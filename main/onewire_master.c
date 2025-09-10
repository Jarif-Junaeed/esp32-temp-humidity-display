// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "config.h"
#include "onewire_master.h"

static void onewire_periodic_cb(void* arg){
    isr_device_context_t* dev = (isr_device_context_t*) arg;
    if(!dev->data_transmitting) return;
    if (dev->action) {
        dev->action(dev);
    }
}

static void onewire_finish_cb(void* arg){
    isr_device_context_t* dev = (isr_device_context_t*) arg;
    if(!dev) return;

    gpio_isr_handler_remove(dev->data_pin);

    uint16_t captured_count = dev->edge_count;

    //Swaps buffers, so that we don't have to wait to start another read.
    uint32_t* temp = dev->active_edge_buffer;
    dev->active_edge_buffer = dev->processing_edge_buffer;
    dev->processing_edge_buffer = temp;

    pulse_packet_t pkt = {
        .buffer = dev->processing_edge_buffer,
        .length = captured_count
    };
    xQueueSend(edge_duration_queue, &pkt, 0);

    dev->edge_count = 0;
    dev->data_transmitting = false;

    if (dev->start_periodic_timer) {
        esp_timer_stop(dev->start_periodic_timer);
    }
}

void onewire_master_create_timer(isr_device_context_t *dev){
    if(!dev) return;

    char name_period[32];
    snprintf(name_period, sizeof(name_period), "%s_%d_prd", dev->name, dev->data_pin);
    
    if(!dev->start_periodic_timer){
        esp_timer_create_args_t args_a = {
            .callback = onewire_periodic_cb,
            .arg = dev,
            .name = name_period
        };
        esp_timer_create(&args_a, &dev->start_periodic_timer);
    }

    if(!dev->finish_periodic_timer){
        esp_timer_create_args_t args_b = {
            .callback = onewire_finish_cb,
            .arg = dev,
            .name = name_period
        };
        esp_timer_create(&args_b, &dev->finish_periodic_timer);
    }
}

void onewire_master_start_sequence(isr_device_context_t *dev, uint32_t slot_period_us, uint32_t num_slots, uint32_t slack_us){
    if(!dev || dev->data_transmitting) return;

    onewire_master_create_timer(dev);
    dev->edge_count = 0;
    dev->data_transmitting = true;
    dev->last_timestamp = esp_timer_get_time();
    gpio_isr_handler_add(dev->data_pin, gpio_isr_handler, dev);
    esp_timer_start_periodic(dev->start_periodic_timer, slot_period_us);
    uint64_t finish_after = (uint64_t)(num_slots * slot_period_us) + slack_us;
    esp_timer_start_once(dev->finish_periodic_timer, finish_after);
}

void onewire_master_stop_sequence(isr_device_context_t *dev){
    if (!dev) return;
    dev->data_transmitting = false;
    if (dev->start_periodic_timer) esp_timer_stop(dev->start_periodic_timer);
    gpio_isr_handler_remove(dev->data_pin);
}