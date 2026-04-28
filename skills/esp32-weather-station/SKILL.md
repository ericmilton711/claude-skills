# ESP32 Weather Station

**Status:** Flashed and working. BLE connected to Homestead Pi. DHT11 sensor not yet wired. Gift for Rosemary.
**Last Updated:** 2026-04-28

## Hardware

- **Board:** ESP32-D0WD-V3 (Hosyond ESP-WROOM-32, 2-pack)
- **MAC:** a4:f0:0f:76:70:0c
- **USB Port:** /dev/ttyUSB0 (Silicon Labs CP210x)
- **Display:** Amazon Fire HD 8 tablet (to be ordered) showing web dashboard in browser
- **Sensor:** DHT11 (blue square, 3 pins) — not yet wired

## Wiring Schematic (TODO — not yet wired)

```
ESP-WROOM-32       Component
────────────       ─────────
3.3V  ──────┬──── DHT11 VCC (left pin)
            │
GND   ──────┬──── DHT11 GND (right pin)
            │
GPIO 4  ───────── DHT11 DATA (middle pin)
                  (10K pull-up resistor between VCC and DATA)
```

### DHT11 Pin Order (facing the grid)
- Pin 1 (left): VCC
- Pin 2 (middle): DATA
- Pin 3 (right): GND

**Dropped from original plan:** HR202 humidity sensor (redundant with DHT11), SSD1306 OLED (using tablet instead).

## Software Stack

- **arduino-cli** 1.4.1 at `~/.local/bin/arduino-cli`
- **Board package:** esp32:esp32 3.3.8 (FQBN: `esp32:esp32:esp32`)
- **Partition scheme:** `min_spiffs` (1.9MB app + 1.9MB OTA + 128KB SPIFFS)
- **Libraries:** DHT sensor library, Adafruit Unified Sensor, ArduinoJson 7.4.3, BLE (built-in)
- **esptool** 5.2.0 via pip

## Sketch Location

- Working copy: `~/esp32-weather/esp32-weather.ino`
- Skills copy: `~/.claude/skills/esp32-weather-station/esp32-weather.ino`

## Features

- **Live clock** — Eastern time (America/New_York) via NTP, 12-hour format, no seconds
- **Current weather** — Open-Meteo API (free, no API key), Willow Street PA (lat 39.98, lon -76.28)
  - Temperature (°F), high/low, conditions with emoji, humidity, wind speed
- **Sunrise/Sunset** — 12-hour AM/PM format
- **2-day forecast** — day name, conditions, high/low temps
- **Indoor sensor** — DHT11 temp (°F and °C) + humidity (shows "Sensor Not Connected" until wired)
- **Homestead Pi via BLE** — connects to Pi's GATT server every 30 seconds
  - LED state (ON/OFF), irrigation state (ON/OFF), CPU temp, uptime
  - Shows "Not in Range" when Pi is unreachable
  - BLE task runs on core 0, web server on core 1 (FreeRTOS dual-core)
- **Web dashboard** — served at `http://<ESP32_IP>/` on port 80, auto-refreshes every 5 seconds
- **OTA firmware updates** — browse to `/update` to upload .bin files wirelessly
- **Weather updates** every 10 minutes from Open-Meteo
- **Font:** Comfortaa (Google Fonts)
- **Theme:** Dark (#1a1a2e background), red (#e94560) accents, teal (#4ecca3) humidity

## BLE Connection

The ESP32 connects to the Homestead Pi's BLE GATT server (`ble-homestead.service`):

- **Service UUID:** `12345678-1234-5678-1234-56789abcdef0`
- **Command characteristic:** `...def1` (write "status")
- **Response characteristic:** `...def2` (read parsed result)
- **Pi BLE name:** `homestead`
- **Poll interval:** 30 seconds
- **Thread safety:** FreeRTOS mutex protects shared Pi data between BLE task (core 0) and web server (core 1)

## WiFi

- **SSID:** DIEMILTONHAUS
- **IP:** 192.168.12.240 (DHCP — should set static before gifting)

## Flashing

### Via USB (first time or recovery)

This board does NOT always auto-enter bootloader. If upload fails, try:

1. Hold **BOOT** button
2. While holding BOOT, press and release **EN/RST** button
3. Release BOOT after ~1 second
4. Upload within a few seconds

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --port /dev/ttyUSB0 ~/esp32-weather
```

**IMPORTANT:** Must use `PartitionScheme=min_spiffs` — the sketch is 93% of the 1.9MB min_spiffs partition and does NOT fit in the default 1.3MB partition.

### Via OTA (wireless, after first flash)

1. Compile: `arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather`
2. Open browser: `http://192.168.12.240/update`
3. Upload the .bin file from `~/esp32-weather/build/esp32.esp32.esp32/esp32-weather.ino.bin`
4. ESP32 reboots automatically with new firmware

## JSON API

`GET /data` returns:
```json
{
  "tempC": 0.0, "tempF": 0.0, "dhtH": 0.0, "sensor": false,
  "oTemp": "47.3", "oHigh": "59", "oLow": "44",
  "oHum": "63", "oWind": "1.9", "oDesc": "&#9925; Partly Cloudy",
  "sunrise": "6:08 AM", "sunset": "7:56 PM",
  "f1Day": "Wed", "f1Desc": "&#127783;&#65039; Rain", "f1Hi": "60", "f1Lo": "46",
  "f2Day": "Thu", "f2Desc": "&#127782;&#65039; Drizzle", "f2Hi": "58", "f2Lo": "42",
  "piConn": true, "piLed": "OFF", "piWater": "OFF",
  "piTemp": "30.6'C", "piUp": "up 5 hours, 43 minutes"
}
```

## Architecture

- **ESP32** connects to WiFi, pulls weather from Open-Meteo, reads DHT11 sensor, connects to Pi via BLE, serves web dashboard
- **Homestead Pi** runs BLE GATT server exposing status/control commands
- **Any browser on the network** can view the dashboard at the ESP32's IP
- **Amazon Fire HD 8 tablet** will be the dedicated always-on display (gift for Rosemary)
- No Raspberry Pi or intermediate device needed — tablet runs browser directly to ESP32

## Build Notes

- Raw string literals in Arduino: avoid `function`, `!`, or other C++ keywords at column 1 inside `R"rawliteral(...)rawliteral"` — the compiler parses them as C++. Use `void(function(){...}())` or `var x = function(){}` instead.
- ESP32 program storage is 93% used (1.84MB of 1.96MB with min_spiffs partition).
- BLE library (v3.3.8) uses Arduino `String` type, not `std::string` — use `String()` for writeValue/readValue.
- BLE scan + connect cycle takes ~5-10 seconds. Runs on core 0 via FreeRTOS task to avoid blocking web server on core 1.
- Blink test sketch at `~/esp32-blink/esp32-blink.ino` (GPIO 2 = onboard blue LED).
- User must be in `dialout` group for serial access (`/dev/ttyUSB0`).

## TODO

- [ ] Wire DHT11 sensor
- [ ] Set static IP (DHCP reservation or hardcode in sketch)
- [ ] Order Amazon Fire HD 8 tablet (32GB) as dedicated display
- [ ] Order tablet stand for countertop
- [ ] Gift wrap for Rosemary

---

*Created: 2026-04-26 | Updated: 2026-04-28*
