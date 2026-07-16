# ESP32 Weather Station

> **⚠ CURRENT AS OF 2026-07-16** — WiFi reconnect watchdog fix deployed (self-heals total-network-dropout crashes). See "What Changed 2026-07-16" below. Family calendar integration complete. See "What Changed 2026-07-03" for architecture. Dark "Forest" palette via taste-skill (redesigned 2026-07-02).

**Status:** Deployed at 192.168.12.240. Dark "Forest" theme (redesigned 2026-07-02). NWS weather (real station obs). DHT11 reading. Hero temp layout (no boxed card) + glass side panel (indoor gauge, conditions, Chicken Lights segmented toggle) + 3x2 kid chip grid. All emoji replaced with inline-SVG icons. Family calendar (themiltonfam@gmail.com) live via ThinkCentre poller on port 8182.
**Last Updated:** 2026-07-16

---

## What Changed 2026-07-16 (DEPLOYED — flash: 58% / RAM: 16%)

**Fixed: ESP32 going totally unreachable (not even pingable) requiring a physical power cycle — recurring issue, root cause found**

### Problem
Symptom: dashboard shows stale/frozen data (e.g. "Mostly Clear" hours after skies turned cloudy/smoky), and the device doesn't respond to `ping` or port 80 at all — a harder failure than the 2026-06-27 "pingable but web server dead" case.

