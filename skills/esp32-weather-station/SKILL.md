# ESP32-S3 Weather Station

Indoor/outdoor weather station using ESP32-S3 with dual humidity sensors, OLED display, and WiFi web dashboard.

---

## Components

| Component | Role | Pins Used |
|-----------|------|-----------|
| ESP32-S3 | Brain + WiFi web server | — |
| DHT11 | Outdoor temp + humidity | GPIO 4 (DATA) |
| HR202 module | Primary outdoor humidity (analog) | GPIO 1 (AO) |
| SSD1306 OLED 0.96" | Local indoor display (I2C) | GPIO 8 (SDA), GPIO 9 (SCL) |
| Breadboard | Prototyping platform | — |
| USB cable + adapter | Power (5V) | — |
| 10K resistor | DHT11 pull-up (VCC to DATA) | — |

## Wiring Diagram

```
ESP32-S3          Component
─────────         ─────────
3.3V  ──────┬──── DHT11 VCC (pin 1)
            ├──── HR202 module VCC
            └──── OLED VCC

GND   ──────┬──── DHT11 GND (pin 3)
            ├──── HR202 module GND
            └──── OLED GND

GPIO 4 ────────── DHT11 DATA (pin 2)
                  (10K pull-up resistor between VCC and DATA)

GPIO 1 ────────── HR202 module AO
                  (ignore DO pin)

GPIO 8 ────────── OLED SDA

GPIO 9 ────────── OLED SCL
```

## DHT11 Pin Order (facing the grid)
- Pin 1: VCC
- Pin 2: DATA
- Pin 3: GND

## HR202 Module Pins
- VCC, GND, DO (unused), AO (analog humidity reading)

## Architecture

- **Outside:** DHT11 + HR202 in 3D-printed weatherproof vented enclosure, connected by wires back inside
- **Inside:** ESP32-S3 + OLED on breadboard, powered by USB
- **Network:** ESP32 connects to WiFi, serves a live weather dashboard web page accessible from any browser on the LAN
- **Internet weather:** Pulls forecast/conditions from OpenWeatherMap API

## Software Stack

- **IDE:** Arduino IDE 2 (installed via Flatpak on Fedora)
- **Board:** ESP32-S3 (install esp32 board package by Espressif in Arduino Board Manager)
- **Libraries:**
  - DHT sensor library (by Adafruit)
  - Adafruit Unified Sensor
  - Adafruit SSD1306
  - Adafruit GFX
  - WiFi (built-in with ESP32 board package)
  - WebServer (built-in with ESP32 board package)
  - HTTPClient (built-in — for OpenWeatherMap API calls)
  - ArduinoJson (for parsing weather API responses)

## Setup Steps

1. Wire everything on breadboard per diagram above
2. Install Arduino IDE 2 (`flatpak install flathub cc.arduino.IDE2`)
3. In Arduino IDE: File → Preferences → Additional Board URLs → add: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
4. Tools → Board → Board Manager → search "esp32" → install by Espressif
5. Install libraries via Library Manager: DHT, Adafruit SSD1306, Adafruit GFX, Adafruit Unified Sensor, ArduinoJson
6. Select board: ESP32S3 Dev Module
7. Upload sketch

## OpenWeatherMap API

- Free tier: 1000 calls/day
- Sign up at openweathermap.org for API key
- Endpoint: `api.openweathermap.org/data/2.5/weather?q=CITY&appid=KEY&units=imperial`

---

*Created: 2026-04-26*
