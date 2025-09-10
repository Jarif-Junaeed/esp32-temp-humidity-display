// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "config.h"
#include "DHT11.h"
#include "DS18B20.h"
#include "DS18B20_ISR.h"

QueueHandle_t edge_duration_queue;

//I have not touched the gpio_isr_handler yet, because not sure how to modify in relation to all the other changes i made
void IRAM_ATTR gpio_isr_handler(void* arg){
    isr_device_context_t* dev = (isr_device_context_t*) arg;

    uint64_t current_timestamp = esp_timer_get_time();
    if(dev->edge_count < dev->max_edge){
        dev->active_edge_buffer[dev->edge_count] = current_timestamp - dev->last_timestamp;
        dev->edge_count++;
    }
    dev->last_timestamp = current_timestamp;
}

void app_main(void){
    gpio_reset_pin(LED_GPIO);
    gpio_reset_pin(DHT11_PIN);

    /*
    gpio_reset_pin(DS18B20_PIN);

    DS18B20_init_pin();

    edge_duration_queue = xQueueCreate(4, sizeof(pulse_packet_t));
    if (!edge_duration_queue) {
        printf("Failed to create queue\n");
        return;
    }

    gpio_install_isr_service(0);

    DS18B20_create_timer();*/
    
    printf("Press 'p' + Enter to send start signal\n");
    while (1) {
        char c = getchar(); // wait for user input
        if (c == 'p' || c == 'P') {
            read_DHT11();
        
            /*
            //Read temp from DS18B20
            if(DS18B20_start_single_slave_tconv() == ESP_FAIL){
                printf("Failed to send TCONV command to DS18B20\n");
            }
            else{
                DS18B20_start_read_sequence();
            }*/
            read_DS18B20();
            
            printf("Press 'p' + Enter to send again\n");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}