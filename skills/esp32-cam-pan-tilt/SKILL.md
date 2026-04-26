# ESP32-CAM Pan-Tilt Camera

Pan-tilt camera project on the ESP32-CAM. Original source code lost — this skill documents what's known from the firmware currently loaded on the board, to help rebuild it.

---

## Hardware

- **Board:** ESP32-CAM + ESP32-CAM-MB (USB programmer base)
- **Chip:** ESP-32S (WiFi + Bluetooth)
- **USB:** CH340 serial converter
- **Serial baud:** 115200

## What the Firmware Does (from serial boot log)

```
ESP32-CAM Pan-Tilt Starting...
Joystick serial initialized on GPIO13
Servos initialized
Camera initialized
Connecting to WiFi.........
```

### Features
1. **Pan-tilt servos** — two servos for horizontal/vertical camera movement
2. **Joystick control** — joystick input on GPIO 13 (serial protocol)
3. **Camera** — OV2640 camera initialized (likely streaming via WiFi)
4. **WiFi** — connects to a network (SSID/password unknown, needs reconfiguring)

## To Rebuild

- Source code is NOT saved — will need to be rewritten
- Likely used the ESP32-CAM camera streaming example as a base
- Added servo control via joystick input
- Key libraries: ESP32 Camera, ESP32Servo, WiFi, WebServer
- GPIO 13 used for joystick serial input
- Typical pan-tilt uses 2x SG90 servo motors

## Programming

- Plug ESP32-CAM into ESP32-CAM-MB
- Connect via USB (shows as /dev/ttyUSB0, CH340)
- Arduino IDE: select "AI Thinker ESP32-CAM" board
- Hold BOOT button on CAM-MB if upload fails

---

*Created: 2026-04-26*
