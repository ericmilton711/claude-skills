# ESP32 Weather Station

**Status:** Deployed and working at 192.168.12.240. BLE connected to Homestead Pi. DHT11 sensor wired and reading. OLED removed (caused heap exhaustion). Gift for Rosemary.
**Last Updated:** 2026-05-11

## Hardware

- **Board:** ESP32-D0WD-V3 (Hosyond ESP-WROOM-32, 2-pack)
- **Active unit MAC:** a4:f0:0f:74:11:74
- **Spare unit MAC:** a4:f0:0f:76:70:0c
- **USB Port:** /dev/ttyUSB0 (Linux) or COM15 (Windows) — Silicon Labs CP210x
- **Display:** Amazon Fire HD 8 tablet (to be ordered) showing web dashboard in browser
- **Sensor:** DHT11 (blue square, 3 pins) — wired and reading (GPIO 4)

## Wiring Schematic

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

**Dropped from original plan:** HR202 humidity sensor (redundant with DHT11).
**Removed 2026-05-09:** SSD1306 OLED — caused heap exhaustion (~7.5KB free), killed WiFi/web server.

## Software Stack

- **arduino-cli** 1.5.0 at `~/.local/bin/arduino-cli` (Linux) or `%USERPROFILE%\.local\bin\arduino-cli.exe` (Windows)
- **Board package:** esp32:esp32 3.3.8 (FQBN: `esp32:esp32:esp32`)
- **Partition scheme:** `min_spiffs` (1.9MB app x2 with OTA, 192KB SPIFFS) — sketch uses 93% of partition
- **Libraries:** DHT sensor library, Adafruit Unified Sensor, ArduinoJson 7.4.3, BLE (built-in)
- **esptool** 5.2.0 via pip
- **CRITICAL: Do NOT use PlatformIO.** PlatformIO's framework-arduinoespressif32 3.3.7 breaks BLE connectivity. The ESP32 can connect to WiFi but BLE fails silently (connect() returns false, Pi never receives any commands). Only use arduino-cli with esp32:esp32 3.3.8. Learned the hard way 2026-05-05.

## Sketch Location

- Working copy: `~/esp32-weather/esp32-weather.ino`
- Skills copy: `~/.claude/skills/esp32-weather-station/esp32-weather.ino`

## Features

- **Live clock** — Eastern time (America/New_York) via NTP, 12-hour format, no seconds
- **Current weather** — Open-Meteo API via HTTP (free, no API key), Willow Street PA (lat 39.98, lon -76.28)
  - Temperature (°F), high/low, conditions with emoji, humidity, wind speed
- **Sunrise/Sunset** — 12-hour AM/PM format
- **7-day forecast** — horizontal 7-column grid, day name, conditions with emoji, high/low temps
- **Indoor sensor** — DHT11 temp (°F and °C) + humidity, wired and reading
- **Homestead Pi via BLE** — connects to Pi's GATT server every 30 seconds
  - LED state (ON/OFF), irrigation state (ON/OFF), CPU temp, uptime
  - Shows "Not in Range" when Pi is unreachable
  - BLE task runs on core 0, web server on core 1 (FreeRTOS dual-core)
- **Web dashboard** — served at `http://192.168.12.240/` on port 80, auto-refreshes every 5 seconds
  - Compact 2-column CSS grid layout, fits on phone without scrolling
  - `overflow: hidden` on body to prevent scroll
