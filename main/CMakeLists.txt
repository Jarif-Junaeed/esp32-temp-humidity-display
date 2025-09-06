idf_component_register(SRCS "read_DHT11.c" "main.c"
                       PRIV_REQUIRES spi_flash
                       REQUIRES driver
                       REQUIRES esp_timer
                       INCLUDE_DIRS "" "headers")
