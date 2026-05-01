# Homestead Macro Keypad — QINIZX 4-Key USB-C Pad

## Product Info
- **Brand:** QINIZX
- **Amazon:** B09PD2BWHL
- **Description:** Mini 4-Key Macro Mechanical Keypad RGB
- **Features:** Red switches, RGB backlight, hot-swap sockets, USB-C cable
- **USB Vendor ID:** 0x8808
- **USB Product ID:** 0x6611
- **Connected to:** Homestead Pi (Raspberry Pi 3 A+) via USB hub (Genesys Logic 05e3:0610)
- **Config software:** key.itytsoft.com (Windows .exe, needed to program keys — ships with no default mapping)

---

## Key Layout & Mapping

Keys were programmed via the QINIZX config software to send Z/X/C/V.

| Physical Position | HID Code | Letter | Action | homestead.py Command |
|-------------------|----------|--------|--------|---------------------|
| Top (single key) | 0x1d | Z | LED test | `leds-test` (60s rapid on/off) |
| Bottom-left | 0x1b | X | Water ON | `irrigate-on` |
| Bottom-center | 0x06 | C | Water OFF | `irrigate-off` |
| Bottom-right | 0x19 | V | Safe shutdown | `sudo shutdown now` (double-press within 2s required) |

---

## How It Works

This keypad does NOT generate evdev input events — the kernel's hid-generic driver handles the USB reports but doesn't route them to /dev/input/event*. The daemon reads raw HID reports directly from /dev/hidraw instead.

Each key press sends an 8-byte HID keyboard report. Byte 2 contains the HID usage code (0x1d=Z, 0x1b=X, 0x06=C, 0x19=V). Byte 2 = 0x00 is a key release (ignored).

---

## USB Interface Layout

The keypad enumerates as 4 HID interfaces:

| Interface | hidraw | evdev | Name | HID_PHYS |
|-----------|--------|-------|------|----------|
| 0 | hidraw0 | event2/event4 | HID 8808:6611 / Keyboard | input0 — **keypad.py reads this one via hidraw** |
| 1 | hidraw1 | event3 | HID 8808:6611 | input1 |
| 2 | hidraw2 | event5 | HID 8808:6611 | input2 |
| 3 | hidraw3 | event6 | HID 8808:6611 Mouse | input3 |

---

## Scripts on the Pi

### `/home/eric/keypad.py` — Keypad daemon
- Finds the keypad by scanning /sys/class/hidraw/*/device/uevent for vendor/product ID
- Reads raw 8-byte HID reports from /dev/hidraw (NOT evdev)
- Maps HID usage codes to homestead.py commands
- Power down requires double-press within 2 seconds (safety)
- Power down chains `leds-off` first if between 18:00-23:00 (LED hours)
- Runs as root via systemd

### `/home/eric/homestead.py` — GPIO control script
- GPIO 17 = LED circuit (SSR #1)
- GPIO 27 = Solenoid valve — chicken water (SSR #2)
- Commands: `leds-on`, `leds-off`, `leds-test`, `irrigate [minutes]`, `irrigate-on`, `irrigate-off`, `status`

---

## Systemd Service

```
/etc/systemd/system/keypad.service
```

- **Enabled at boot**, restarts on failure (5s delay)
- `systemctl status keypad` to check
- `systemctl restart keypad` after editing keypad.py

---

## Diagnosing Key Issues

### Test if keys send raw HID data:
```bash
# Stop keypad daemon first
ssh eric@192.168.12.114 "sudo systemctl stop keypad"

# Read raw HID bytes — press keys and watch for output
ssh eric@192.168.12.114 "sudo timeout 15 cat /dev/hidraw0 | od -A x -t x1"

# Restart daemon after testing
ssh eric@192.168.12.114 "sudo systemctl start keypad"
```

### If no raw HID data appears:
- Keys may need reprogramming via the Windows config software (key.itytsoft.com)
- Check USB connection: `lsusb` should show `8808:6611`

---

## Previous Keypad (Replaced)

The original QINIZX 6-key pad (product ID 0x6606) had a dead top-left key (hardware defect). It used evdev (event0) and mapped KEY_A through KEY_H. The new 4-key pad replaced it on 2026-05-01.