Root cause: in `loop()`'s WiFi-reconnect block, `wifiFailCount` was incremented on every failed reconnect attempt but **never checked against anything** — the code just kept looping `WiFi.disconnect()` + `WiFi.begin()` every 30s forever. If the WiFi/lwIP driver gets into a wedged state (a known ESP32 issue, not necessarily a router problem), a software-level disconnect/reconnect can't fix it — only a full chip reboot resets the driver. Since `loop()` itself never hangs during this (it's still calling `esp_task_wdt_reset()` every pass), the hardware task watchdog stays satisfied and never intervenes either. Nothing escalates → device spins on a broken WiFi stack indefinitely until someone physically power-cycles it.

### Fix
After 5 consecutive failed reconnect attempts (~2.5 min), force `ESP.restart()` instead of retrying forever:
```cpp
} else {
  wifiFailCount++;
  Serial.printf("WiFi reconnect failed (%d)\n", wifiFailCount);
  if (wifiFailCount >= 5) {
    Serial.println("WiFi reconnect failed 5x in a row - forcing reboot");
    delay(100);
    ESP.restart();
  }
}
```

### Lesson learned
A failure counter that's incremented but never read is worse than useless — it looks like error handling exists but does nothing. When `loop()` stays alive and keeps petting the hardware watchdog, a wedged WiFi/lwIP driver can persist forever with no automatic recovery unless something explicitly escalates to a full restart. Verified by confirming the current condition (`/data` → `oDesc`) matched the real live NWS station observation immediately after the fix, both before flashing (device unreachable) and after (matched reality).

### Open question: remote power-cycle for harder crashes
Discussed adding a ThinkCentre-side health check that could power-cycle the ESP32 via a smart plug for crash modes this software fix can't self-heal (e.g. a genuine hard hang/panic loop that never gets back to running `loop()`). Eric has a Feit Electric (Tuya-based) WiFi smart plug already, currently used for basement grow lights (see `home-assistant-plant-monitoring` skill) — undecided whether to repurpose/share it or get a dedicated one for the ESP32. Deferred as of 2026-07-16, no action taken yet.

---

## What Changed 2026-07-02 (DEPLOYED — flash: 58% / RAM: 16%)

**Full dashboard redesign via `taste-skill` collaboration — dark "Forest" palette, hero layout, icon-based weather glyphs**

### What changed
Worked with Eric using the `taste-skill` design-taste skill (an anti-slop *frontend* skill explicitly scoped to "landing pages, portfolios, and redesigns — not dashboards," adapted anyway since only the typography/color/materiality judgment transfers, not the landing-page hard rules). Iterated on a standalone static mockup (`~/esp32-weather/taste-skill-mockup.html`, copy saved alongside this skill) first, before touching the real firmware, specifically so there was an easy side-by-side to react to before any risk to the live device. **Note:** the older `preview.html` workflow (see "Dashboard Redesign 2026-06-04" below) is now stale — this round iterated directly against the redesign mockup instead, then ported straight into `page[]`.

- **Palette:** warm earth-tone (`#d2c6a5` bg) → dark "Forest" theme (deep green/charcoal radial-gradient bg, `#d8b45c` amber accent, `#5fce8b` green for humidity/gauge fill).
- **Hero layout:** outside temp is no longer boxed in a `.card` — a large amber icon-chip + big temperature number IS the layout. `hero-temp` scales up to `10rem` max via `clamp(5rem, min(20vh,25vw), 10rem)` — needed 3 rounds of bumping (started at 5.6rem) because `vh`-based sizing tuned for the tablet's 800px-tall screen reads much smaller in a normal desktop browser window.
- **Icons:** every emoji (🏠 ☀️ ⛅ ⏱️ ✎) replaced with inline-SVG icons. Client-side `condIcon(desc)` keyword-matcher (`ICONS.sun/cloud/rain/snow/fog/thunder`) mirrors the server's existing `nwsDescToEmoji()` logic — the server-side C++ is untouched and still sends emoji in the JSON payload, the client just ignores it and re-derives its own icon from the description text.
- **Layout consolidation:** indoor gauge + humidity/wind/sunrise/sunset + Chicken Lights now live in one translucent "glass" side panel instead of scattered boxes. Chicken Lights is a real two-button segmented ON/OFF toggle (`#ledOnBtn`/`#ledOffBtn`, `.active` class) replacing the old tiny ~10px pill buttons + separate `#chickenStat` text.
- **Kids:** 3×2 chip grid with initial avatars, replacing the 6-row stacked list. All 6 kids (Benedict, Evangelina, Gianna, Patrick, Clementine, Adelaide) — easy to accidentally drop names when iterating on a grid, double-check the count matches.
- **"Today — Family Calendar" section added** (`.hero-events`), placeholder only ("Not connected yet"). Sits below the temp block with a large deliberate gap (`margin-top: clamp(90px,16vh,170px)`) per Eric's repeated "lower" feedback. Real data is a TODO — see below.
- **Forecast strip deliberately does NOT highlight "today."** `fetchForecast()` (untouched C++) consumes today's high/low into `oHigh`/`oLow` before populating `forecast[]`, so `forecast[0]` is actually *tomorrow*. The mockup had assumed index 0 = today for illustration purposes only — porting that assumption into the real UI would have mislabeled every tile.

### Lessons learned
- **Arduino raw-string-literal gotcha, confirmed:** a JS line starting with the literal word `function` (e.g. `    function condIcon(desc){...}`) inside `R"rawliteral(...)"` fails with `'function' does not name a type` — the Arduino/ctags auto-prototyping preprocessor scans for function-looking declarations even inside string literals. Other `  function foo(){}` lines elsewhere in the same file compiled fine, so it isn't purely about indentation — safest fix is rewriting as `var foo = function(...){...};` so no line starts with the bare keyword.
- **A second `.ino` file left in the sketch folder gets compiled together with the real one.** `arduino-cli compile` treats every `.ino` in a directory as one combined sketch — a `esp32-weather.classic-backup.ino` backup caused `redefinition of 'void setup()'` etc. until moved to a sibling folder (`~/esp32-weather-backups/`), outside the sketch directory.
- **Rollback workflow that worked:** before flashing, copy the working `.ino` to `~/esp32-weather-backups/esp32-weather.classic-backup.ino`. Revert = recompile that backup + OTA-flash (~90s), no code archaeology needed.
- **Device dropped completely off the network once after an OTA flash** — unreachable even to `ping`, not just "web server hung." Recovered cleanly after a physical power cycle. Root cause unconfirmed (no WiFi/watchdog C++ was touched, only `page[]`), so it may recur — get it on USB serial to check for a crash loop before assuming new page content is the cause.

---

## What Changed 2026-07-03 (DEPLOYED — flash: 58% / RAM: 16%)

**Family calendar integration — DONE**

Google Calendar events for `themiltonfam@gmail.com` now display in the "Today — Family Calendar" section on the dashboard.

### Architecture
- **ThinkCentre poller:** Flask app at `/home/milton/family-calendar/app.py`, port 8182. Fetches the Google Calendar iCal feed every 10 minutes, parses today's events, serves JSON at `GET /calendar`.
- **Systemd service:** `family-calendar.service` — enabled, starts on boot, auto-restarts.
- **iCal URL:** Secret address from Google Calendar settings (themiltonfam@gmail.com primary calendar). No OAuth needed.
- **Dashboard JS:** Fetches `http://192.168.12.136:8182/calendar` every 5 minutes, populates `#calEvents` with event time + title rows.
- **Libraries installed on ThinkCentre:** `icalendar`, `recurring-ical-events`

### ThinkCentre Dashboard Kiosk
Firefox auto-launches the weather dashboard in fullscreen kiosk mode on every boot:
- **Autostart:** `~/.config/autostart/miltonhaus-dashboard.desktop` — waits 10s, dismisses GNOME overview via ydotool, then launches `firefox --kiosk http://192.168.12.240`
- **GNOME config:** Single workspace (dynamic-workspaces=false, num-workspaces=1), `no-overview@fthx` extension enabled (skips Activities screen on login)
- **ydotool:** Installed + enabled on boot for remote keystroke simulation. Passwordless sudo via `/etc/sudoers.d/ydotool`.
- **Weekly reboot:** Saturday 3 AM via root crontab: `0 3 * * 6 /usr/bin/killall firefox; /bin/sleep 3; /sbin/reboot`. Kills Firefox first for clean shutdown (avoids BIOS warning from force reboot, avoids Firefox inhibitor blocking normal reboot).
- **To exit kiosk remotely:** `sudo ydotool key 56:1 62:1 62:0 56:0` (Alt+F4)
- **To launch Firefox on ThinkCentre monitor remotely:** `WAYLAND_DISPLAY=wayland-0 XDG_RUNTIME_DIR=/run/user/1000 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus firefox --kiosk http://192.168.12.240`

### Notes
- Events refresh daily automatically (poller always queries today's date).
- Shows "No events today" when the calendar is empty, "Calendar offline" if the ThinkCentre is unreachable.
- Two empty "Family" sub-calendars also exist in the Google account but have no events. The primary `themiltonfam@gmail.com` calendar is the one with data.

---

## What Changed 2026-07-01 (DEPLOYED — flash: 87% / RAM: 16%)

**Fixed: dashboard was unusable on phones — content clipped instead of reflowing**

### Problem
The layout is a fixed two-column CSS grid (`.grid { grid-template-columns: 3fr 2fr }`) built for the Fire tablet's wide landscape screen, inside a no-scroll kiosk body (`overflow: hidden`, fixed `100dvh` height). The only mobile media query (`max-width: 460px`) kept the same 2-column split and only tweaked letter-spacing. On a phone in portrait, both columns (and the nested `.left-panel` sub-grid: weather card + indoor gauge side by side) got squeezed into a narrow width — text and cards were clipped at the edge instead of wrapping, and the 5-column stats strip (Humidity/Wind/Sunrise/Sunset/Chicken) did the same.

### Fix
Replaced the old 460px query with a `max-width: 700px` query that switches to a stacked, scrollable mobile layout:
- `body` gets `height: auto; overflow-y: auto` instead of the fixed-height no-scroll kiosk mode
- `.grid` and `.left-panel` switch from `display: grid` to `display: flex; flex-direction: column` — weather card, indoor gauge, forecast, and stats stack vertically instead of squeezing side by side
- `.kid-card` gets `flex: none; min-height: 56px` so cards don't collapse now that `.right-panel` isn't height-constrained
- `.stats` drops to a 3-column grid so the 5 stat tiles wrap to two rows instead of clipping

Tablet/kiosk layout (>700px) is untouched — this only affects narrow phone widths.

### Lesson learned
The single `@media (max-width: 460px)` query looked like mobile support existed, but it never actually changed the grid shape — worth checking what a media query *actually does*, not just that one exists, when a "responsive" layout still breaks on a real device.

---

## What Changed 2026-06-30 v2 (DEPLOYED — flash: 58% / RAM: 16%)

**Daily Chores are now checkboxes — kids can tap to mark them done, resets every Sunday night**

### What changed
In the kid detail overlay, `showKid()` now renders each chore as a checkbox row (`.chore-item` is a `<label>` wrapping an `<input type="checkbox">` + `<span>`) instead of a plain `<div>`. Checked chores get struck through (`.chore-done span`). Checkboxes are 24×24px for easy tapping on the Fire tablet.

### Where the state lives — no ThinkCentre code change needed
This was built entirely client-side, without touching `kids-dashboard/app.py`, because `/kids/save` has no schema validation — it just `json.dump()`s whatever JSON it's given. So the dashboard JS itself now manages two extra fields that ride along in the same `kids.json`:
- Each kid object gets a `choreDone` object: `{"<chore text>": true/false}`, keyed by the chore's exact text (not index, so edits/reorders in `/kids-admin` don't misalign it — though renaming a chore does lose its checked state, which is an acceptable tradeoff)
- Top-level `weekStart`: the ISO date (`YYYY-MM-DD`) of the most recent Monday, used to detect when a week has rolled over

### Weekly reset logic (`mondayKey()` in the dashboard JS)
On every `loadKids()` poll (every 2 minutes), the dashboard computes the current week's Monday date. If it doesn't match the `weekStart` stored in `kids.json`, every kid's `choreDone` is cleared and the new `weekStart` is POSTed back via `/kids/save` — so completed chores stay checked all week and clear automatically Sunday night → Monday morning.

### Toggling
`toggleChore(kidIndex, choreIndex, checkboxEl)` looks up the chore text from `kidsData` (not from a DOM attribute, to avoid HTML-escaping issues with chore text), flips `choreDone[chore]`, and POSTs the whole `kidsData` array + current `weekStart` back to `/kids/save`.

### Known tradeoff
If a parent edits the chores list via `/kids-admin` (served by `app.py`, which only sends `{name, chores, schedule}` on save), that save overwrites the kids array and drops `choreDone`/`weekStart` for everyone — so an admin edit resets all check-state for that day. Acceptable since chore edits are infrequent; a proper fix would move this tracking server-side into `app.py` (see `project_thinkcentre_ssh_exec_hang` memory — deferred because SSH exec was down on the ThinkCentre when this was built).

---

## What Changed 2026-06-30 v1 (DEPLOYED — flash: 58% / RAM: 16%)

**Fixed: dashboard always showed chicken LEDs as OFF, even when they were ON**

### Problem
`handleChickenStatus()` parsed the chicken ESP32's `/status` text by checking for the literal substring `"leds:  on"` (exactly 2 spaces). The chicken firmware (`chicken-leds-esp32.ino`) actually formats that line with 5 spaces of padding (`"leds:     %s\n"`), so the substring never matched. Result: `on` was always `false` — the dashboard tile and buttons showed OFF/gray regardless of the real LED state.

### Fix
`handleChickenStatus()` now finds the `leds:` line, then checks that line for `"off"`/`"on"` instead of matching an exact padded string — immune to whitespace changes on either firmware:
```cpp
int lp = raw.indexOf("leds:");
String ledsLine = lp >= 0 ? raw.substring(lp, raw.indexOf('\n', lp)) : "";
bool on = ledsLine.indexOf("off") < 0 && ledsLine.indexOf("on") >= 0;
```

### Lesson learned
Don't parse a status line with an exact-whitespace substring match across two independently-edited firmware files — a padding tweak on one side silently breaks the other. Verified by comparing `curl http://192.168.12.241/status` directly against `curl http://192.168.12.240/chicken-status` and confirming they agree after the fix.

---

## What Changed 2026-06-27 (DEPLOYED — flash: 58% / RAM: 16%)

**Core 0 weatherTask watchdog fix — prevents web server deadlock**

### Problem
The `weatherTask` on core 0 had no watchdog. When a `WiFiClientSecure` TLS handshake hung (known ESP32 bug where timeouts don't apply to TLS), it blocked the lwIP TCP stack. Result: ESP32 was pingable but port 80 never responded. The `loop()` watchdog on core 1 couldn't help because `loop()` kept running fine, it just never received TCP connections.

OTA flashing was impossible (web server dead). Required USB serial flash to fix.

### Fix
- `weatherTask` now calls `esp_task_wdt_add(NULL)` on entry to register with the hardware watchdog
- `esp_task_wdt_reset()` called before and after `fetchWeather()` in the task loop
- `nwsFetch()` calls `esp_task_wdt_reset()` before and after `http.GET()` (the call most likely to hang)
- `fetchSunriseSunset()` calls `esp_task_wdt_reset()` at entry

If any HTTPS fetch hangs for >30 seconds, the watchdog resets the chip instead of deadlocking the web server.

### Lesson learned
OTA is useless when the web server is the thing that's broken. Keep a micro-USB data cable accessible near the ESP32 for emergencies. The CP210x shows up as COM15 on Eric's Windows laptop.

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
  - Comfortaa font, dark "Forest" theme (redesigned 2026-07-02 — was warm earth-tone `#d2c6a5`/`#8b5e3c`; now deep green/charcoal radial gradient bg, `#d8b45c` amber accent)
  - All condition/UI icons are inline SVG (`condIcon()` in JS) — no emoji anywhere in the rendered page, though the server's JSON payload still carries emoji internally (client ignores it)
  - Responsive — Fire HD 10 landscape (1280×800) full layout, phone ≤600px adapts
- **OTA firmware updates** — browse to `/update`
- **Weather updates** every 10 minutes
- **~~World Cup scores~~** — REMOVED 2026-06-16

## Dashboard Layout (current — redesigned 2026-07-02, hero + glass rail)

```
┌───────────────────────────────────────────┬──────────────┐
│ MILTONHAUS WEATHER      [Hourly] [clock]  │              │  ← top bar
├─────────────────────────────────────┬─────┤   Indoor     │
│  (icon) 78°F                        │     │  (glass)     │
│         Clear · H:84° / L:62°       │     ├──────────────┤
│                                      │     │  Conditions  │
│  Today — Family Calendar            │     │  (glass):    │
│  (placeholder, not connected)       │     │  Humidity    │
│                                      │     │  Wind        │
│  [forecast tiles, bottom-anchored]  │     │  Sunrise     │
│                                      │     │  Sunset      │
│                                      │     │  Chicken     │
│                                      │     │  Lights ⏻    │
├──────────────────────────────────────┴─────┼──────────────┤
│                                             │ B  E  G      │
│                                             │ P  C  A      │  ← kid chip grid
└─────────────────────────────────────────────┴──────────────┘
              HERO (2.1fr)                       RAIL (1.1fr)
```

Outer grid: `.body { grid-template-columns: 2.1fr 1.1fr }` (was `.grid { 3fr 2fr }` pre-redesign)
Hero (`.hero`): flex column — `.hero-loc` → `.hero-row` (icon-chip + big temp) → `.hero-meta` (condition + H/L) → `.hero-events` (calendar placeholder) → `.fc-strip` (forecast, `flex:1`, bottom-anchored via `align-items:flex-end`)
Rail (`.rail`): flex column — Indoor `.glass` panel (ring gauge) → Conditions `.glass` panel (stat-lines + Chicken Lights segmented toggle) → `.kid-strip` (3×2 CSS grid of `.kid-chip`, each with an initial avatar)

**Kid chips:** avatar initial + first name. Tap → `showKid(i)` → overlay with chores + schedule fetched from ThinkCentre (unchanged from pre-redesign).

**Chicken Lights toggle:** `#ledOnBtn` / `#ledOffBtn`, `.active` class shows current state; `#chiOffline` tag appears if the chicken ESP32 doesn't respond. Replaced the old `#chickenStat` text + tiny pill-button pattern.

**Overlays (full-screen, z-index 100, restyled dark but same IDs/JS):**
- Hourly — tap the Hourly pill in the top bar, `showHourly()` / `closeHourly()`
- Day detail — tap any forecast tile, `showDayDetail(day)` / `closeDayDetail()`
- Kid detail — tap any kid chip, `showKid(i)` / `closeKid()` — data from `http://192.168.12.136:8181/kids`

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
- `esp_task_wdt_reset()` called in `nwsFetch()` and `fetchSunriseSunset()` — weatherTask pets its own watchdog during long HTTPS fetches
- `fetchWeather()` removed from WiFi reconnect path — weatherTask handles it on next cycle

**Result:** Dashboard reachable in ~3s. Shows `--` briefly while first fetch completes on core 0.

## Hardware Watchdog 2026-06-04 + 2026-06-27 (DEPLOYED)

**Symptom (2026-06-04):** ESP32 went completely unreachable (no ping), only manual power-cycle recovered it.

**Root cause:** Soft heap-watchdog in `loop()` never ran when a `WiFiClientSecure` TLS handshake blocked past its timeout — a known ESP32 core bug.

**Fix (2026-06-04):** `esp_task_wdt` hardware watchdog (30s). Resets the chip if `loop()` doesn't pet it. Fed at top of `loop()`, and inside the OTA upload callback (uploads take >30s).

**Symptom (2026-06-27):** ESP32 pingable but web server on port 80 completely unresponsive. Hung TLS handshake on core 0 blocked the lwIP TCP stack, preventing core 1's web server from accepting connections.

**Fix (2026-06-27):** `weatherTask` on core 0 now registered with the same hardware watchdog via `esp_task_wdt_add(NULL)`. Watchdog fed before/after HTTPS fetches. Both cores are now protected.

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
# Linux
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --port /dev/ttyUSB0 ~/esp32-weather

# Windows (COM15 on Eric's laptop — CP210x driver)
arduino-cli.exe compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs ~/esp32-weather
arduino-cli.exe upload --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs --port COM15 ~/esp32-weather
```

If upload fails: hold BOOT, press+release EN/RST, release BOOT, upload within a few seconds.

**Important:** OTA is useless when the web server itself is hung. Keep a micro-USB data cable accessible near the ESP32.

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

*Created: 2026-04-26 | Updated: 2026-07-16*
