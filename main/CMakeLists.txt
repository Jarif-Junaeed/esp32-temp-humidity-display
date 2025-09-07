idf_component_register(SRCS "DS18B20.c" "DHT11.c" "main.c"
                       PRIV_REQUIRES spi_flash
                       REQUIRES driver
                       REQUIRES esp_timer
                       INCLUDE_DIRS "" "headers")
