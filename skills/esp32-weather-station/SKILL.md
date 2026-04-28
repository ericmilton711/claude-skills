# ESP32 Weather Station

**Status:** Flashed and working. DHT11 sensor not yet wired. Gift for Rosemary.
**Last Updated:** 2026-04-27

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
- **Libraries:** DHT sensor library, Adafruit Unified Sensor, ArduinoJson 7.4.3
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
- **Web dashboard** — served at `http://<ESP32_IP>/` on port 80, auto-refreshes every 5 seconds
- **Weather updates** every 10 minutes from Open-Meteo
- **Font:** Comfortaa (Google Fonts)
- **Theme:** Dark (#1a1a2e background), red (#e94560) accents, teal (#4ecca3) humidity

## WiFi

- **SSID:** DIEMILTONHAUS
- **IP:** 192.168.12.240 (DHCP — should set static before gifting)

## Flashing

This board does NOT auto-enter bootloader. Every upload requires:

1. Hold **BOOT** button
2. While holding BOOT, press and release **EN/RST** button
3. Release BOOT after ~1 second
4. Upload within a few seconds:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 ~/esp32-weather
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 ~/esp32-weather
```

## JSON API

`GET /data` returns:
```json
{
  "tempC": 0.0, "tempF": 0.0, "dhtH": 0.0, "sensor": false,
  "oTemp": "57.6", "oHigh": "71", "oLow": "37",
  "oHum": "55", "oWind": "10.8", "oDesc": "☁️ Partly Cloudy",
  "sunrise": "6:09 AM", "sunset": "7:55 PM",
  "f1Day": "Tue", "f1Desc": "☁️ Partly Cloudy", "f1Hi": "61", "f1Lo": "43",
  "f2Day": "Wed", "f2Desc": "🌧️ Rain Showers", "f2Hi": "64", "f2Lo": "45"
}
```

## Architecture

- **ESP32** connects to WiFi, pulls weather from Open-Meteo, reads DHT11 sensor, serves web dashboard
- **Any browser on the network** can view the dashboard at the ESP32's IP
- **Amazon Fire HD 8 tablet** will be the dedicated always-on display (gift for Rosemary)
- No Raspberry Pi or intermediate device needed — tablet runs browser directly to ESP32

## Build Notes

- Raw string literals in Arduino: avoid `function`, `!`, or other C++ keywords at column 1 inside `R"rawliteral(...)rawliteral"` — the compiler parses them as C++. Use `void(function(){...}())` or `var x = function(){}` instead.
- ESP32 program storage is 84% used (1.1MB of 1.3MB) — limited room for more features.
- Blink test sketch at `~/esp32-blink/esp32-blink.ino` (GPIO 2 = onboard blue LED).
- User must be in `dialout` group for serial access (`/dev/ttyUSB0`).

## TODO

- [ ] Wire DHT11 sensor
- [ ] Set static IP (DHCP reservation or hardcode in sketch)
- [ ] Order Amazon Fire HD 8 tablet (32GB) as dedicated display
- [ ] Order tablet stand for countertop
- [ ] Gift wrap for Rosemary

---

*Created: 2026-04-26 | Updated: 2026-04-27*
