# Chicken LEDs — ESP32-S3 Controller

**Last Updated:** 2026-06-17
**Status:** ✅ Firmware flashed and running. Physical wiring pending (fuse short to resolve). New relay on order.

---

## Overview

ESP32-S3 replaces the dead Homestead Pi for controlling the UV/Blue chicken bug lights.
Controls MY2NJ ice cube relay via GPIO 16 through a 2N2222 transistor driver. Manages its own NTP-based schedule autonomously.
ThinkCentre can override via HTTP at any time.

---

## Hardware

| Item | Detail |
|------|--------|
| Controller | ESP32-S3 (spare from inventory) |
| IP | 192.168.12.241 (static) |
| MAC | 30:ed:a0:bb:45:a4 |
| Control pin | GPIO 16 → 2N2222 transistor → MY2NJ relay coil |
| Relay | MY2NJ DPDT 12VDC coil, 5A contacts, with socket base (MECCANIXITY 6-pack) |
| Transistor | 2N2222 NPN — drives 12V relay coil from 3.3V ESP32 GPIO |
| Flyback diode | 1N4148 across relay coil terminals |
| Gate resistor | 1kΩ between GPIO 16 and transistor base |
| LEDs | 5× UV 365nm (CHANZON 3W) + 5× Blue 460nm (1W), parallel at 9.5V |
| LED power | DROK 5A buck: 12V → 9.5V |
| ESP32 power | D-Planet 5A buck: 12V → 5V → ESP32-S3 VIN |
| Battery | DieHard Marine 24M 12V |
| Fuse | 3A inline on battery (+) |
| SSR-41FDD #1 | Replaced by MY2NJ — set aside as spare |

---

## Parts List

| Item | Status |
|------|--------|
| MY2NJ 12VDC relay + socket (MECCANIXITY 6-pack) | Ordered |
| 2N2222 NPN transistor (BOJACK 200-pack) | Ordered |
| 1N4148 diode | On hand |
| 1kΩ resistor | On hand |
| ESP32-S3 | On hand / installed |
| DROK 5A buck converter | On hand / installed |
| D-Planet 5A buck converter | On hand / installed |
| DieHard Marine 12V battery | On hand / installed |
| 3A inline fuse | On hand |
| 5× CHANZON UV 365nm 3W LEDs | On hand / installed |
| 5× Blue 460nm 1W LEDs | On hand / installed |
| Hookup wire | On hand |

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

### 3 — Relay Driver Circuit (2N2222 transistor)
| From | To |
|------|----|
| ESP32-S3 GPIO 16 | 1kΩ resistor (one end) |
| 1kΩ resistor (other end) | 2N2222 Base |
| 12V (from battery/fuse) | MY2NJ coil (+) |
| MY2NJ coil (−) | 2N2222 Collector |
| 2N2222 Emitter | GND |
| 1N4148 anode | MY2NJ coil (−) |
| 1N4148 cathode | MY2NJ coil (+) |

### 4 — LED Circuit (through relay contacts)
| From | To |
|------|----|
| DROK buck OUT+ (9.5V) | MY2NJ NO contact (line in) |
| MY2NJ COM contact (line out) | LEDs (+) |
| LEDs (−) | DROK buck OUT− |

### 5 — Common Ground (all share one point)
Battery (−), D-Planet IN−, DROK IN−, D-Planet OUT−, DROK OUT−, ESP32-S3 GND, 2N2222 Emitter

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

### Fuse Pops — Short Isolation Procedure

Disconnect everything from the battery. Reconnect one section at a time with the 3A fuse in place. If the fuse pops at any step, that section has the short. Use a multimeter on continuity between positive and negative of each section before connecting it.

| Step | Connect | Fuse pops = |
|------|---------|-------------|
| 1 | Battery + fuse only, nothing on output | Fuse holder or wiring itself is shorted |
| 2 | D-Planet buck IN+ to fuse, IN- to battery neg (nothing on output) | Bad D-Planet buck converter |
| 3 | DROK buck IN+ to fuse, IN- to battery neg (nothing on output) | Bad DROK buck converter |
| 4 | D-Planet output to ESP32 VIN and GND | ESP32 pulling too much or wiring short on that run |
| 5 | Relay driver: GPIO 16 wiring, 1kΩ resistor, 2N2222, MY2NJ coil between 12V and collector | Short in relay coil circuit (check 1N4148 diode polarity, coil wires not reversed or touching) |
| 6 | LEDs: DROK output through relay NO/COM to LEDs, trigger with `curl http://192.168.12.241/leds-on` | Short in LED string or relay contacts |

### Web server not responding after reset

Check Pi-hole. If the ESP32 was removed and re-added, it gets auto-assigned to group 0 (block-all). Delete the client_by_group entry (see Pi-hole section above).

### LEDs on/off at wrong times

Check NTP sync via `/status`. If not synced, DNS may be blocked again.

---

## Related

- `server-side-led-timer` — ThinkCentre `at`/cron overrides via curl
- `homestead-automation` — original Pi-based setup (solenoid/chicken water still pending new Pi)
- `electronics-inventory` — ESP32-S3 listed as installed at .241
