# ESP32 Weather Station

> **⚠ CURRENT AS OF 2026-06-20 (v2)** — Kids section added as right-side column. See "What Changed 2026-06-20 v2" first if you're picking this up mid-project.

**Status:** Deployed at 192.168.12.240. NWS weather (real station obs). DHT11 reading. Kids column on right. Chicken LED control in stats strip.
**Last Updated:** 2026-06-20

---

## What Changed 2026-06-20 v2 (DEPLOYED — flash: 87% / RAM: 16%)

**Kids section added — right-side column with chores/schedule editor on ThinkCentre**

### Dashboard layout change
- Grid changed from 3-row single-column to **two-column**: left panel (all weather content) + right panel (kids)
- `grid-template-columns: 3fr 2fr` — weather takes 60%, kids take 40%
- Left panel is a nested grid with the same 3-row weather layout as before
- Right panel: 6 tappable cards stacked vertically — Benedict, Evangelina, Gianna, Patrick, Clementine, Adelaide
- Tapping any kid card opens a full-screen overlay showing their **Daily Chores** and **Work Schedule**
- Overlay has an "Edit" link that opens the admin page in a new tab

### Kids data server (ThinkCentre .136, port 8181)
- Flask app at `/home/milton/kids-dashboard/app.py`
- Runs as `kids-dashboard.service` (systemd, auto-starts on boot)
- Data stored in `/home/milton/kids-dashboard/kids.json`
- **Admin page:** `http://192.168.12.136:8181/kids-admin` — edit chores (one per line) + schedule for each kid, hit Save. No Claude Code needed.
- **API:** `GET /kids` returns JSON, `POST /kids/save` saves JSON (CORS enabled)
- ESP32 dashboard fetches kids data every 2 minutes from `http://192.168.12.136:8181/kids`

### To update kids' chores/schedule
Open `http://192.168.12.136:8181/kids-admin` from any browser on the network. Edit and Save. Done.

---

## What Changed 2026-06-20 v1 (DEPLOYED — flash: 57% / RAM: 16%)

**Chicken LED control card added to stats strip**
- Stats strip expanded from 4 to 5 columns — Humidity, Wind, Sunrise, Sunset, **Chicken LEDs**
- Shows live ON/OFF status (green=ON, brown=OFF, gray=Offline), polls every 5 seconds
- ON and OFF buttons control the chicken LEDs at 192.168.12.241 directly from the dashboard
- Three new proxy endpoints added to weather ESP32 firmware:
  - `GET /chicken-status` → returns `{"on": true/false, "ok": true/false}`
  - `GET /chicken-on` → sends leds-on to chicken ESP32
  - `GET /chicken-off` → sends leds-off to chicken ESP32
- Proxy uses plain HTTP `HTTPClient` (not secure) on LAN — 1.5s timeout so a dead chicken ESP32 doesn't slow the dashboard

---

## What Changed 2026-06-16 (DEPLOYED — flash: 57% / RAM: 16%)

**1. World Cup scores strip REMOVED**
- Removed the entire bottom row from the dashboard grid (`"scores scores"` area gone)
- Deleted all CSS (`.scores-strip`, `.scores-overlay`, `.sc-*`, `.match-*`)
- Deleted HTML for the strip card and the full-screen overlay
- Deleted all JS: `WC_URL`, `parseMTC()`, `fetchTodayScores()`, `showScores()`, `closeScores()`, and the 60s interval
- Grid is now 3 rows: `"main gauge" / "fc fc" / "stats stats"` — stats row fills the space naturally

**2. Forecast day tiles are now tappable (day detail overlay)**
- Tap any day tile (Wed, Thu, Fri…) on any device — phone, tablet, desktop
- Opens a full-screen overlay (same warm earth-tone style as the hourly overlay)
- Fetches NWS `gridpoints/CTP/128,27/forecast` client-side, filters to periods matching the tapped day (first 3 chars of period name)
- Shows both daytime and overnight cards for that day, each with:
  - Period name (e.g. "Wednesday" / "Wednesday Night")
  - Temperature in big text
  - Full NWS detailed forecast sentence
  - Wind direction + speed, precipitation probability, humidity
