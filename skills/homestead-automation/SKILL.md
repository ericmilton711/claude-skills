# Homestead Automation — Raspberry Pi 3 A+ Controller

## Project Overview
Raspberry Pi 3 A+ controlling two things in the chicken run:
1. **Bug attraction lights** — UV (365nm) + Blue (460nm) LEDs to attract insects for chickens
2. **Chicken water** — 12V solenoid valve off a rain barrel → chicken waterer

**Plan change (2026-04-29):** Drip line PSI too high for rain barrel gravity. Garden drip line now hooks to outdoor spout instead. Solenoid controls chicken water only.

Pi runs completely **offline** (no WiFi needed in the field). Cron fires the Python script on schedule.

---

## Hardware

| Item | Detail |
|------|--------|
| Pi 3 A+ | MILTONRP3, booting from 64GB microSD |
| Battery | DR.PREPARE 12V 20Ah LiFePO4 |
| Solar panel | ECO-WORTHY 25W |
| Charge controller | Renogy Wanderer 10A PWM (LiFePO4) |
| Buck converters | D-Planet 5A DC-DC adjustable ×2 (12V→5V for Pi, 12V→3.2V for LEDs) — 4-pack, 2 spares |
| SSRs | SSR-41FDD ×2 (input 3–32VDC, output 6A 60VDC) |
| UV LEDs | 5× CHANZON 3W 365nm (UV) |
| Blue LEDs | 5× 1W 460–465nm (blue) |
| Solenoid | U.S. Solid ¼" NC 12V solenoid valve |
| Enclosure | 3D printed PETG (weatherproof) |
| Sealant | DAP High Temp RTV Silicone |

**LED wiring**: All 10 chips in **parallel** at ~3.2V. LEDs buried in aluminum bar in ground for heatsinking.

---

## GPIO Wiring (BCM numbering) — Updated 2026-04-24

| Pi Pin | BCM | Connected to |
|--------|-----|-------------|
| Pin 1  | 3.3V | DS3231 RTC VCC |
| Pin 2/4 | 5V | Two cooling fans (always on) |
| Pin 3  | GPIO 2 (SDA) | DS3231 RTC SDA |
| Pin 5  | GPIO 3 (SCL) | DS3231 RTC SCL |
| Pin 6/9 | GND | Fan ground + SSR input (−) |
| Pin 11 | GPIO 17 | SSR #1 input (+) → LED circuit |
| Pin 13 | GPIO 27 | SSR #2 input (+) → solenoid |
| Pin 14 | GND | LED circuit ground |
| Pin 20 | GND | DS3231 RTC ground |

SSR input (−) → Pi GND  
SSR output → load circuit (LEDs via buck, or solenoid directly)

---

## Power Budget

- LEDs: 5× UV @ 450mA + 5× Blue @ 350mA = **4A @ 3.2V** (~13W)
- Pi: ~500mA @ 5V
- Solenoid: ~1A @ 12V (only during irrigation window)
- Battery: 12V 20Ah = 240Wh → ~5 hours LED runtime
- Solar: 25W panel charges during day, covers nightly draw

---

## The Script — `/home/eric/homestead.py`

