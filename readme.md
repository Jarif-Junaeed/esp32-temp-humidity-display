# ESP32 Temperature & Humidity Sensor Reader

A bare-metal ESP-IDF project for the ESP32 that reads temperature and humidity from a **DHT11** sensor and temperature from a **DS18B20** sensor, using bit-banged protocols implemented from scratch without external sensor libraries.

---

## Features

- DHT11 single-wire protocol implementation for humidity and temperature
- DS18B20 1-Wire protocol implementation for high-resolution temperature
- CRC-8 validation on DS18B20 scratchpad data
- Interactive serial interface — trigger readings on demand by pressing `p`
- All timing constants centralized in `config.h` for easy tuning

---

## Hardware

| Component | GPIO  |
|-----------|-------|
| DHT11     | 22    |
| DS18B20   | 21    |
| LED       | 2     |

> The DS18B20 data line requires a **4.7kΩ pull-up resistor** to 3.3V.  
> The DHT11 data line requires a **10kΩ pull-up resistor** to 3.3V.

---

## Project Structure

```
├── main/
│   ├── headers/
│   │   ├── config.h       # Pin assignments, timing constants, DS18B20 command macros
│   │   ├── DHT11.h        # DHT11 public API
│   │   └── DS18B20.h      # DS18B20 public API
│   ├── DHT11.c            # DHT11 protocol driver
│   ├── DS18B20.c          # DS18B20 1-Wire protocol driver
│   ├── main.c             # Entry point
│   └── CMakeLists.txt
```

---

## Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) v5.x
- An ESP32 development board
- DHT11 and DS18B20 sensors with appropriate pull-up resistors

---

## Building & Flashing

```bash
# Set up the IDF environment
. $IDF_PATH/export.sh

# Configure target
idf.py set-target esp32

# Build
idf.py build

# Flash and open serial monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

---

## Usage

Once flashed, open a serial monitor at **115200 baud**. You will see:

```
Press 'p' + Enter to send start signal
```

Press `p` followed by Enter to trigger a reading. Both sensors are read in sequence and results are printed:

```
Sending start DHT11 signal...
DHT11 Humidity = 55%
DHT11 Temperature = 28c
DS18B20 Temp: 27.6c
Press 'p' + Enter to send again
```

---

## How It Works

### DHT11
The start signal is sent **twice** per reading. The first read returns a stale cached value from the sensor; the second returns a fresh measurement. The driver waits for the sensor's ACK pulse, then times the high-pulse duration of each of the 40 data bits — pulses longer than 30 µs are decoded as `1`, shorter as `0`. A checksum byte is verified before the result is accepted.

### DS18B20
The driver implements the standard 1-Wire protocol manually using `esp_rom_delay_us` for precise timing. The sequence is:
1. **Reset & presence pulse** — confirms the sensor is on the bus
2. **Skip ROM + Convert T command** — starts temperature conversion
3. **Poll for completion** — reads the bus every 7.5 ms until the sensor signals done (up to 1 second)
4. **Reset & presence pulse** again
5. **Skip ROM + Read Scratchpad** — reads all 9 bytes
6. **CRC-8 check** — validates the received data using the Dallas/Maxim polynomial before computing the final temperature

---

## License

MIT © 2025 Jarif Junaeed