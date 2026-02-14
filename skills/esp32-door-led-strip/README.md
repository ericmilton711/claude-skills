# ESP32 Door LED Strip

Motion-activated LED strip for doorways using ESP32 and PIR sensor. Battery-optimized with deep sleep (~270 days on 10,000mAh power bank).

## Features

- PIR motion detection triggers LED strip
- Rainbow animation while active
- Auto-off after 15 seconds of no motion
- Deep sleep mode (~10µA power consumption)
- Sweep-on and fade-off effects

## Parts List

| Part | Description | Price |
|------|-------------|-------|
| ESP32 Dev Board | ESP32-WROOM-32 or DevKitC | $6-10 |
| WS2812B LED Strip | 60 LEDs/m, 5V | $8-15 |
| HC-SR501 PIR Sensor | Passive infrared motion sensor | $2-3 |
| Anker Power Bank | 10,000mAh, 5V 2A output | $15-25 |
| Jumper Wires | Female-to-female | $3-5 |
| USB Cable | For ESP32 | $3-5 |

**Optional:**
- 1000µF capacitor (LED surge protection)
- 330Ω resistor (data line)

**Total: ~$40-70**

## Wiring

```
Power Bank (5V)
    │
    ├───► LED Strip VCC (red)
    ├───► ESP32 VIN
    ├───► HC-SR501 VCC
    │
    └───► GND ◄──┬── ESP32 GND
                 ├── LED Strip GND
                 └── HC-SR501 GND

ESP32 GPIO 13 ──► HC-SR501 OUT (signal)
ESP32 GPIO 5 ───► LED Strip Data (green)
```

## Configuration

Edit `src/config.h` to customize:

```cpp
#define LED_PIN         5       // GPIO for LED data
#define NUM_LEDS        60      // Number of LEDs
#define DEFAULT_BRIGHTNESS  80  // 0-255
#define PIR_PIN         GPIO_NUM_13  // PIR sensor pin
#define LED_ON_TIME     15000   // LEDs stay on for 15 seconds
```

## Installation

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
2. Open this folder in PlatformIO
3. Edit `src/config.h` with your settings
4. Build and upload to ESP32

## PIR Sensor Setup (HC-SR501)

- Set jumper to "L" position (single trigger mode)
- Adjust sensitivity pot (clockwise = more sensitive)
- Adjust time pot to minimum (let ESP32 handle timing)

## Power Consumption

| State | Current | 10,000mAh Battery |
|-------|---------|-------------------|
| Deep sleep | ~0.15mA | ~270 days |
| Active (LEDs on) | ~800mA | ~12 hours continuous |

Realistic usage (10 triggers/day): **~9 months battery life**

## File Structure

```
esp32-door-led-strip/
├── platformio.ini      # PlatformIO configuration
├── parts-list.txt      # Printable parts checklist
├── README.md           # This file
└── src/
    ├── config.h        # Configuration settings
    └── main.cpp        # Main program
```
