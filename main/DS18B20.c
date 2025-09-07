// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT
#include <stdint.h>

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

static uint8_t DS18B20_crc_check(const uint8_t *data, uint8_t len){
    uint8_t crc = 0;

    for(uint8_t i = 0; i < len; i++){
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++){
            if(crc & 0x01){
                crc = (crc >> 1) ^ DS18B20_CRC8_POLY;
            }
            else{
                crc >>= 1;
            }
        }
    }

    return crc;
}

static esp_err_t reset_DS18B20(void){
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
    return ESP_OK;
}

esp_err_t read_DS18B20(void){

    if (reset_DS18B20() != ESP_OK) {
        printf("Failed to reset DS18B20 before conversion\n");
        return ESP_FAIL;
    }

    DS18B20_onewire_write_byte(0xCC);
    DS18B20_onewire_write_byte(0x44);

    int64_t tconv_START_time = esp_timer_get_time();
    while(!DS18B20_onewire_read_bit()){
        if(esp_timer_get_time() - tconv_START_time > LONG_TIMEOUT){ //1 second timeout
            return ESP_FAIL;
        }
    }

    if (reset_DS18B20() != ESP_OK) {
        printf("Failed to reset DS18B20 before reading\n");
        return ESP_FAIL;
    }

    DS18B20_onewire_write_byte(0xCC);
    DS18B20_onewire_write_byte(0xBE);
    
    uint8_t data[9];
    for(int i = 0; i < 9; i++){
        data[i]=DS18B20_onewire_read_byte();
    }

    if(DS18B20_crc_check(data, 8) != data[8]){
        printf("CRC Mismatch!\n");
        return ESP_FAIL;
    }
    else{
        printf("CRC Match!\n");
        return ESP_OK;
    }

}