- Tap **Back** to return to dashboard
- Tiles have hover darkening and `scale(0.96)` active feedback

---

## What Changed 2026-06-14 (DEPLOYED)

**FreeRTOS Weather Task** — boot time to reachable dashboard: ~3s instead of ~57s.
`fetchWeather()` moved to a FreeRTOS task on core 0; web server starts immediately on core 1.
`WxData` struct + `wxMutex` protect shared weather data. See full details below.

---

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
- **Partition scheme:** `min_spiffs` (1.9MB app x2 with OTA, 192KB SPIFFS)
- **Libraries:** DHT sensor library, Adafruit Unified Sensor, ArduinoJson 7.4.3, WiFiClientSecure (built-in)
- **esptool** 5.2.0 via pip
- **CRITICAL: Do NOT use PlatformIO.** Only use arduino-cli with esp32:esp32 3.3.8. PlatformIO 3.3.7 breaks BLE silently.

## Sketch Location

- Working copy: `~/esp32-weather/esp32-weather.ino`
- Skills copy: `~/.claude/skills/esp32-weather-station/esp32-weather.ino`

## Features (current)

- **Live clock** — Eastern time (America/New_York) via NTP, 12-hour format, no seconds
- **Current weather** — NWS (api.weather.gov) via HTTPS, station KLNS (Lancaster Airport), grid CTP/128,27
  - Temperature (°F), high/low, conditions with emoji, humidity, wind speed
