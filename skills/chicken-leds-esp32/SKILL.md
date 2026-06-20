# Chicken LEDs — ESP32 Controller

**Last Updated:** 2026-06-20
**Status:** ✅ Fully deployed. OTA enabled. Blink endpoint live. Dashboard integrated. Timezone correct.

---

## Overview

Classic ESP32 replaces the dead Homestead Pi for controlling the UV/Blue chicken bug lights.
Controls MY2NJ ice cube relay via GPIO 16 through a 2N2222 transistor driver. Manages its own NTP-based schedule autonomously, with DS3231 RTC as offline fallback.
ThinkCentre or MILTONHAUS Weather Dashboard can override via HTTP at any time.

---

## Hardware

| Item | Detail |
|------|--------|
| Controller | ESP32-D0WD-V3 (classic ESP32, dual core) |
| IP | 192.168.12.241 (static) |
| MAC | a4:f0:0f:76:70:0c |
| Control pin | GPIO 16 → 2N2222 transistor → MY2NJ relay coil |
| Relay | MY2NJ DPDT 12VDC coil, 5A contacts, with socket base (MECCANIXITY 6-pack) |
| Transistor | 2N2222 NPN — drives 12V relay coil from 3.3V ESP32 GPIO |
| Flyback diode | 1N4148 across relay coil terminals |
| Gate resistor | 1kΩ between GPIO 16 and transistor base |
| RTC | DS3231 via I2C — offline timekeeping fallback if WiFi/NTP unavailable |
| LEDs | 5× UV 365nm (CHANZON 3W) + 5× Blue 460nm (1W), parallel at 9.5V |
| LED power | DROK 5A buck: 12V → 9.5V |
| ESP32 power | D-Planet 5A buck: 12V → 5V → ESP32 VIN |
| Battery | DieHard Marine 24M 12V |
| Fuse | 3A inline on battery (+) |
| SSR-41FDD #1 | Replaced by MY2NJ — set aside as spare |

---

## Parts List

| Item | Status |
|------|--------|
| MY2NJ 12VDC relay + socket (MECCANIXITY 6-pack) | Installed |
| 2N2222 NPN transistor (BOJACK 200-pack) | Installed |
| 1N4148 diode | Installed |
| 1kΩ resistor | Installed |
| ESP32 | Installed |
| DS3231 RTC module | Installed |
| INA219 I2C current sensor (HiLetgo 2-pack) | Ordered — for battery voltage + solar charging status on dashboard |
| DROK 5A buck converter | Installed |
| D-Planet 5A buck converter | Installed |
| DieHard Marine 12V battery | Installed |
| 3A inline fuse | Installed |
| 5× CHANZON UV 365nm 3W LEDs | Installed |
| 5× Blue 460nm 1W LEDs | Installed |

---

## Schedule

| Time | State |
|------|-------|
| 12:00am | OFF |
| 5:00am | ON |
| 8:00am | OFF |
| 6:00pm | ON |

---

## Wiring Connections

### 1 — Battery to Buck Converters
| From | To |
|------|----|
| Battery (+) | 3A fuse (one end) |
| 3A fuse (other end) | D-Planet buck IN+ |
| 3A fuse (other end) | DROK buck IN+ |
| Battery (−) | D-Planet buck IN− |
| Battery (−) | DROK buck IN− |

### 2 — ESP32 Power (from D-Planet buck)
| From | To |
|------|----|
| D-Planet buck OUT+ (5V) | ESP32 VIN |
| D-Planet buck OUT− | ESP32 GND |

### 3 — Relay Driver Circuit (2N2222 transistor)
| From | To |
|------|----|
| ESP32 GPIO 16 | 1kΩ resistor (one end) |
| 1kΩ resistor (other end) | 2N2222 Base |
| 12V (from battery/fuse) | MY2NJ socket pin 14 (coil +) |
| MY2NJ socket pin 13 (coil −) | 2N2222 Collector |
| 2N2222 Emitter | GND |
| 1N4148 anode | MY2NJ socket pin 13 (coil −) |
| 1N4148 cathode | MY2NJ socket pin 14 (coil +) |

### 4 — LED Circuit (through relay contacts)
| From | To |
|------|----|
| DROK buck OUT+ (9.5V) | MY2NJ socket pin 9 (NO) |
| MY2NJ socket pin 5 (COM) | LEDs (+) |
| LEDs (−) | DROK buck OUT− |

### 5 — Common Ground (all share one point)
Battery (−), D-Planet IN−, DROK IN−, D-Planet OUT−, DROK OUT−, ESP32 GND, 2N2222 Emitter

---

## Pi-hole

- Client ID: 15
- IP: 192.168.12.241
- Group: none (full DNS access — required for NTP sync via pool.ntp.org)
- **Note:** Pi-hole auto-assigns new clients to group 0 which has block-all `.*`. Must DELETE from client_by_group after registering, or NTP fails and web server never starts.

```bash
# Register and unrestrict
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  "docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"INSERT OR IGNORE INTO client (ip, comment) VALUES ('192.168.12.241', 'Chicken LED ESP32');\" && \
   docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"DELETE FROM client_id=(SELECT id FROM client WHERE ip='192.168.12.241');\" && \
   docker exec pihole pihole reloaddns"
```

