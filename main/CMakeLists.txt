idf_component_register(SRCS "onewire_master.c" "devices.c" "DS18B20_ISR.c" "DS18B20.c" "DHT11.c" "main.c"
                       PRIV_REQUIRES spi_flash
                       REQUIRES driver
                       REQUIRES freertos
                       REQUIRES esp_timer
                       INCLUDE_DIRS "" "headers")