- **Sunrise/Sunset** — sunrise-sunset.org API via HTTPS, UTC→Eastern conversion, 12-hour AM/PM format
- **7-day forecast strip** — NWS forecast periods, day name, conditions with emoji, high/low temps; **tap any tile to open day detail overlay**
- **Day detail overlay** — tap a forecast tile → full-screen overlay with NWS detailed text, wind, precip %, humidity for that day's periods (NEW 2026-06-16)
- **Hourly forecast overlay** — tap Conditions card → next 24 hours, fetched client-side from NWS
- **Indoor sensor** — DHT11 temp (°F and °C) + humidity, wired and reading
- **Web dashboard** — served at `http://192.168.12.240/` on port 80, auto-refreshes every 5 seconds
  - Comfortaa font, warm earth-tone theme (#d2c6a5 background, #8b5e3c accents)
  - Responsive — Fire HD 10 landscape (1280×800) full layout, phone ≤600px adapts
- **OTA firmware updates** — browse to `/update`
- **Weather updates** every 10 minutes
- **~~World Cup scores~~** — REMOVED 2026-06-16

## Dashboard Layout (current — two-column)

```
┌─────────────────────────────────────┬──────────────┐
│ 🏠 MILTONHAUS Weather      [clock]  │              │  ← top bar
├──────────────────┬──────────────────┤              │
│  Weather (2fr)   │ Indoor dial (1fr)│  Benedict    │
├──────────────────┴──────────────────┤  Evangelina  │
│       6-Day Forecast (tappable)     │  Gianna      │
├─────────────────────────────────────┤  Patrick     │
│  Humidity  Wind  Sunrise  Sunset    │  Clementine  │
│  Chicken LEDs                       │  Adelaide    │
└─────────────────────────────────────┴──────────────┘
          LEFT (3fr)                     RIGHT (2fr)
```

Outer grid: `grid-template-columns: 3fr 2fr`, `grid-template-areas: "left right"`
Left panel (`.left-panel`): nested grid `2fr 1fr`, rows `1.3fr 1fr 0.72fr`, areas `"main gauge" "fc fc" "stats stats"`
Right panel (`.right-panel`): flex column, 6 `.kid-card` divs (each `flex: 1`)

**Kid cards:** name + "Tap to view" subtitle. Tap → `showKid(i)` → overlay with chores + schedule fetched from ThinkCentre.

**Overlays (full-screen, z-index 100):**
- Hourly — tap Conditions card, `showHourly()` / `closeHourly()`
- Day detail — tap any forecast tile, `showDayDetail(day)` / `closeDayDetail()`
- Kid detail — tap any kid card, `showKid(i)` / `closeKid()` — data from `http://192.168.12.136:8181/kids`

## WiFi Hardening (do not change)

- `WiFi.setSleep(false)` — keeps WiFi radio always active
- `WiFi.setTxPower(WIFI_POWER_19_5dBm)` — max transmit power
- `WiFi.setAutoReconnect(true)` — auto-reconnect on drop
- **Static IP set AFTER `WiFi.begin()` connects** — Core 3.3.8 bug ignores pre-connect `WiFi.config()`
- Non-blocking WiFi reconnect in `loop()` — calls disconnect+begin and checks on next pass
- NWS API calls: 10s connect + read timeouts; sunrise-sunset.org: 5s timeouts

## FreeRTOS Weather Task 2026-06-14 (DEPLOYED)

**Problem:** Boot-to-reachable took ~57 seconds — `setup()` ran all 3 HTTPS fetches before `server.begin()`.

**Fix:** `weatherTask` pinned to core 0 with 10240-byte stack. Web server starts on core 1 immediately after WiFi+NTP (~3s). Task fetches immediately on boot, then `vTaskDelay(600000ms)` between cycles.

**Key design:**
- `WxData` struct holds all weather fields; replaces individual globals
- `wxMutex` protects `wx` — fetch task builds local `WxData fresh`, swaps atomically (~1-2ms hold)
- `handleData()` takes mutex to read safely
- `esp_task_wdt_reset()` removed from `fetchWeather()` — loop() pets the watchdog freely
- `fetchWeather()` removed from WiFi reconnect path — weatherTask handles it on next cycle

**Result:** Dashboard reachable in ~3s. Shows `--` briefly while first fetch completes on core 0.

## Hardware Watchdog 2026-06-04 (DEPLOYED)

**Symptom:** ESP32 went completely unreachable (no ping), only manual power-cycle recovered it.

**Root cause:** Soft heap-watchdog in `loop()` never ran when a `WiFiClientSecure` TLS handshake blocked past its timeout — a known ESP32 core bug.

**Fix:** `esp_task_wdt` hardware watchdog (30s). Resets the chip if `loop()` doesn't pet it. Fed at top of `loop()`, and inside the OTA upload callback (uploads take >30s).

## Crash / Reliability Fixes 2026-06-04 (DEPLOYED)

1. **ArduinoJson filters** — `nwsFetch()` passes filters; only ~4-6 needed fields parsed. Prevents heap fragmentation from 50-100KB NWS payloads.
2. **NWS timeouts** 10s → 8s.
3. **Heap watchdog reboots** at <10000 bytes free (guarded by `!otaInProgress`).

## Dashboard Redesign 2026-06-04 (DEPLOYED)

Layout matches a Home Assistant wall-tablet photo (layout only — NOT dark theme). Warm earth-tone palette kept (Rosemary's gift).

**Preview workflow:** `~/esp32-weather/preview.html` — open with `setsid xdg-open` before flashing. ALWAYS sync preview changes into the sketch before compiling. ALWAYS verify the live page after flashing.

**Firefox headless screenshot** (close Firefox first — headless fails with Wayland instance running):
```bash
firefox --headless --new-instance --profile /tmp/ffprof --window-size=1280,800 \
  --screenshot /tmp/shot.png "http://192.168.12.240/"
```

## WiFi

- **SSID:** DIEMILTONHAUS
- **IP:** 192.168.12.240 (static — hardcoded in firmware)

## Remote Access (Tailscale)

- **Remote URL:** `http://100.70.179.60:8240`
- **Local URL:** `http://192.168.12.240`
- **How:** Tailscale on ThinkCentre (100.70.179.60) + `weather-proxy.service` forwarding port 8240 → ESP32:80
- **Phone:** Galaxy S23 with Tailscale, IP 100.111.139.83
- **Why not WireGuard:** T-Mobile CGNAT blocks inbound connections

## Flashing

### Via OTA (preferred)

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --output-dir /tmp/esp32-build ~/esp32-weather
curl --max-time 90 --form "update=@/tmp/esp32-build/esp32-weather.ino.bin;type=application/octet-stream" http://192.168.12.240/update
```

### Via USB (when board is physically connected)

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --port /dev/ttyUSB0 ~/esp32-weather
```

If upload fails: hold BOOT, press+release EN/RST, release BOOT, upload within a few seconds.

**NEVER use `esptool erase_flash`** — wipes NVS, causes boot loops, loses network state.

## JSON API

`GET /data` returns:
```json
{
  "tempC": 25.8, "tempF": 78.4, "dhtH": 46.0, "sensor": true,
  "oTemp": "72.4", "oHigh": "74", "oLow": "47",
  "oHum": "51", "oWind": "9.0", "oDesc": "&#9728;&#65039; Clear",
  "sunrise": "5:54 AM", "sunset": "8:08 PM",
  "forecast": [
    {"day": "Wed", "desc": "&#127783;&#65039; Chance Rain Showers", "hi": "82", "lo": "64"},
    {"day": "Thu", "desc": "&#9889; Chance Showers And Thunderstorms", "hi": "90", "lo": "65"}
  ]
}
```
Note: array key is `forecast`, item keys are `day`/`desc`/`hi`/`lo` (not `fc`/`d`/`c`/`h`/`l`).

## Build Notes

- Raw string literals in Arduino: avoid `function`, `!`, or other C++ keywords at column 1 inside `R"rawliteral(...)"`.
- HTML pages use `const char page[] PROGMEM`. Served in 1KB chunks via `sendContent()`.
- String literal concatenation: `"foo" + "bar"` fails. Use `String("foo") + "bar"` or `+=`.
- User must be in `dialout` group for `/dev/ttyUSB0` access.

## TODO

- [x] Wire DHT11 sensor (2026-05-09)
- [x] Set static IP — hardcoded 192.168.12.240 (2026-04-30)
- [x] Fix WiFi stability — setSleep(false), max TX power, post-connect config (2026-05-01)
- [x] Switch to min_spiffs partition — enables OTA (2026-05-10)
- [x] 7-day forecast (2026-05-10)
- [x] Remote access via Tailscale (2026-05-11)
- [x] Switched to NWS (api.weather.gov) — real station observations (2026-05-27)
- [x] Sunrise/sunset via sunrise-sunset.org (2026-05-27)
- [x] Hourly forecast overlay (2026-05-27)
- [x] BLE code removed (2026-05-27)
- [x] Crash/reliability fixes — ArduinoJson filters, heap watchdog (2026-06-04)
- [x] Hardware Task Watchdog (2026-06-04)
- [x] Dashboard redesign — Fire HD 10 landscape layout (2026-06-04)
- [x] FreeRTOS weather task — 3s boot time (2026-06-14)
- [x] World Cup scores REMOVED (2026-06-16)
- [x] Forecast day tiles → tap for day detail overlay (2026-06-16)
- [x] Kids column — 6 tappable cards on right, chores/schedule editor on ThinkCentre (2026-06-20)
- [ ] Order Amazon Fire HD 8 tablet (32GB) as dedicated display
- [ ] Order tablet stand for countertop
- [ ] Gift wrap for Rosemary
- [ ] ESP32 needs heatsink + fan + case (runs warm with WiFi)
- [ ] **Yard camera** — ESP32-CAM pointed at yard/garden/chicken coop
- [ ] **Outdoor sensor** — second ESP32 with DHT11/DHT22 outside, sends readings via HTTP

---

*Created: 2026-04-26 | Updated: 2026-06-20*
