// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "headers/config.h"
#include "headers/DS18B20.h"

static void DS18B20_onewire_write_bit(int bit){

    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    
    gpio_set_level(DS18B20_PIN, 0);

    if(bit){
        //write 1
        esp_rom_delay_us(ONEWIRE_WRITE1_LOW);
        gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
        esp_rom_delay_us(ONEWIRE_SLOT_TIME - ONEWIRE_WRITE1_LOW + ONEWIRE_SLOT_RECOVERY);
    }
    else{
        //write 0
        esp_rom_delay_us(ONEWIRE_WRITE0_LOW);
        gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
        esp_rom_delay_us(ONEWIRE_SLOT_RECOVERY);
    }
}

static int DS18B20_onewire_read_bit(void){

    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    esp_rom_delay_us(ONEWIRE_READ_INIT);

    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
    esp_rom_delay_us(ONEWIRE_READ_SAMPLE - ONEWIRE_READ_INIT);

    int bit = gpio_get_level(DS18B20_PIN);

    esp_rom_delay_us(ONEWIRE_SLOT_TIME - ONEWIRE_READ_SAMPLE + ONEWIRE_SLOT_RECOVERY);

    return bit;

}

static void DS18B20_onewire_write_byte(uint8_t byte){
    for(int i = 0; i < 8; i++){
        DS18B20_onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t DS18B20_onewire_read_byte(void){
    uint8_t value = 0;
    for(int i = 0; i < 8; i++){
        value >>=1;
        if(DS18B20_onewire_read_bit()){
            value |= 0x80;
        }
    }
    return value;
}

esp_err_t read_DS18B20(void){
    esp_rom_delay_us(ONEWIRE_RESET_RELEASE);
    gpio_reset_pin(DS18B20_PIN);

    //Reset Pulse
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    esp_rom_delay_us(ONEWIRE_RESET_LOW_TIME);
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);

    //Presenece Pulse
    int64_t sensor_PRESCENCE_wait = esp_timer_get_time();
    while(gpio_get_level(DS18B20_PIN) == 1){
        if(esp_timer_get_time() - sensor_PRESCENCE_wait >= TIMEOUT){
            return ESP_FAIL;
        }
    }

    int64_t sensor_PRESCENCE_start = esp_timer_get_time();
    while(gpio_get_level(DS18B20_PIN) == 0){
        if(esp_timer_get_time() - sensor_PRESCENCE_start >= TIMEOUT){
            return ESP_FAIL;
        }
    }

    int64_t rx_wait = ONEWIRE_RESET_LOW_TIME - (esp_timer_get_time() - sensor_PRESCENCE_start);
    esp_rom_delay_us(rx_wait);
    return ESP_OK;
}