```python
#!/usr/bin/env python3
"""
Homestead Automation — Raspberry Pi 3 A+
Chicken run bug lights + garden irrigation controller

GPIO (BCM):
  17 → SSR #1 → D-Planet buck (12V→3.2V) → 5x UV + 5x Blue LEDs
  27 → SSR #2 → 12V solenoid valve → rain barrel → garden
"""

import sys
import time
import logging
import RPi.GPIO as GPIO

LED_PIN      = 17
SOLENOID_PIN = 27
LOG_FILE     = '/home/eric/homestead.log'

logging.basicConfig(
    filename=LOG_FILE,
    level=logging.INFO,
    format='%(asctime)s  %(message)s',
    datefmt='%Y-%m-%d %H:%M'
)

def log(msg):
    logging.info(msg)
    print(msg)

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(LED_PIN,      GPIO.OUT)
GPIO.setup(SOLENOID_PIN, GPIO.OUT)

def leds_on():
    GPIO.output(LED_PIN, GPIO.HIGH)
    log("LEDs ON")

def leds_off():
    GPIO.output(LED_PIN, GPIO.LOW)
    log("LEDs OFF")

def leds_test():
    log("LEDs TEST start -- 0.125s on / 0.25s off for 1 min")
    end_time = time.time() + 60
    while time.time() < end_time:
        GPIO.output(LED_PIN, GPIO.HIGH)
        time.sleep(0.125)
        GPIO.output(LED_PIN, GPIO.LOW)
        time.sleep(0.25)
    log("LEDs TEST done")
    from datetime import datetime
    hour = datetime.now().hour
    if 18 <= hour < 23:
        GPIO.output(LED_PIN, GPIO.HIGH)
        log("LEDs ON")

def irrigate(minutes=20):
    log(f"Irrigation ON -- {minutes} min")
    GPIO.output(SOLENOID_PIN, GPIO.HIGH)
    time.sleep(minutes * 60)
    GPIO.output(SOLENOID_PIN, GPIO.LOW)
    log("Irrigation OFF")

def irrigate_on():
    GPIO.output(SOLENOID_PIN, GPIO.HIGH)
    log("Irrigation ON")

def irrigate_off():
    GPIO.output(SOLENOID_PIN, GPIO.LOW)
    log("Irrigation OFF")

if __name__ == '__main__':
    cmd = sys.argv[1] if len(sys.argv) > 1 else ''
    if cmd == 'leds-on':
        leds_on()
    elif cmd == 'leds-off':
        leds_off()
    elif cmd == 'leds-test':
        leds_test()
    elif cmd == 'irrigate':
        mins = int(sys.argv[2]) if len(sys.argv) > 2 else 20
        irrigate(mins)
    elif cmd == 'irrigate-on':
        irrigate_on()
    elif cmd == 'irrigate-off':
        irrigate_off()
    elif cmd == 'status':
        import subprocess
        result = subprocess.run(['tail', '-20', LOG_FILE], capture_output=True, text=True)
        print(result.stdout or '(no log entries yet)')
    else:
        print("Usage: homestead.py [leds-on | leds-off | leds-test | irrigate [minutes] | irrigate-on | irrigate-off | status]")
        sys.exit(1)
```

---

## Cron Jobs (`crontab -e`)

```
# Bug lights: on 6PM, off 11PM (America/New_York)
0 18 * * * /usr/bin/python3 /home/eric/homestead.py leds-on
0 23 * * * /usr/bin/python3 /home/eric/homestead.py leds-off

# Irrigation: 20 min at 6AM
0  6 * * * /usr/bin/python3 /home/eric/homestead.py irrigate 20

# Nightly reboot at 2AM (prevent long-uptime crashes)
0  2 * * * sudo /sbin/reboot

# On boot: restore scheduled state (LEDs if 6-11PM, irrigation if 6:00-6:19AM only)
@reboot sleep 10 && /usr/bin/python3 /home/eric/boot_check.py
```

### Boot State Recovery — `/home/eric/boot_check.py` (added 2026-04-19, fixed 2026-04-30)

