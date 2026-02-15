# ESP32-CAM Setup Guide

Complete setup instructions for programming the Aideepen ESP32-CAM-MB.

## Hardware Required

- Aideepen ESP32-CAM-MB (with CH340G USB-Serial)
- **Micro USB DATA cable** (charge-only cables won't work)
- 2x SG90 Micro Servos
- External 5V power supply (2-3A) for servos

## Arduino IDE 2.x Setup

### Step 1: Add ESP32 Board URL

1. Open Arduino IDE 2.x
2. Go to **File > Preferences**
3. Find **"Additional boards manager URLs"** at the bottom
4. Click the small icon next to the field to open the URL list
5. Add this URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
6. Click OK

### Step 2: Install ESP32 Board Package

1. Click the **Board Manager** icon in the left sidebar (looks like a chip)
2. Search "esp32"
3. Find **"esp32 by Espressif Systems"** (NOT "Arduino ESP32 Boards")
4. Click Install

**Important:** There are two ESP32 packages:
- "Arduino ESP32 Boards" - Only has Arduino-branded boards
- "esp32 by Espressif Systems" - Has 100+ boards including AI Thinker ESP32-CAM

### Step 3: Install ESP32Servo Library

1. Click the **Library Manager** icon in the left sidebar (looks like books)
2. Search "ESP32Servo"
3. Install **ESP32Servo by Kevin Harrington**

### Step 4: Select Board

1. Go to **Tools > Board > esp32**
2. Scroll or search for **"AI Thinker ESP32-CAM"**
3. Click to select it

### Step 5: Install CH340 Driver (if needed)

The ESP32-CAM-MB uses a CH340G USB-to-serial chip. Most systems have drivers built-in, but if your board isn't recognized:

1. Download driver: https://www.wch-ic.com/downloads/CH341SER_EXE.html
2. Run the installer
3. Reconnect the board

### Step 6: Connect and Select Port

1. Connect ESP32-CAM-MB via **Micro USB data cable**
2. Open **Device Manager** (Windows key > type "Device Manager")
3. Expand **Ports (COM & LPT)**
4. Look for **"USB-SERIAL CH340 (COMx)"** - note the COM number
5. In Arduino IDE: **Tools > Port > COMx**

### Step 7: Configure Settings

| Setting | Value |
|---------|-------|
| Board | AI Thinker ESP32-CAM |
| Port | COMx (your port) |
| CPU Frequency | 240MHz (default) |
| Flash Frequency | 80MHz (default) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |

### Step 8: Edit WiFi Credentials

Open `esp32_cam_pan_tilt.ino` and edit these lines:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

Or use Access Point mode (creates its own WiFi network):
```cpp
bool useAP = true;
```

### Step 9: Upload

1. Click the Upload button (arrow icon)
2. Wait for compilation and upload to complete
3. Open **Serial Monitor** (magnifying glass icon, top right)
4. Set baud rate to **115200**
5. Press the reset button on ESP32-CAM if needed
6. Note the IP address displayed
7. Open that IP in a web browser

## Troubleshooting

### No COM port appears
- Use a DATA cable, not charge-only
- Install CH340 driver (link above)
- Try a different USB port
- Check Device Manager for errors

### Upload fails
- Make sure correct board is selected (AI Thinker ESP32-CAM)
- Try lowering upload speed: Tools > Upload Speed > 115200
- Press and hold the BOOT button while uploading starts

### Camera not working
- Check ribbon cable is fully inserted
- Ensure board is set to AI Thinker ESP32-CAM
- Check Serial Monitor for error messages

### Servos not responding
- Verify common ground connection
- Check signal wires on GPIO 14 (pan) and GPIO 15 (tilt)
- Use external 5V power for servos, not ESP32's 5V pin

### Can't connect to WiFi
- Double-check SSID and password (case-sensitive)
- Move closer to router
- Board will automatically create AP "ESP32-CAM-PanTilt" if WiFi fails

## USB Cable Note

**Charge-only vs Data cables:**
- Charge-only cables have 2 wires (power only)
- Data cables have 4 wires (power + data)
- You MUST use a data cable to program the ESP32

Data cables are typically included with:
- External hard drives
- Arduino kits
- Printers
- Phones (for file transfer)

## Arduino Joystick Setup (Optional)

Add physical joystick control using an Arduino UNO.

### Joystick Hardware

- Arduino UNO
- Analog joystick module (2-axis with button)

### Joystick Wiring to Arduino

| Joystick | Arduino |
|----------|---------|
| VCC | 5V |
| GND | GND |
| VRx | A0 |
| VRy | A1 |
| SW | D2 |

### Arduino to ESP32 Connection

| Arduino | ESP32 |
|---------|-------|
| TX (pin 1) | GPIO13 (IO13) |
| GND | GND |

### Upload Joystick Code

1. Open `arduino_joystick/arduino_joystick.ino`
2. Select board: **Arduino UNO**
3. Select the Arduino's COM port
4. Upload

### Usage

- Move joystick to control pan/tilt
- Press joystick button to center servos
- Web controls and joystick work simultaneously

## Files in This Project

| File | Description |
|------|-------------|
| `esp32_cam_pan_tilt/esp32_cam_pan_tilt.ino` | Main ESP32 sketch |
| `arduino_joystick/arduino_joystick.ino` | Arduino joystick controller |
| `wiring_guide.txt` | Complete wiring diagram |
| `SETUP_GUIDE.md` | This file |
| `README.md` | Project overview |
