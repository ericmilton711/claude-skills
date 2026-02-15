# ESP32-CAM Pan-Tilt Web Controller

Web-controlled pan-tilt camera mount using ESP32-CAM with OV2640. Supports both web interface control and optional Arduino joystick control.

## Hardware

- Aideepen ESP32-CAM-MB (with CH340G USB-serial)
- 2x SG90 Micro Servos
- USB-C Power Module (5V 2-3A) for servos
- **Micro USB DATA cable** (charge-only cables won't work for programming)
- 3D printed pan-tilt mount (see `servo_camera_mount.scad`)
- **Optional:** Arduino UNO + analog joystick for physical control

## Features

- Live MJPEG video streaming
- Pan/tilt control via web interface
- Arrow buttons for 10-degree steps
- Sliders for precise positioning
- Center button to reset to 90/90
- Mobile-friendly touch controls
- WiFi station or Access Point mode
- **Optional Arduino joystick control** (physical joystick via serial)

## Quick Start

1. Install **esp32 by Espressif Systems** in Board Manager
2. Install **ESP32Servo** library
3. Select board: **AI Thinker ESP32-CAM**
4. Edit WiFi credentials in the .ino file
5. Connect via Micro USB data cable
6. Upload and open the IP address in browser

**See `SETUP_GUIDE.md` for detailed step-by-step instructions** (includes Arduino IDE 2.x specifics, driver installation, and troubleshooting)

## GPIO Pin Assignments

| Function | GPIO |
|----------|------|
| Pan Servo | 14 |
| Tilt Servo | 15 |
| Joystick RX (Serial2) | 13 |

These pins are safe to use with the camera active.

## Usage

1. Power on the ESP32-CAM
2. Connect to WiFi (or connect to "ESP32-CAM-PanTilt" AP)
3. Open the IP address in a browser
4. Use arrow buttons or sliders to control pan/tilt
5. Video streams automatically

## Files

| File | Description |
|------|-------------|
| `esp32_cam_pan_tilt/esp32_cam_pan_tilt.ino` | Main ESP32 Arduino sketch |
| `arduino_joystick/arduino_joystick.ino` | Arduino UNO joystick controller |
| `wiring_guide.txt` | Detailed wiring diagram |
| `SETUP_GUIDE.md` | Full Arduino IDE 2.x setup instructions |
| `README.md` | This file |

## Joystick Control (Optional)

To add physical joystick control:

1. Upload `arduino_joystick.ino` to an Arduino UNO
2. Connect joystick to Arduino (VRx=A0, VRy=A1, SW=D2)
3. Wire Arduino TX (pin 1) to ESP32 GPIO13
4. Wire Arduino GND to ESP32 GND
5. Move joystick to control pan/tilt, press button to center

See `wiring_guide.txt` for detailed wiring diagram.

## Related Files

In parent directory (`C:\Users\ericm\`):
- `servo_camera_mount.scad` - OpenSCAD 3D model
- `servo_camera_mount.stl` - 3D printable STL
- `servo_wiring_guide.txt` - General servo wiring reference

## Troubleshooting

**Camera shows black/no image:**
- Check camera ribbon cable connection
- Ensure board is set to "AI Thinker ESP32-CAM"

**Servos don't respond:**
- Verify common ground between ESP32 and servo power
- Check signal wires on GPIO 14 and 15

**Can't connect to WiFi:**
- Check SSID/password
- Device falls back to AP mode automatically

**Jerky video:**
- Reduce frame size in code (FRAMESIZE_CIF)
- Increase jpeg_quality value (lower quality = faster)
