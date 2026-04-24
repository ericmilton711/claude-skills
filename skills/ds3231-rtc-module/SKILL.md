# DS3231 RTC Module — Homestead Pi

## Purpose
Add a hardware real-time clock to the Homestead Pi so it keeps accurate time through reboots and power loss, enabling fully offline operation in the barn without WiFi/NTP dependency.

## Hardware
- **Module:** DS3231 RTC (I2C, 3.3V/5V compatible)
- **Battery:** CR2032 coin cell (keeps clock running when Pi is off)
- **Pi:** Homestead Pi — `eric@192.168.12.114`
- **Connection:** 4 wires — VCC (3.3V), GND, SDA (GPIO 2), SCL (GPIO 3)

## Wiring (Pi 3 A+ GPIO Header) — CONFIRMED WORKING 2026-04-24
| RTC Pin | Pi Pin | GPIO |
|---------|--------|------|
| VCC     | Pin 1  | 3.3V |
| SDA     | Pin 3  | GPIO 2 (SDA1) |
| SCL     | Pin 5  | GPIO 3 (SCL1) |
| GND     | Pin 20 | GND  |

Pin 6 is taken by fan/SSR grounds, Pin 14 is LED ground — Pin 20 is the RTC's dedicated GND.

## Setup Steps

### 1. Enable I2C
```bash
sudo raspi-config nonint do_i2c 0
```

### 2. Verify module detected
```bash
sudo i2cdetect -y 1
# Should show device at address 0x68 (RTC) and 0x57 (EEPROM)
```

### 3. Install required packages
```bash
sudo apt install -y i2c-tools util-linux-extra
```
Trixie Lite doesn't include `i2cdetect` or `hwclock` by default.

### 4. Load RTC kernel module
```bash
echo "dtoverlay=i2c-rtc,ds3231" | sudo tee -a /boot/firmware/config.txt
sudo reboot
```
Note: Trixie uses `/boot/firmware/config.txt`, not `/boot/config.txt`.

### 5. Remove fake-hwclock (if installed)
```bash
sudo apt remove -y fake-hwclock
sudo update-rc.d -f fake-hwclock remove
sudo systemctl disable fake-hwclock
```
On Trixie Lite this was not installed by default — skip if not present.

### 6. Set the hardware clock
```bash
# Make sure system time is correct first (while NTP is still available)
date
# Write system time to RTC
sudo hwclock -w
# Verify
sudo hwclock -r
# Confirm via timedatectl (should show RTC time)
timedatectl status
```

### 7. Boot-time RTC read
On Trixie with systemd, the RTC is read automatically on boot — no need to edit `/lib/udev/hwclock-set` (that file doesn't exist on this image). Confirmed via `timedatectl` showing RTC time matching system time.

## Uses Beyond Timekeeping

### Scheduled wake from shutdown
The DS3231 has two alarm outputs (SQW/INT pin). Wire this to the Pi's RUN header to wake it from halt at a specific time. Enables full shutdown overnight to save power — useful for solar/battery setups.

### Hardware timed power control
Wire the RTC alarm pin to an SSR channel to switch 12V loads (chicken lights, irrigation pump) completely independent of the Pi. If the Pi crashes, the RTC still fires on schedule.

### Temperature logging
The DS3231 has a built-in temperature sensor accurate to ~1°C. Read it for free barn temperature data:
```bash
cat /sys/bus/i2c/devices/1-0068/hwmon/hwmon0/temp1_input
# Returns millidegrees — divide by 1000 for °C
```

### Watchdog reboot
Combine the alarm with a relay on the Pi's power. If the Pi hangs and stops resetting the alarm, the RTC cuts power and forces a hard reboot — essential for unattended barn operation.

## Why This Module
- WiFi on the Pi has been increasingly unreliable (6 drops in 3 days, Apr 20–22, 2026)
- Pi 3 A+ has no hardware RTC — loses time on every reboot without NTP
- Cron schedules (LEDs 6pm–11pm, irrigation 6am) break if system clock is wrong
- Goal is fully standalone barn operation with no network dependency

## Installation Status — COMPLETE 2026-04-24
- Module detected at I2C address 0x68
- RTC overlay loaded via `/boot/firmware/config.txt`
- Hardware clock set and verified: `2026-04-24 16:10:47 EDT`
- `timedatectl` confirms RTC time matches system clock
- Pi now keeps accurate time through reboots and power loss

## Order Info
- Amazon: Dorhea 4-pack DS3231 (ordered, installed 1 of 4)
- CR2032 coin cell battery included
