# Server-Side LED Timer — Chicken Lights

**Last Updated:** 2026-07-24
**Status:** ✅ Live — see `chicken-leds-esp32` skill for current hardware/firmware/schedule details (this doc previously had stale, contradictory copies of that info — removed).

---

## Overview

The chicken LEDs are controlled by a classic ESP32 directly — no Pi required.
The ESP32 syncs time via NTP and manages its own schedule autonomously.
The ThinkCentre can override it anytime via `curl`, which is what this doc covers.

**Firmware:** `~/Documents/chicken-leds-esp32/chicken-leds-esp32.ino` (see `chicken-leds-esp32` skill for full hardware/flash/schedule reference)

---

## Schedule

See `chicken-leds-esp32` skill for the current schedule and firmware details — this doc only covers the ThinkCentre-side override mechanism (`at`/cron + curl). Don't duplicate the schedule table here; it has gone stale before.

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

See `chicken-leds-esp32` skill — board is a classic ESP32 (not S3), OTA at `http://192.168.12.241/update` or USB at `/dev/ttyUSB0`.

---

## Notes

- Boot-state restore: if the ESP32 reboots mid-schedule, it checks the time and restores the correct LED state immediately
- `getLocalTime()` blocks up to 5s per call — after a reset, allow ~15s for NTP sync before the web server comes up
- If the ESP32 loses WiFi, it reconnects automatically in `loop()`
- Pi-hole group 0 block-all (`.*`) applies to unregistered devices — the ESP32 is registered as client_id 15 with no group to bypass this