Solves the problem where a reboot mid-schedule (e.g. power cycle at 8 PM) leaves LEDs off until the next 6 PM cron. On every boot, checks the current time and restores the correct GPIO state:
- 6-11 PM: turns LEDs on
- 6:00-6:19 AM: starts irrigation (minute < 20 guard added 2026-04-30 — `irrigate-on` has no timer, so it must only fire during the cron's actual 20-min window or the solenoid stays open indefinitely)

## Boot Configuration (added 2026-04-13)

- **Autologin enabled** on tty1 via systemd override:
  `/etc/systemd/system/getty@tty1.service.d/override.conf`
- Pi boots straight to `eric@homestead:~ $` — no login prompt
- Cron starts at boot without login (systemd service, always enabled)
- **Timezone**: America/New_York (EDT) — was incorrectly Europe/London, fixed 2026-04-13
- Pi is fully autonomous — no keyboard, monitor, or WiFi needed for operation

---

## Pi Setup (Flashing from Windows — No GUI)

```bash
# Download image
curl -L -o raspi-lite.img.xz "https://downloads.raspberrypi.com/raspios_lite_arm64/images/raspios_lite_arm64-2025-12-04/2025-12-04-raspios-trixie-arm64-lite.img.xz"

# Extract (Python 3.12)
python.exe -c "
import lzma, sys, os
with lzma.open('raspi-lite.img.xz','rb') as src, open('raspi-lite.img','wb') as dst:
    while True:
        chunk = src.read(4*1024*1024)
        if not chunk: break
        dst.write(chunk)
        sys.stdout.write(f'\r{os.path.getsize(\"raspi-lite.img\")//1024//1024}MB')
"
```

Flash to disk using elevated PowerShell script `raspi-flash.ps1`:
- Uses `diskpart clean` to release all volume locks
- Uses Win32 `CreateFile`/`WriteFile` P/Invoke (same as Rufus) — `.NET FileStream` alone gets access denied
- After write: rescan, assign drive letter, write `ssh` file, `firstrun.sh`, update `cmdline.txt`

`firstrun.sh` on boot partition handles: hostname, user creation, SSH enable, password via `chpasswd`.

---

## Pi Info

- **Hostname**: homestead
- **User**: eric
- **Password**: 645866
- **IP**: 192.168.12.114 (DHCP, confirmed 2026-04-12)
- **SSH key**: `~/.ssh/id_ed25519` (Eric's Windows PC)
- **SSH from Eric's PC**: `ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114`
- **SSH from ThinkCentre**: `ssh -i /home/milton/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114` (requires nc proxy fix in ~/.ssh/config — see ThinkCentre skill)
- **NEVER use sshpass** — use key auth only (see feedback_ssh_key_auth.md)
- **Authorized keys on Pi**: Eric's Windows PC key + ThinkCentre's key (both in ~/.ssh/authorized_keys)
- **Fans**: Two mini fans on pins 2/4 (5V) and 6/9 (GND) for cooling

### Quick LED Test (confirmed working 2026-04-12)

```bash
# On, off, on for 10s, off — full test sequence
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114 \
  "python3 /home/eric/homestead.py leds-on && sleep 2 && \
   python3 /home/eric/homestead.py leds-off && sleep 2 && \
   python3 /home/eric/homestead.py leds-on && sleep 10 && \
   python3 /home/eric/homestead.py leds-off"
```

Individual commands:
- `python3 /home/eric/homestead.py leds-on`
- `python3 /home/eric/homestead.py leds-off`
- `python3 /home/eric/homestead.py irrigate 20`
- `python3 /home/eric/homestead.py status`

## Notes

- Pi 3 A+ has **no Ethernet** — WiFi only. For headless deployment WiFi not needed (offline operation).
- Pi 3 A+ needs **5V/2.5A micro-USB** supply minimum. Undervoltage crashes WiFi chip.
- `RPi.GPIO` was pre-installed on Trixie Lite image — no `apt install` needed.
- Do NOT call `GPIO.cleanup()` in leds-on/off — pin state must persist after script exits.
- Timezone set to `America/New_York` — cron fires at correct local time.

---

## SSH Troubleshooting — Lessons Learned

### ❌ NEVER connect the Pi to a phone hotspot
A phone hotspot puts the Pi on a **different subnet** (e.g. `10.90.107.x`) than the home network (`192.168.12.x`). SSH from a PC on DIEMILTONHAUS will never reach a Pi on the phone hotspot. Always connect the Pi directly to **DIEMILTONHAUS** via `sudo nmtui`.

### T-Mobile "Advanced Security" blocks device-to-device traffic
The T-Mobile TMO-G5AR gateway has an **Advanced Security** feature enabled by default that blocks local LAN device-to-device traffic (including SSH, ping, ARP). 

**Symptom**: Ping from PC returns `Reply from 192.168.12.219: Destination host unreachable` (reply comes from your *own* IP — ARP failed).

**Fix**: Open the **T-Mobile T-Life app** (NOT the router web UI at 192.168.12.1 — the setting isn't exposed there). Disable Advanced Security. Wait for "Disabling in progress" to complete.

### SSH diagnostic order
1. On Pi: `sudo systemctl status ssh` → confirm active
2. On Pi: `sudo ss -tlnp | grep 22` → confirm listening on `0.0.0.0:22`
3. On Pi: `ip addr show wlan0` → confirm IP is `192.168.12.x` (NOT `10.x.x.x` = phone hotspot)
4. On Pi: `ping 192.168.12.1` → confirm Pi can reach router
5. From PC: `ping <pi-ip>` → if "Destination host unreachable from your own IP" it's Advanced Security / AP isolation
6. From PC: `ssh -i ~/.ssh/id_ed25519 eric@<pi-ip>`

### WiFi setup
- `sudo nmtui` is the tool on Trixie Lite (no `raspi-config` or `wpa_supplicant.conf` editing needed)
- Select "Activate a connection" → pick DIEMILTONHAUS → enter password

---

## Spare Hardware

| Item | Spec | Notes |
|------|------|-------|
| Battery | Rapthor 24V 4Ah (4000mAh) Li-ion with charger | Ordered by mistake. Does NOT fit this 12V project — 96Wh capacity can't cover overnight LED schedule (100Wh needed), and charge controller is 12V LiFePO4 only. Save for a separate indoor/portable project. |

---

## Pi-Tools Desktop App (added 2026-04-25)

Zenity-based GUI at `/home/ericmilton/Pi-Tools/` on Eric's laptop. One-click access to all Pi commands via a popup menu. Each tool opens in a ptyxis terminal.

### Menu Options
1. **Pi Status** — LEDs ON/OFF, Irrigation ON/OFF, uptime, temp, memory, disk
2. **Pi Log** — full homestead.log
3. **Pi LED Test** — 1-minute blink test (GPIO 17)
4. **Water On** — opens solenoid (GPIO 27)
5. **Water Off** — closes solenoid (GPIO 27)

### Connectivity: SSH with Bluetooth Fallback
All scripts use `pi-run.sh` — a shared helper that:
1. Pings the Pi (192.168.12.114) with a 2-second timeout
2. If reachable → runs the command via SSH
3. If unreachable → runs the command via Bluetooth (`~/homestead-bt.py`)

This means the app works identically whether the Pi is on WiFi or deployed offline in the barn.

### Files
| File | Purpose |
|------|---------|
| `pi-tools.sh` | Main Zenity menu |
| `pi-run.sh` | Shared SSH/Bluetooth fallback helper |
| `pi-status.sh` | Status with LED/irrigation state at top |
| `pi-log.sh` | Full log display |
| `pi-led-test.sh` | Blink test |
| `pi-water-on.sh` | Open solenoid |
| `pi-water-off.sh` | Close solenoid |
| `pi-shutdown.sh` | Safe shutdown (LEDs off, solenoid closed, then poweroff) |

Shutdown has a Zenity confirmation dialog before executing.

---

## Bluetooth Command Server (added 2026-04-25)

The Pi runs `bt-homestead.service` (systemd, enabled, auto-restart) which accepts commands over Bluetooth RFCOMM. See skill `homestead-bluetooth` for full details.

**Commands:** `status`, `log`, `ledtest`, `ledsoff`, `wateron`, `wateroff`, `shutdown`, `help`

The `status` command includes current LED and irrigation state (parsed from homestead.log) at the top of its output.

---

## Remaining Tasks (updated 2026-04-29)

- [ ] **Wire UV/Blue LEDs** — Replace green test LED with 5x UV (365nm) + 5x Blue (460nm), parallel at 3.2V through DROK buck. Mount in aluminum bar in ground for heatsinking.
- [ ] **Wire solenoid** — GPIO 27 → SSR #2 → 12V solenoid → rain barrel → chicken waterer. Update schedule for chicken watering (shorter/more frequent than old 20-min garden cycle).
- [ ] **Print enclosure** — Base (~12-14 hrs) + lid (~3 hrs) in PETG. Need: 4x neodymium magnets (8mm×3mm), 1x hinge pin (3mm rod or M3×180 screw).
- [ ] **Assemble box** — Mount Pi, Wanderer, SSRs, bucks inside with zip ties. Glue magnets, insert hinge pin. Route cables out bottom slot.
- [ ] **Deploy to barn** — Mount box on wall (4x M4 screws), connect solar panel, battery, LED circuit, solenoid. Field test all functions.
- [ ] **Update solenoid code** — Rename irrigate() to chicken_water() in homestead.py. Update cron schedule, BLE GATT labels, ESP32 dashboard labels.
- [x] **Bluetooth dongle** — Edimax BT-8500 installed 2026-04-30. Built-in BT disabled (`dtoverlay=disable-bt`). BLE service using Edimax (hci0 after reboot). ESP32 connects by name, no code change needed. Range test from barn pending.

### Hardware Still Needed
- 4x neodymium disc magnets (8mm × 3mm)
- 1x 3mm steel rod or M3×180mm screw (hinge pin)
- 4x M4 × 16mm screws + wall anchors
- Aluminum bar stock (LED heatsink mount)
- CA (super glue) for magnets
