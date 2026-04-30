# Homestead Macro Keypad — QINIZX 6-Key USB HID Pad

## Product Info
- **Brand:** QINIZX
- **Description:** Mini 6-Key Macro Mechanical Keypad RGB Gaming Keyboard
- **Features:** Red switches, RGB backlight, hot-swap sockets, USB-Micro cable
- **USB Vendor ID:** 0x8808
- **USB Product ID:** 0x6606
- **Connected to:** Homestead Pi (Raspberry Pi 3 A+) via USB hub (Genesys Logic 05e3:0610)
- **Status:** Top-left key position is DEAD (hardware defect, needs return/replacement)

---

## USB Interface Layout

The keypad enumerates as 4 HID interfaces:

| Interface | hidraw | evdev | Name | Usage Page | Purpose |
|-----------|--------|-------|------|------------|---------|
| 0 | hidraw0 | event0 | HID 8808:6606 | 0x01 (Generic Desktop) / Keyboard | **Main key events — keypad.py grabs this one** |
| 1 | hidraw1 | event1 | HID 8808:6606 | 0x01 (Generic Desktop) / Mouse | Mouse HID (unused) |
| 2 | hidraw2 | event2 | HID 8808:6606 Keyboard | 0xFFA0 + 0x0C (Consumer) | Multimedia/consumer keys |
| 3 | hidraw3 | — | HID 8808:6606 | 0xFF00 (Vendor Defined) | **RGB LED control interface** |

### Interface 3 — RGB LED Control
- **Bidirectional:** EP 4 IN + EP 4 OUT, both 64 bytes, interrupt transfer
- **Report descriptor (34 bytes):** `06 00 ff 09 01 a1 01 09 02 15 00 26 ff 00 75 08 95 40 81 06 09 02 15 00 26 00 ff 75 08 95 40 91 06 c0`
- **Decoded:** Vendor-defined usage page 0xFF00, 64-byte input report + 64-byte output report, no report ID
- **RGB protocol:** UNKNOWN — blind byte-pattern probing did not produce visible LED changes. Needs USB packet capture from Windows config software to reverse-engineer.

---

## Key Mapping

The 6 keys send standard keyboard scancodes through interface 0 (event0):

| Physical Position | Key Code | keypad.py Action | homestead.py Command |
|-------------------|----------|------------------|---------------------|
| Top-left (pos 1) | KEY_A | LEDs TEST | `leds-test` (60s rapid on/off) |
| Top-center (pos 2) | KEY_S | LEDs ON | `leds-on` |
| Top-right (pos 3) | KEY_D | LEDs OFF | `leds-off` |
| Bottom-left (pos 4) | KEY_F | Irrigation ON | `irrigate-on` |
| Bottom-center (pos 5) | KEY_G | Irrigation OFF | `irrigate-off` |
| Bottom-right (pos 6) | KEY_H | Safe shutdown | `sudo shutdown now` |

---

## Scripts on the Pi

### `/home/eric/keypad.py` — Primary keypad daemon
- Finds the keypad by vendor/product ID (0x8808/0x6606)
- Filters OUT devices named "Keyboard", "Mouse", or "Jack"
- Grabs the device exclusively via `dev.grab()`
- Maps KEY_A through KEY_H to homestead.py commands
- Runs as root (needs GPIO access)
- Started at boot via systemd or cron

### `/home/eric/keypad_controller.py` — Display + keypad controller
- More feature-rich version with display sleep/wake, sensor readings, manual override mode
- Uses numeric keypad mapping (KEY_1, KEY_KP1, etc.) — different from keypad.py
- Currently attaches to wrong device (vc4-hdmi instead of keypad) — has a bug in `find_keypad()` that picks the first device with EV_KEY capability regardless of vendor ID
- Runs alongside keypad.py
- **Bug fixed 2026-04-30:** `get_gpio_state()` was using `GPIO.setup(pin, GPIO.OUT)` to read pin state — setting a pin to OUTPUT on a fresh boot drives it HIGH, which turned on the LEDs (GPIO 17 → SSR #1) every reboot. Fixed to `GPIO.setup(pin, GPIO.IN)`.

### `/home/eric/homestead.py` — GPIO control script
- GPIO 17 = LED circuit (SSR #1)
- GPIO 27 = Solenoid valve (SSR #2)
- Commands: `leds-on`, `leds-off`, `leds-test`, `irrigate [minutes]`, `irrigate-on`, `irrigate-off`, `status`

---

## Diagnosing Key Issues

### Test if a key registers:
```bash
# Stop keypad daemon first (it grabs the device exclusively)
ssh eric@192.168.12.114 "sudo killall python3 2>/dev/null"

# Monitor ALL keypad event devices for any input
ssh eric@192.168.12.114 "sudo timeout 30 python3 -u -c '
import evdev, select, time
devices = [evdev.InputDevice(p) for p in evdev.list_devices() if evdev.InputDevice(p).info.vendor == 0x8808]
print(\"Press keys now...\", flush=True)
end = time.time() + 30
while time.time() < end:
    r, w, x = select.select(devices, [], [], 1)
    for dev in r:
        for event in dev.read():
            if event.type == 1 and event.value == 1:
                name = evdev.ecodes.KEY.get(event.code, \"UNK_%d\" % event.code)
                print(\"%s %s key=%s code=%d\" % (dev.path, dev.name, name, event.code), flush=True)
'"

# IMPORTANT: Restart keypad daemon after testing
ssh eric@192.168.12.114 "nohup sudo python3 /home/eric/keypad.py &>/dev/null &"
```

### If a key sends zero events on ALL 5 interfaces:
- Not a mapping issue — the key position is electrically dead
- Hot-swap socket may have a bent pin or cracked solder joint
- Swap a known-working switch into that position to confirm socket vs switch

---

## RGB Backlight — What We Know

- The keypad has built-in RGB LEDs that cycle through colors by default
- Control is through interface 3 (/dev/hidraw3) via 64-byte output reports
- The vendor-specific protocol is NOT documented
- Common Chinese keypad byte patterns (0x03, 0x07, 0xB0 commands) did NOT work
- **Next step:** Plug keypad into Windows PC, install QINIZX config software, capture USB traffic with Wireshark + USBPcap, decode the LED control packets, then replicate from Pi
- WARNING: Writing arbitrary bytes to hidraw3 does NOT break the keypad but the keypad daemon may need restarting afterward

---

## Known Issues

1. **Top-left key (KEY_A position) is dead** — switch and socket inspected, new Cherry MX switch tried, zero events on any interface. Hardware defect. Keypad needs to be returned/replaced.

2. **keypad_controller.py picks wrong device** — `find_keypad()` selects the first input device with EV_KEY capability, which is often vc4-hdmi (HDMI hotplug) instead of the actual keypad. Should filter by vendor/product ID like keypad.py does.

3. **Both keypad.py and keypad_controller.py run simultaneously** — keypad.py grabs event0 exclusively, so keypad_controller.py can't read from it. The controller ends up on the HDMI device doing nothing useful. These should be consolidated into one script.
