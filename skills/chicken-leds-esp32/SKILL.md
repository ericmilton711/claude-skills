# Chicken LEDs — ESP32-S3 Controller

**Last Updated:** 2026-06-16
**Status:** ✅ Firmware flashed and running. Physical wiring pending (fuse short to resolve).

---

## Overview

ESP32-S3 replaces the dead Homestead Pi for controlling the UV/Blue chicken bug lights.
Controls SSR-41FDD #1 via GPIO 16. Manages its own NTP-based schedule autonomously.
ThinkCentre can override via HTTP at any time.

---

## Hardware

| Item | Detail |
|------|--------|
| Controller | ESP32-S3 (spare from inventory) |
| IP | 192.168.12.241 (static) |
| MAC | 30:ed:a0:bb:45:a4 |
| Control pin | GPIO 16 → SSR-41FDD #1 IN+ |
| SSR | SSR-41FDD #1 (3–32VDC input, 6A 60VDC output) |
| LEDs | 5× UV 365nm (CHANZON 3W) + 5× Blue 460nm (1W), parallel at 9.5V |
| LED power | DROK 5A buck: 12V → 9.5V |
| ESP32 power | D-Planet 5A buck: 12V → 5V → ESP32-S3 VIN |
| Battery | DieHard Marine 24M 12V |
| Fuse | 3A inline on battery (+) |

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

### 2 — ESP32-S3 Power (from D-Planet buck)
| From | To |
|------|----|
| D-Planet buck OUT+ (5V) | ESP32-S3 VIN |
| D-Planet buck OUT− | ESP32-S3 GND |

### 3 — SSR Control (low-voltage side)
| From | To |
|------|----|
| ESP32-S3 GPIO 16 | SSR-41FDD #1 IN+ |
| ESP32-S3 GND | SSR-41FDD #1 IN− |

### 4 — LED Circuit (high-voltage side)
| From | To |
|------|----|
| DROK buck OUT+ (9.5V) | SSR-41FDD #1 OUT+ |
| SSR-41FDD #1 OUT− | LEDs (+) |
| LEDs (−) | DROK buck OUT− |

### 5 — Common Ground (all share one point)
Battery (−), D-Planet IN−, DROK IN−, D-Planet OUT−, DROK OUT−, ESP32-S3 GND

---

## Pi-hole

- Client ID: 15
- IP: 192.168.12.241
- Group: none (full DNS access — required for NTP sync via pool.ntp.org)
- **Note:** Pi-hole auto-assigns new clients to group 0 which has block-all `.*`. Must DELETE from client_by_group after registering, or NTP fails and web server never starts.

```bash
# Register and unrestrict
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  "docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"INSERT OR IGNORE INTO client (ip, comment) VALUES ('192.168.12.241', 'Chicken LED ESP32-S3');\" && \
   docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"DELETE FROM client_by_group WHERE client_id=(SELECT id FROM client WHERE ip='192.168.12.241');\" && \
   docker exec pihole pihole reloaddns"
```

---

## HTTP Control

```bash
curl http://192.168.12.241/leds-on
curl http://192.168.12.241/leds-off
curl http://192.168.12.241/status
```

Status response example:
```
leds:  on
ntp:   synced
time:  20:31:14
ip:    192.168.12.241
```

---

## Firmware

**File:** `~/Documents/chicken-leds-esp32/chicken-leds-esp32.ino`
**Board:** `esp32:esp32:esp32s3`
**Port:** `/dev/ttyACM0`

### Flash command
```bash
/home/ericmilton/.local/bin/arduino-cli compile --upload \
  -b esp32:esp32:esp32s3 \
  -p /dev/ttyACM0 \
  /home/ericmilton/Documents/chicken-leds-esp32/
```

### To change the schedule
Edit `applySchedule()` in the `.ino`:
```cpp
// ON 5am–8am, ON 6pm–midnight
bool shouldBeOn = (h >= 5 && h < 8) || (h >= 18);
```

### ESP32-S3 flash notes
- Board select: `ESP32S3 Dev Module` in Arduino IDE
- If it won't enter bootloader: hold **BOOT** then press **RESET**
- After reset, allow ~15s for NTP sync before web server responds
- Port is `/dev/ttyACM0` (native USB on S3)

---

## Troubleshooting

**Fuse pops immediately on connection** — dead short somewhere. Disconnect everything, reconnect one section at a time starting with just the two buck converters on the battery.

**Web server not responding after reset** — check Pi-hole. If the ESP32 was removed and re-added to Pi-hole, it gets auto-assigned to group 0 (block-all). Delete the client_by_group entry (see Pi-hole section above).

**LEDs on when they shouldn't be / off when they should be on** — check NTP sync via `/status`. If not synced, DNS may be blocked again.

---

## Related

- `server-side-led-timer` — ThinkCentre `at`/cron overrides via curl
- `homestead-automation` — original Pi-based setup (solenoid/chicken water still pending new Pi)
- `electronics-inventory` — ESP32-S3 listed as installed at .241
