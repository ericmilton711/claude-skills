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
```

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
- **IP**: DHCP (check router — often 192.168.12.114 or .162)
- **SSH key**: `~/.ssh/id_ed25519` (Eric's Windows PC)
- **SSH**: `ssh -i ~/.ssh/id_ed25519 eric@<ip>`
- **Fans**: Two mini fans on pins 2/4 (5V) and 6/9 (GND) for cooling

## Notes

- Pi 3 A+ has **no Ethernet** — WiFi only. For headless deployment WiFi not needed (offline operation).
- Pi 3 A+ needs **5V/2.5A micro-USB** supply minimum. Undervoltage crashes WiFi chip.
- `RPi.GPIO` was pre-installed on Trixie Lite image — no `apt install` needed.
- Do NOT call `GPIO.cleanup()` in leds-on/off — pin state must persist after script exits.
- Timezone set to `America/New_York` — cron fires at correct local time.
