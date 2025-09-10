// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "DS18B20_ISR.h"
#include "onewire_master.h"
#include "config.h"

static SemaphoreHandle_t DS18B20_conv_sema = NULL;

static esp_err_t reset_DS18B20(void){
    esp_rom_delay_us(DS18B20_RESET_RELEASE);

    //Reset Pulse
    gpio_set_level(DS18B20_PIN, 0);
    esp_rom_delay_us(DS18B20_RESET_LOW_TIME);
    gpio_set_level(DS18B20_PIN, 1);

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

static void DS18B20_write_bit(int bit){

    gpio_set_level(DS18B20_PIN, 0);

    if(bit){
        //write 1
        esp_rom_delay_us(DS18B20_WRITE1_LOW);
        gpio_set_level(DS18B20_PIN, 1);
        esp_rom_delay_us(DS18B20_SLOT_TIME - DS18B20_WRITE1_LOW + DS18B20_SLOT_RECOVERY);
    }
    else{
        //write 0
        esp_rom_delay_us(DS18B20_WRITE0_LOW);
        gpio_set_level(DS18B20_PIN, 1);
        esp_rom_delay_us(DS18B20_SLOT_RECOVERY);
    }
}

static void DS18B20_write_byte(uint8_t byte){
    for(int i = 0; i < 8; i++){
        DS18B20_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static void IRAM_ATTR DS18B20_conv_isr(void* arg){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    isr_device_context_t* dev = (isr_device_context_t*) arg;

    if(gpio_get_level(dev->data_pin) == 1){
        if(DS18B20_conv_sema){
            xSemaphoreGiveFromISR(DS18B20_conv_sema, &xHigherPriorityTaskWoken);
        }
    }

    gpio_isr_handler_remove(dev->data_pin);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

esp_err_t DS18B20_start_single_slave_tconv(void){
    
    if (reset_DS18B20() != ESP_OK) {
        printf("Failed to reset DS18B20 before conversion\n");
        return ESP_FAIL;
    }

    if(!DS18B20_conv_sema) {
        DS18B20_conv_sema = xSemaphoreCreateBinary();
        if(!DS18B20_conv_sema){
            printf("Failed to create conv semaphore\n");
            return ESP_FAIL;
        }
    }
    else{
        xSemaphoreTake(DS18B20_conv_sema, 0);
    }

    DS18B20_write_byte(0xCC);
    DS18B20_write_byte(0x44);

    gpio_set_intr_type(DS18B20_PIN, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(DS18B20_PIN, DS18B20_conv_isr, &DS18B20_ctx);

    if(xSemaphoreTake(DS18B20_conv_sema, pdMS_TO_TICKS(LONG_TIMEOUT/1000)) == pdTRUE) {
        gpio_set_intr_type(DS18B20_PIN, GPIO_INTR_ANYEDGE);
        return ESP_OK;
    }
    else{
        printf("CONV took too long\n");
        gpio_isr_handler_remove(DS18B20_PIN);
        gpio_set_intr_type(DS18B20_PIN, GPIO_INTR_ANYEDGE);
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

esp_err_t DS18B20_start_single_slave_read(void){
    
    if(reset_DS18B20() != ESP_OK){
        printf("Failed to reset DS18B20 before reading\n");
        return ESP_FAIL;
    }

    DS18B20_write_byte(0xCC);
    DS18B20_write_byte(0xBE);    

    return ESP_OK;
}

void IRAM_ATTR DS18B20_start_read_slot(isr_device_context_t* dev){

    gpio_set_level(dev->data_pin, 0);
    esp_rom_delay_us(DS18B20_READ_INIT);
    gpio_set_level(dev->data_pin, 1);
}

uint8_t DS18B20_crc_check(const uint8_t *data, uint8_t len){
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

void DS18B20_create_timer(void){
    onewire_master_create_timer(&DS18B20_ctx);
}

void DS18B20_start_read_sequence(void){
    onewire_master_start_sequence(
        &DS18B20_ctx, 
        DS18B20_SLOT_TIME + DS18B20_SLOT_RECOVERY, 
        DS18B20_TOTAL_READ_SLOTS, 
        DS18B20_TIME_BETWEEN_SLOTS
    );
}

void DS18B20_stop_read_sequence(void){
    onewire_master_stop_sequence(&DS18B20_ctx);
}

void DS18B20_init_pin(void){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DS18B20_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE, // Make sure to have an external pull up resistor(a 3.3k-5k should work) between the GPIO and DATA line
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&io_conf);
}