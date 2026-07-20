# PiCam Camera Server

**Status:** WORKING. Live MJPEG stream on port 8080, integrated into MILTONHAUS Weather dashboard. Video-only — no audio yet (see "Planned: Microphone" below).
**Date:** 2026-07-03 (Camera/Voice buttons briefly disappeared from the dashboard 2026-07-16–2026-07-20 due to an unrelated commit; restored, see `esp32-weather-station` skill)

## Planned: Microphone (2026-07-20)

Eric wants **one-way ambient audio only** (not two-way intercom) alongside the video feed. No mic hardware on hand yet.

- **Recommended:** [SunFounder USB 2.0 Mini Microphone](https://www.amazon.com/SunFounder-Microphone-Raspberry-Recognition-Software/dp/B01KLRBHGM) — plug-and-play, no driver, confirmed compatible with Pi 3.
- **Once purchased and plugged into the Pi 3 (192.168.12.211):** build an audio capture service (e.g. `arecord`/`ffmpeg` piped to a simple HTTP audio stream, same pattern as the existing `mjpeg_server.py`) and add an `<audio>` element to the `#camOverlay` div in `esp32-weather.ino`, started/stopped alongside `showCam()`/`closeCam()`.

## Hardware

- Raspberry Pi 3 Model B Rev 1.2
- OV5647 camera module (v1, 5MP) confirmed working
- 59GB micro SD card
- Ethernet connection to router

## Network

- **IP:** 192.168.12.211
- **Hostname:** picam
- **User:** eric / 645866
- **SSH:** `ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.211`

## OS

- Raspbian GNU/Linux 13
- Kernel: 6.18.34+rpt-rpi-v7 (32-bit armv7l)
- Flashed 2026-07-03 via Raspberry Pi Imager (32-bit Lite)

## Camera Config

Added to `/boot/firmware/config.txt`:
```
camera_auto_detect=1
dtoverlay=ov5647
start_x=1
gpu_mem=256
```

Camera confirmed: `rpicam-hello --list-cameras` shows OV5647 at 2592x1944.
Note: `vcgencmd get_camera` reports `detected=0` but this is a legacy tool issue. libcamera/rpicam tools work fine.

## MJPEG Stream Service

**Service:** `picam-stream.service`
**Port:** 8080
**Script:** `/home/eric/mjpeg_server.py`

Python HTTP server that captures JPEG frames via `rpicam-still` (640x480, quality 60, vflip+hflip) and serves them as:
- MJPEG stream at `http://192.168.12.211:8080/` (multipart/x-mixed-replace)
- Single snapshot at `http://192.168.12.211:8080/snapshot`

Captures at ~2 fps. CORS headers included for cross-origin dashboard access.

## Dashboard Integration

Camera button added to the MILTONHAUS Weather dashboard footer. Tapping it opens a fullscreen overlay (same pattern as hourly forecast, day detail, and kids chores overlays) showing the live MJPEG stream. Closing the overlay stops the stream to save bandwidth.

Changes in `esp32-weather.ino`:
- `.cam-btn` CSS class for the footer button
- `#camOverlay` div with img tag for the stream
- `showCam()` sets img src to start stream, `closeCam()` clears src to stop

## Setup Steps Completed

1. Flashed fresh Raspbian Lite 32-bit via Raspberry Pi Imager
2. Created user eric, set password
3. Enabled SSH via empty `ssh` file on boot partition
4. Set hostname to `picam`
5. Set WLAN country to US (via raspi-config)
6. Added camera overlays to config.txt
7. Added SSH public key to authorized_keys
8. Created MJPEG server script (`~/mjpeg_server.py`)
9. Created systemd service (`picam-stream.service`), enabled on boot
10. Added camera overlay button to weather dashboard (OTA flashed)
11. Fixed upside-down image with `--vflip --hflip` flags

## Remote Commands

```bash
# SSH in
ssh -i ~/.ssh/id_ed25519 eric@192.168.12.211

# Check camera
rpicam-hello --list-cameras

# Restart stream
echo 645866 | sudo -S systemctl restart picam-stream.service

# Check stream status
echo 645866 | sudo -S systemctl status picam-stream.service

# Test snapshot
curl -o test.jpg http://192.168.12.211:8080/snapshot

# Reboot
echo 645866 | sudo -S reboot
```
