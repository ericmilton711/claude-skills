# Server-Side LED Timer — Chicken Lights

**Last Updated:** 2026-06-16
**Status:** ✅ Live — ESP32-S3 at 192.168.12.241

---

## Overview

The chicken LEDs are controlled by an ESP32-S3 directly — no Pi required.
The ESP32 syncs time via NTP and manages its own schedule autonomously.
The ThinkCentre can override it anytime via `curl`.

**Hardware:** ESP32-S3 → GPIO 16 → SSR-41FDD #1 → LEDs (5× UV + 5× Blue at 9.5V)
**Firmware:** `~/Documents/chicken-leds-esp32/chicken-leds-esp32.ino`
**Pi-hole:** client_id 15, no group (full DNS access)
**Pi-hole MAC:** 30:ed:a0:bb:45:a4

---

## Schedule

| Time | State |
|------|-------|
| 12:00am | OFF |
| 5:00am | ON |
| 8:00am | OFF |
| 6:00pm | ON |
| (repeat) | |

---

## HTTP Control (from anywhere on MILTONHAUS)

```bash
curl http://192.168.12.241/leds-on
curl http://192.168.12.241/leds-off
curl http://192.168.12.241/status
```

---

## ThinkCentre One-Shot Override (at a specific time)

```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'echo "curl -s http://192.168.12.241/leds-off" | at 11pm'
```

Check scheduled jobs:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'atq'
```

Cancel a job (replace `<ID>`):
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'atrm <ID>'
```

---

## Reflashing the Firmware

ESP32-S3 must be plugged in via USB (`/dev/ttyACM0`):

```bash
/home/ericmilton/.local/bin/arduino-cli compile --upload \
  -b esp32:esp32:esp32s3 \
  -p /dev/ttyACM0 \
  /home/ericmilton/Documents/chicken-leds-esp32/
```

To change the schedule, edit `applySchedule()` in the `.ino` file:
```cpp
// Current schedule: ON 5am-8am, ON 6pm-midnight
bool shouldBeOn = (h >= 5 && h < 8) || (h >= 18);
```

---

## Notes

- Boot-state restore: if the ESP32 reboots mid-schedule, it checks the time and restores the correct LED state immediately
- `getLocalTime()` blocks up to 5s per call — after a reset, allow ~15s for NTP sync before the web server comes up
- If the ESP32 loses WiFi, it reconnects automatically in `loop()`
- Pi-hole group 0 block-all (`.*`) applies to unregistered devices — the ESP32 is registered as client_id 15 with no group to bypass this
