// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "headers/config.h"
#include "headers/DHT11.h"


esp_err_t read_DHT11(void) {
    printf("Sending start DHT11 signal...\n");

    uint8_t data[5] = {0};

    gpio_reset_pin(LED_GPIO);
    gpio_reset_pin(DHT11_PIN);

    //Start Signal
    // The reason I'm sending the start signal twice is for the sensor to give me updated readings. Because for some reason
    // it always gives me a stale reading the first time I try, only on the second try does it give me an updated
    // reading. So, instead of figuring out why that is, I am just sending the start signal twice. 
    for(int attempt = 0; attempt < 2; attempt++ ){
        gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(DHT11_PIN, 0); // pull low
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(DHT11_PIN, 1); // pull high
        esp_rom_delay_us(30);
        gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
        if(attempt == 0){
            vTaskDelay(pdMS_TO_TICKS(1500));;
        }
    }

    int64_t sensor_ACK_wait = esp_timer_get_time();
    while(gpio_get_level(DHT11_PIN) == 1){
        if(esp_timer_get_time() - sensor_ACK_wait >= TIMEOUT){
            printf("Timeout waiting for Sensor ACK\n");
            return ESP_FAIL;
        }
    }

    int64_t ACK_LOW_start = esp_timer_get_time();
    while(gpio_get_level(DHT11_PIN) == 0){
        if(esp_timer_get_time() - ACK_LOW_start >= TIMEOUT){
            printf("Timeout waiting for Sensor ACK to go HIGH\n");
            return ESP_FAIL;
        }
    }
    
    int64_t ACK_HIGH_start = esp_timer_get_time();
    while(gpio_get_level(DHT11_PIN) == 1){
        if(esp_timer_get_time() - ACK_HIGH_start >= TIMEOUT){
            printf("Timeout waiting for Sensor to send START bit\n");
            return ESP_FAIL;
        }
    }

    for(int i = 0; i < 40; i++){
        while(gpio_get_level(DHT11_PIN) == 0); //Start Bit

        int64_t high_time_start = esp_timer_get_time();
        while(gpio_get_level(DHT11_PIN) == 1){
            if(esp_timer_get_time() - high_time_start >= TIMEOUT){
                printf("Timeout waiting for Sensor to pull LOW");
                return ESP_FAIL;
            }
        }
        int64_t high_time_end = esp_timer_get_time();

        data[i / 8] <<= 1;
        if(high_time_end - high_time_start > 30){
            data[i/8] |= 1;
        }
    }
    if(data[4] != (uint8_t)(data[0] + data[1]+ data[2]+ data[3])){
        return ESP_FAIL;
    }

    int humidity = data[0];
    int temperature = data[2];
    printf("DHT11 Humidity = %d%%\nDHT11 Temperature = %dc\n", humidity, temperature);

    return ESP_OK;
}