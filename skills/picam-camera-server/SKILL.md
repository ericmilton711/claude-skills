# PiCam Camera Server

**Status:** WORKING. Live MJPEG video (port 8080) + one-way ambient audio (port 8081), integrated into MILTONHAUS Weather dashboard.
**Date:** 2026-07-24 — microphone (USB PnP Sound Device, TI PCM2902 codec, ALSA card 2) added and wired up.

## Microphone / Audio Stream

One-way ambient audio only (not two-way intercom), per Eric's request.

- **Hardware:** generic USB microphone (PCM2902 codec), shows up as ALSA card 1 (`plughw:1,0`) on the Pi 3. **Note:** USB sound card numbers can shift across reboots. If audio goes silent, run `arecord -l` to find the current card number and update `ALSA_DEVICE` in `audio_server.py`.
- **Service:** `picam-audio.service` — `/home/eric/audio_server.py`, port 8081.
- **How it works:** a single `arecord` process captures from the mic and an `AudioBroadcaster` fans the PCM chunks to all connected HTTP clients. Each client gets a synthetic streaming-WAV header (oversized bogus data-chunk size, since total length is unknown) followed by the live PCM stream. `arecord` starts on the first client connection and stops when the last client disconnects. Multiple devices can listen simultaneously.
- **Test:** `curl http://192.168.12.211:8081/ -o test.wav` (a few seconds), then `file test.wav` should say `RIFF ... WAVE audio ... 16 bit, mono 16000 Hz`.
- **Dashboard integration:** `<audio id="camAudio" autoplay>` in `#camOverlay` in `esp32-weather.ino`; `showCam()` sets its `src` and explicitly calls `.load()`/`.play()` (autoplay attribute alone isn't reliable on mobile Safari for a dynamically-set src), `closeCam()` pauses and clears `src`.

## Remote Access (Tailscale) — added 2026-07-24

Camera/audio don't work over Tailscale by default because `showCam()` hardcodes the LAN IP `192.168.12.211`, which a remote Tailscale client can't reach (see `esp32-weather-station` skill — remote access only works via a single-port proxy on the ThinkCentre, not a full subnet route).

**Fix:** two new streaming reverse-proxy services on the ThinkCentre (100.70.179.60), mirroring `weather-proxy.service` but supporting infinite streams (the existing proxy does a blocking full `read()` which never returns for MJPEG/WAV):
- `camera-proxy.service` — port 8241 → `http://192.168.12.211:8080` (video)
- `audio-proxy.service` — port 8242 → `http://192.168.12.211:8081` (audio)
- Both run `/opt/stream-proxy.py <upstream-url> <port>` (generic chunked passthrough proxy, `ThreadingTCPServer` with `allow_reuse_address=True`).

`camBase()` in `esp32-weather.ino` picks the URL set based on `window.location.hostname`: LAN (`192.168.12.240`) uses the picam IPs directly; Tailscale (`100.70.179.60`) uses the two proxy ports above.

**To verify:** `curl http://192.168.12.136:8241/snapshot` and `curl http://192.168.12.136:8242/ -o test.wav --max-time 3` from the LAN, or the same against `100.70.179.60` from a Tailscale client.
- **Gotchas found 2026-07-24 (first test was silent):**
  - Mic capture volume defaulted to **0%** even though ALSA reported it "on" — fixed with `amixer -c 1 sset Mic 100% unmute`, then persisted across reboots with `sudo alsactl store` (saved to `/var/lib/alsa/asound.state`, restored automatically by `alsa-restore.service`). If audio ever goes silent again, check `amixer -c 1 sget Mic` first. **Also check card number** — USB devices can shift across reboots (was card 2, now card 1 as of 2026-07-26).
  - `socketserver.ThreadingTCPServer` needs `allow_reuse_address = True` (subclass it) or the service crash-loops on every restart with `OSError: Address already in use` until the OS's TIME_WAIT expires. Already fixed in the deployed script.

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

# Check mic is detected
arecord -l

# Restart audio stream
echo 645866 | sudo -S systemctl restart picam-audio.service

# Check audio stream status
echo 645866 | sudo -S systemctl status picam-audio.service

# Test audio stream (few seconds of WAV, then inspect with `file`)
curl --max-time 4 http://192.168.12.211:8081/ -o test.wav

# Reboot
echo 645866 | sudo -S reboot
```