- **OTA firmware updates** — browse to `/update` to upload .bin files wirelessly
- **Weather updates** every 10 minutes from Open-Meteo
- **Font:** Comfortaa (Google Fonts)
- **Theme:** Dark (#1a1a2e background), red (#e94560) accents, teal (#4ecca3) humidity

## WiFi Hardening (critical — do not change)

- `WiFi.setSleep(false)` — keeps WiFi radio always active (prevents phantom disconnects)
- `WiFi.setTxPower(WIFI_POWER_19_5dBm)` — max transmit power
- `WiFi.setAutoReconnect(true)` — auto-reconnect on drop
- **Static IP set AFTER `WiFi.begin()` connects** — ESP32 Arduino Core 3.3.8 bug ignores `WiFi.config()` before `WiFi.begin()`
- Non-blocking WiFi reconnect: calls `WiFi.disconnect(true)` + `WiFi.begin()` and moves on, checks result on next loop pass (does NOT block in a retry loop)
- `fetchWeather()` uses plain HTTP (not HTTPS) to save ~40KB heap and avoid SSL cert issues
- `fetchWeather()` has 5-second connect and read timeouts to prevent blocking the web server

## BLE Connection (critical — do not change)

The ESP32 connects to the Homestead Pi's BLE GATT server (`ble-homestead.service`):

- **Pi BLE MAC:** `b8:27:eb:f6:24:e9` (built-in BCM, NOT Edimax dongle)
- **Service UUID:** `12345678-1234-5678-1234-56789abcdef0`
- **Command characteristic:** `...def1` (write "status")
- **Response characteristic:** `...def2` (read parsed result)
- **Poll interval:** 30 seconds — **NEVER reduce below 30s** (10s causes WiFi drops due to shared radio)
- **Connect timeout:** 5 seconds
- **Address type:** `BLE_ADDR_TYPE_PUBLIC` (Pi uses public BCM address)
- **Backoff:** 30s → 60s on repeated failures (10s increments), resets to 30s on success
- **Read retry:** retries readValue() up to 3 times (500ms apart) if empty response
- **Miss recovery:** successful connection decrements bleMissCount even if read is empty
- **Manual reset:** `GET /ble-reset` forces immediate retry and clears backoff
- **Client cleanup:** new `BLEClient` per attempt, `delete client` after each attempt (prevents heap leak)
- **try/catch:** around `connect()` to prevent crash loops from exceptions
- **Thread safety:** FreeRTOS mutex protects shared Pi data between BLE task (core 0) and web server (core 1)
- **Known issue:** Pi BCM handles only 1 BLE connection at a time. If another client (laptop, phone) connects to Pi, ESP32 fails until that client disconnects. Restarting `ble-homestead.service` on Pi clears stuck state.
- **piConn bug: FIXED (2026-05-05).** `piConn` now uses `lastSuccessfulBleMillis` -- true if a successful BLE response was received within the last 90 seconds. Dashboard JS also updated: shows Pi data whenever it exists (not just when piConn is true), with status pill showing "Connected" / "Last Data" / "Not in Range".

### Pi BLE Startup Flags (fixed 2026-05-10)

`ble-homestead.py` now sets BOTH `btmgmt connectable on` AND `btmgmt advertising on` at startup. Previously only set `connectable`, causing the Pi to be invisible to the ESP32 after reboots/service restarts. Both flags are required for BLE LE connections.

### BLE Watchdog (added 2026-05-02)

The Pi side has a self-healing watchdog to recover from BlueZ/hci0 wedges (common on Pi 3 where BLE and WiFi share the BCM43455 chip):

1. **Heartbeat:** `ble-homestead.py` writes a timestamp to `/tmp/ble-heartbeat` on every BLE command received
2. **Watchdog script:** `/home/eric/ble-watchdog.sh` — checks heartbeat age, if >5 min stale: power-cycles hci0 and restarts ble-homestead.service
3. **Systemd timer:** `ble-watchdog.timer` — runs the watchdog every 2 minutes

See `homestead-automation` skill for full watchdog script and systemd unit details.

## WiFi

- **SSID:** DIEMILTONHAUS
- **IP:** 192.168.12.240 (static — hardcoded in firmware)

## Remote Access (Tailscale)

- **Remote URL:** `http://100.70.179.60:8240` (from phone or any Tailscale device)
- **Local URL:** `http://192.168.12.240` (on DIEMILTONHAUS WiFi)
- **How:** Tailscale on ThinkCentre (100.70.179.60) + reverse proxy (`weather-proxy.service`) forwarding port 8240 to ESP32 port 80
- **Tailscale account:** ericmilton711@gmail.com
- **Phone:** Galaxy S23 with Tailscale app, IP 100.111.139.83
- **Why not WireGuard:** T-Mobile CGNAT blocks inbound connections

## Flashing

### Via OTA (preferred — wireless)

Browse to `http://192.168.12.240/update`, upload the compiled `.bin` file. The ESP32 reboots automatically after upload.

To get the `.bin` file:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather
```
The `.bin` is at `~/.arduino/sketches/<hash>/esp32-weather.ino.bin` (or use `--output-dir` flag).

### Via USB

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --port COM15 ~/esp32-weather
```

**IMPORTANT:** Must use `PartitionScheme=min_spiffs` — the sketch is 93% of the 1.9MB partition.

This board does NOT always auto-enter bootloader. If upload fails:
1. Hold **BOOT** button
2. While holding BOOT, press and release **EN/RST** button
3. Release BOOT after ~1 second
4. Upload within a few seconds

**NEVER use `esptool erase_flash`** — it wipes NVS, causes boot loops, and loses network state. Just compile and re-upload.

## JSON API

`GET /data` returns:
```json
{
  "tempC": 25.8, "tempF": 78.4, "dhtH": 46.0, "sensor": true,
  "oTemp": "72.4", "oHigh": "74", "oLow": "47",
  "oHum": "51", "oWind": "9.0", "oDesc": "&#9728;&#65039; Clear",
  "sunrise": "5:54 AM", "sunset": "8:08 PM",
  "fc": [
    {"d": "Mon", "c": "&#9925; Partly Cloudy", "h": "64", "l": "46"},
    {"d": "Tue", "c": "&#9925; Partly Cloudy", "h": "61", "l": "40"},
    {"d": "Wed", "c": "&#127783; Rain Showers", "h": "66", "l": "45"},
    {"d": "Thu", "c": "&#127783; Rain Showers", "h": "56", "l": "45"},
    {"d": "Fri", "c": "&#9925; Partly Cloudy", "h": "64", "l": "46"},
    {"d": "Sat", "c": "&#9925; Partly Cloudy", "h": "77", "l": "51"},
    {"d": "Sun", "c": "&#9925; Partly Cloudy", "h": "83", "l": "60"}
  ],
  "piConn": true, "piLed": "OFF", "piWater": "OFF",
  "piTemp": "32.7'C", "piUp": "up 6 hours, 19 minutes",
  "bleMiss": 0
}
```

## Pi BLE Service Fix (2026-05-01)

The Pi's `ble-homestead.py` `find_adapter()` was selecting hci0 (Edimax, broken BLE) instead of hci1 (built-in BCM, working). Fixed by changing `adapters.sort()` to `adapters.sort(reverse=True)` so hci1 is preferred. Note: as of 2026-05-10 only hci0 exists (Edimax removed), and it IS the built-in BCM.

## Build Notes

- Raw string literals in Arduino: avoid `function`, `!`, or other C++ keywords at column 1 inside `R"rawliteral(...)rawliteral"` — the compiler parses them as C++.
- HTML pages use `const char page[] PROGMEM` (not `const char*`) to store in flash. Serving uses chunked `sendContent()` in 1KB chunks.
- BLE library (v3.3.8) uses Arduino `String` type, not `std::string`.
- User must be in `dialout` group for serial access (`/dev/ttyUSB0`).
- String literal concatenation in Arduino: `"foo" + "bar"` fails (both are `const char[]`). Use `String("foo") + "bar"` or break into separate `json +=` statements.

## TODO

- [x] Wire DHT11 sensor (2026-05-09)
- [x] Set static IP — hardcoded 192.168.12.240 (2026-04-30)
- [x] Fix WiFi stability — setSleep(false), max TX power, post-connect config (2026-05-01)
- [x] Fix BLE crash loop — timeout, backoff, client cleanup (2026-05-01)
- [x] Fix Pi BLE adapter selection — hci1 over hci0 (2026-05-01)
- [x] BLE resilience — read retry, lower backoff cap, /ble-reset endpoint, bleMiss debug field (2026-05-01)
- [x] Fix piConn display bug — time-based check + JS shows data when available (2026-05-05)
- [x] Switch to min_spiffs partition — enables OTA wireless updates (2026-05-10)
- [x] Fix web server blocking — HTTP timeouts on fetchWeather(), non-blocking WiFi reconnect (2026-05-10)
- [x] Fix Pi BLE advertising flag — added btmgmt advertising on to ble-homestead.py startup (2026-05-10)
- [x] 7-day forecast — expanded from 2-day, dynamic JS rendering (2026-05-10)
- [x] Compact grid layout — 2-column CSS grid, no scrolling on phone (2026-05-10)
- [x] Horizontal 7-day forecast — 7-column grid instead of stacked rows, tighter spacing (2026-05-11)
- [x] Weather API switched to HTTP — saves heap, avoids SSL issues on ESP32 (2026-05-11)
- [x] DHT11 ground wire reconnected — was unplugged, causing "No Sensor" (2026-05-11)
- [x] Remote access via Tailscale — proxy on ThinkCentre, phone accesses http://100.70.179.60:8240 (2026-05-11)
- [ ] Order Amazon Fire HD 8 tablet (32GB) as dedicated display
- [ ] Order tablet stand for countertop
- [ ] Gift wrap for Rosemary
- [ ] ESP32 needs heatsink + fan + case (runs warm with WiFi + BLE)

---

*Created: 2026-04-26 | Updated: 2026-05-11*
