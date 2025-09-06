//TODO: finish writing all the sensor functions to read from the DS1820, HW-498(Analog Temp Sensor and don't use a library for this, manually implement this) and DHT11 then combine all of them to produce accurate
//temps by fusing them, then display it on a 16X2 LCD and send over wifi 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <stdio.h>

#define LED_GPIO    GPIO_NUM_2
#define DHT11_PIN   GPIO_NUM_22
#define DS18B20_PIN GPIO_NUM_21
#define TIMEOUT     1000 //µs

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

void DS18B20_onewire_write_bit(int bit){

    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    
    gpio_set_level(DS18B20_PIN, 0);

    if(bit){
        //write 1
        esp_rom_delay_us(ONEWIRE_WRITE1_LOW);
        gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
        esp_rom_delay_us(ONEWIRE_SLOT_TIME - ONEWIRE_WRITE1_LOW);
    }
    else{
        //write 0
        esp_rom_delay_us(ONEWIRE_WRITE0_LOW);
        gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
        esp_rom_delay_us(ONEWIRE_SLOT_RECOVERY);
    }
}

int DS18B20_onewire_read_bit(void){

    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    esp_rom_delay_us(ONEWIRE_READ_INIT);

    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
    esp_rom_delay_us(ONEWIRE_READ_SAMPLE - ONEWIRE_READ_INIT);

    int bit = gpio_get_level(DS18B20_PIN);

    esp_rom_delay_us(ONEWIRE_SLOT_TIME - ONEWIRE_READ_SAMPLE);

    return bit;

}

void DS18B20_onewire_write_byte(uint8_t byte){
    for(int i = 0; i < 8; i++){
        DS18B20_onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

uint8_t DS18B20_onewire_read_byte(void){
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
    gpio_set_level(DS18B20_PIN, 1);

    //Presenece Pulse
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);

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

esp_err_t read_DHT11(void) {
    printf("Sending start DHT11 signal...\n");

    vTaskDelay(pdMS_TO_TICKS(2000));

    uint8_t data[5] = {0};

    gpio_reset_pin(LED_GPIO);
    gpio_reset_pin(DHT11_PIN);

    //Start Signal
    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0); // pull low
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT11_PIN, 1); // pull high
    esp_rom_delay_us(30);

    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);

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
    printf("Humidity = %d%%\nTemperature = %dc\n", humidity, temperature);

    return ESP_OK;
}

void app_main(void)
{
    printf("Press 'p' + Enter to send start signal\n");

    while (1) {
        char c = getchar(); // wait for user input
        if (c == 'p' || c == 'P') {
            read_DHT11();
            read_DS18B20();
            printf("Press 'p' + Enter to send again\n");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}