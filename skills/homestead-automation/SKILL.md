# Homestead Automation — Raspberry Pi 3 A+ Controller

## Project Overview
Raspberry Pi 3 A+ controlling two things in the chicken run / garden:
1. **Bug attraction lights** — UV (365nm) + Blue (460nm) LEDs to attract insects for chickens
2. **Garden irrigation** — 12V solenoid valve off a rain barrel, 40×50ft garden (18 rows)

Pi runs completely **offline** (no WiFi needed in the field). Cron fires the Python script on schedule.

---

## Hardware

| Item | Detail |
|------|--------|
| Pi 3 A+ | MILTONRP3, booting from 64GB microSD |
| Battery | DR.PREPARE 12V 20Ah LiFePO4 |
| Solar panel | ECO-WORTHY 25W |
| Charge controller | Renogy Wanderer 10A PWM (LiFePO4) |
| Buck converters | DROK LM2596 adjustable ×2 (12V→5V for Pi, 12V→3.2V for LEDs) |
| SSRs | SSR-41FDD ×2 (input 3–32VDC, output 6A 60VDC) |
| UV LEDs | 5× CHANZON 3W 365nm (UV) |
| Blue LEDs | 5× 1W 460–465nm (blue) |
| Solenoid | U.S. Solid ¼" NC 12V solenoid valve |
| Enclosure | 3D printed PETG (weatherproof) |
| Sealant | DAP High Temp RTV Silicone |

**LED wiring**: All 10 chips in **parallel** at ~3.2V. LEDs buried in aluminum bar in ground for heatsinking.

---

## GPIO Wiring (BCM numbering)

| Pi Pin | BCM | Connected to |
|--------|-----|-------------|
| Pin 11 | GPIO 17 | SSR #1 input (+) → LED circuit |
| Pin 13 | GPIO 27 | SSR #2 input (+) → solenoid |
| Pin 6/9/14 | GND | SSR input (−) for both |
| Pin 2/4 | 5V | Two cooling fans (always on) |
| Pin 6/9 | GND | Fan ground |

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
  17 → SSR #1 → DROK buck (12V→3.2V) → 5x UV + 5x Blue LEDs
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

def irrigate(minutes=20):
    log(f"Irrigation ON — {minutes} min")
    GPIO.output(SOLENOID_PIN, GPIO.HIGH)
    time.sleep(minutes * 60)
    GPIO.output(SOLENOID_PIN, GPIO.LOW)
    log("Irrigation OFF")

if __name__ == '__main__':
    cmd = sys.argv[1] if len(sys.argv) > 1 else ''
    if cmd == 'leds-on':
        leds_on()
    elif cmd == 'leds-off':
        leds_off()
    elif cmd == 'irrigate':
        mins = int(sys.argv[2]) if len(sys.argv) > 2 else 20
        irrigate(mins)
    elif cmd == 'status':
        import subprocess
        result = subprocess.run(['tail', '-20', LOG_FILE], capture_output=True, text=True)
        print(result.stdout or '(no log entries yet)')
    else:
        print("Usage: homestead.py [leds-on | leds-off | irrigate [minutes] | status]")
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

# On boot: restore scheduled state (LEDs if 6-11PM, irrigation if 6AM hour)
@reboot sleep 10 && /usr/bin/python3 /home/eric/boot_check.py
```

### Boot State Recovery — `/home/eric/boot_check.py` (added 2026-04-19)

Solves the problem where a reboot mid-schedule (e.g. power cycle at 8 PM) leaves LEDs off until the next 6 PM cron. On every boot, checks the current time and restores the correct GPIO state:
- 6-11 PM: turns LEDs on
- 6-7 AM: starts irrigation

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
| Battery | 24VDC, 1A rated | Ordered by mistake. Does NOT fit this 12V project. Ah capacity unknown — check label. Could be used for a future 24V project. |
