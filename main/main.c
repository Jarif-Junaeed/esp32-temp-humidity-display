// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "headers/config.h"
#include "headers/DHT11.h"
#include "headers/DS18B20.h"

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
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}