---

## HTTP Control

```bash
curl http://192.168.12.241/leds-on
curl http://192.168.12.241/leds-off
curl http://192.168.12.241/status
curl http://192.168.12.241/blink          # 2-minute blink (0.5s on/off), restores schedule after
curl http://192.168.12.241/blink?sec=30   # custom duration in seconds (1–600)
curl http://192.168.12.241/blink-stop     # cancel blink early
```

Status response example:
```
leds:  on
clock: ntp
time:  20:31:14
ip:    192.168.12.241
```

`clock:` will show `ntp`, `rtc`, or `none` depending on time source.

### Why /blink instead of rapid on/off curls
Sending 240 curl requests in 2 minutes crashes the ESP32 web server. `/blink` runs the entire sequence as a FreeRTOS task internally — one HTTP request, no crashes.

---

## MILTONHAUS Weather Dashboard

The dashboard at `http://192.168.12.240/` shows live Chicken LED status and has ON/OFF buttons in the stats strip. It polls `/chicken-status` every 5 seconds via proxy endpoints on the weather ESP32. Control works from any browser on the LAN.

---

## Firmware

**File:** `~/Documents/chicken-leds-esp32/chicken-leds-esp32.ino`
**Board:** `esp32:esp32:esp32`
**Port:** `/dev/ttyUSB0` (USB) or OTA at `http://192.168.12.241/update`

### Flash via OTA (preferred when signal is good — NOT reliable from the garage)
**Note:** The garage has weak WiFi (~9KB/s). OTA uploads time out or drop at ~30%. Use USB when physically in the garage. OTA works fine from inside the house if you ever move the board.


```bash
/home/ericmilton/.local/bin/arduino-cli compile -b esp32:esp32:esp32 \
  --output-dir ~/Documents/chicken-leds-esp32/build \
  ~/Documents/chicken-leds-esp32/
curl --max-time 90 \
  --form "update=@/home/ericmilton/Documents/chicken-leds-esp32/build/chicken-leds-esp32.ino.bin;type=application/octet-stream" \
  http://192.168.12.241/update
```

### Flash via USB
```bash
/home/ericmilton/.local/bin/arduino-cli compile -b esp32:esp32:esp32 \
  --output-dir ~/Documents/chicken-leds-esp32/build \
  ~/Documents/chicken-leds-esp32/ && \
/home/ericmilton/.local/bin/arduino-cli upload -b esp32:esp32:esp32 \
  -p /dev/ttyUSB0 ~/Documents/chicken-leds-esp32/
```

### To change the schedule
Edit `applySchedule()` in the `.ino`:
```cpp
// ON 5am–8am, ON 6pm–midnight
bool shouldBeOn = (h >= 5 && h < 8) || (h >= 18);
```

---

## Troubleshooting

### Relay clicks off on reboot — normal
Boot sequence sets GPIO LOW first, then syncs NTP, then calls `applySchedule()`. LEDs restore to correct state within ~15s.

### Web server not responding after reset
Check Pi-hole. If the ESP32 was removed and re-added, it gets auto-assigned to group 0 (block-all). Delete the client_by_group entry (see Pi-hole section above).

### LEDs on/off at wrong times
Check NTP sync via `/status`. If `clock: none`, DNS may be blocked again.

**Timezone bug (fixed 2026-06-20):** WiFi reconnect loop was using `configTime(0,0,NTP_SERVER)` (UTC) instead of `configTzTime(TIMEZONE, NTP_SERVER)` (Eastern). Fixed in current firmware — `/status` should show Eastern time (e.g. `19:xx:xx` at 7pm, not `23:xx:xx`).

### Fuse Pops — Short Isolation Procedure

Disconnect everything from the battery. Reconnect one section at a time with the 3A fuse in place.

| Step | Connect | Fuse pops = |
|------|---------|-------------|
| 1 | Battery + fuse only, nothing on output | Fuse holder or wiring itself is shorted |
| 2 | D-Planet buck IN+ to fuse, IN- to battery neg (nothing on output) | Bad D-Planet buck converter |
| 3 | DROK buck IN+ to fuse, IN- to battery neg (nothing on output) | Bad DROK buck converter |
| 4 | D-Planet output to ESP32 VIN and GND | ESP32 pulling too much or wiring short on that run |
| 5 | Relay driver: GPIO 16 wiring, 1kΩ resistor, 2N2222, MY2NJ coil between 12V and collector | Short in relay coil circuit (check 1N4148 diode polarity, coil wires not reversed or touching) |
| 6 | LEDs: DROK output through relay NO/COM to LEDs, trigger with `curl http://192.168.12.241/leds-on` | Short in LED string or relay contacts |

---

## Related

- `server-side-led-timer` — ThinkCentre `at`/cron overrides via curl
- `esp32-weather-station` — Weather dashboard that shows LED status + controls
- `homestead-automation` — original Pi-based setup (solenoid/chicken water still pending new Pi)
- `electronics-inventory` — ESP32 listed as installed at .241
