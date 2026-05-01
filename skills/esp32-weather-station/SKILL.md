# ESP32 Weather Station

**Status:** Deployed and working at 192.168.12.240. BLE connected to Homestead Pi. DHT11 sensor not yet wired. Gift for Rosemary.
**Last Updated:** 2026-05-01

## Hardware

- **Board:** ESP32-D0WD-V3 (Hosyond ESP-WROOM-32, 2-pack)
- **Active unit MAC:** a4:f0:0f:74:11:74
- **Spare unit MAC:** a4:f0:0f:76:70:0c
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
- **Partition scheme:** `huge_app` (3MB app, no OTA partition, no SPIFFS)
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
- **Web dashboard** — served at `http://192.168.12.240/` on port 80, auto-refreshes every 5 seconds
- **OTA firmware updates** — browse to `/update` to upload .bin files wirelessly
- **Weather updates** every 10 minutes from Open-Meteo
- **Font:** Comfortaa (Google Fonts)
- **Theme:** Dark (#1a1a2e background), red (#e94560) accents, teal (#4ecca3) humidity

## WiFi Hardening (critical — do not change)

- `WiFi.setSleep(false)` — keeps WiFi radio always active (prevents phantom disconnects)
- `WiFi.setTxPower(WIFI_POWER_19_5dBm)` — max transmit power
- `WiFi.setAutoReconnect(true)` — auto-reconnect on drop
- **Static IP set AFTER `WiFi.begin()` connects** — ESP32 Arduino Core 3.3.8 bug ignores `WiFi.config()` before `WiFi.begin()`
- Reconnect logic in `loop()` checks every 30s, also checks RSSI == 0 for phantom WiFi detection
- `WiFi.disconnect(true)` before reconnect to fully reset the radio

## BLE Connection (critical — do not change)

The ESP32 connects to the Homestead Pi's BLE GATT server (`ble-homestead.service`):

- **Pi BLE MAC:** `b8:27:eb:f6:24:e9` (built-in BCM, NOT Edimax dongle)
- **Service UUID:** `12345678-1234-5678-1234-56789abcdef0`
- **Command characteristic:** `...def1` (write "status")
- **Response characteristic:** `...def2` (read parsed result)
- **Poll interval:** 30 seconds — **NEVER reduce below 30s** (10s causes WiFi drops due to shared radio)
- **Connect timeout:** 5 seconds
- **Backoff:** 30s → 120s on repeated failures, resets to 30s on success
- **Client cleanup:** new `BLEClient` per attempt, `delete client` after each attempt (prevents heap leak)
- **try/catch:** around `connect()` to prevent crash loops from exceptions
- **Thread safety:** FreeRTOS mutex protects shared Pi data between BLE task (core 0) and web server (core 1)

## WiFi

- **SSID:** DIEMILTONHAUS
- **IP:** 192.168.12.240 (static — hardcoded in firmware)

## Flashing

### Via USB (preferred)

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app ~/esp32-weather
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=huge_app --port /dev/ttyUSB0 ~/esp32-weather
```

**IMPORTANT:** Must use `PartitionScheme=huge_app` — the sketch is 58% of the 3MB huge_app partition and does NOT fit in the default 1.3MB partition.

This board does NOT always auto-enter bootloader. If upload fails:
1. Hold **BOOT** button
2. While holding BOOT, press and release **EN/RST** button
3. Release BOOT after ~1 second
4. Upload within a few seconds

**NEVER use `esptool erase_flash`** — it wipes NVS, causes boot loops, and loses network state. Just compile and re-upload.

### Via OTA (wireless — only when WiFi is stable)

1. Compile: `arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app --output-dir /tmp/esp32-build ~/esp32-weather`
2. Upload: `curl -F "update=@/tmp/esp32-build/esp32-weather.ino.bin" http://192.168.12.240/update`
3. Or browse to `http://192.168.12.240/update` and upload the .bin file

**Note:** OTA requires sustained WiFi for the ~1.8MB transfer. If WiFi is flaky, use USB instead.

## JSON API

`GET /data` returns:
```json
{
  "tempC": 0.0, "tempF": 0.0, "dhtH": 0.0, "sensor": false,
  "oTemp": "45.9", "oHigh": "64", "oLow": "39",
  "oHum": "81", "oWind": "5.2", "oDesc": "&#9728;&#65039; Clear",
  "sunrise": "6:04 AM", "sunset": "7:59 PM",
  "f1Day": "Sat", "f1Desc": "&#9925; Partly Cloudy", "f1Hi": "58", "f1Lo": "43",
  "f2Day": "Sun", "f2Desc": "&#9925; Partly Cloudy", "f2Hi": "58", "f2Lo": "35",
  "piConn": true, "piLed": "OFF", "piWater": "OFF",
  "piTemp": "32.7'C", "piUp": "up 6 hours, 19 minutes"
}
```

## Pi BLE Service Fix (2026-05-01)

The Pi's `ble-homestead.py` `find_adapter()` was selecting hci0 (Edimax, broken BLE) instead of hci1 (built-in BCM, working). Fixed by changing `adapters.sort()` to `adapters.sort(reverse=True)` so hci1 is preferred.

## Build Notes

- Raw string literals in Arduino: avoid `function`, `!`, or other C++ keywords at column 1 inside `R"rawliteral(...)rawliteral"` — the compiler parses them as C++.
- HTML pages use `const char page[] PROGMEM` (not `const char*`) to store in flash. Serving uses chunked `sendContent()` in 1KB chunks.
- BLE library (v3.3.8) uses Arduino `String` type, not `std::string`.
- User must be in `dialout` group for serial access (`/dev/ttyUSB0`).

## TODO

- [ ] Wire DHT11 sensor
- [x] Set static IP — hardcoded 192.168.12.240 (2026-04-30)
- [x] Fix WiFi stability — setSleep(false), max TX power, post-connect config (2026-05-01)
- [x] Fix BLE crash loop — timeout, backoff, client cleanup (2026-05-01)
- [x] Fix Pi BLE adapter selection — hci1 over hci0 (2026-05-01)
- [ ] Order Amazon Fire HD 8 tablet (32GB) as dedicated display
- [ ] Order tablet stand for countertop
- [ ] Gift wrap for Rosemary
- [ ] ESP32 needs heatsink + fan + case (runs warm with WiFi + BLE)

---

*Created: 2026-04-26 | Updated: 2026-05-01*
