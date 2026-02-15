# Galaxy Watch Dock LED Skill

ESP32/Arduino project for a Galaxy Watch charging dock with LED illumination.

## Overview
A charging dock for Samsung Galaxy Watch with WS2812B LED ring that creates an orange/amber breathing glow when the watch is placed on the charger.

## Hardware
- Arduino Nano
- WS2812B 7-LED Ring
- TTP-223 Capacitive Touch Module
- Samsung Galaxy Watch magnetic charger
- Semi-transparent 3D printed block (diffuser)

## Wiring

```
Arduino Nano          TTP-223 Module
============          ==============
    5V  -------------> VCC
    GND -------------> GND
    D2  -------------> I/O


Arduino Nano          WS2812B LED Ring
============          ================
    5V  -------------> VCC
    GND -------------> GND
    D6  -------------> DIN
```

## Detection Method
TTP-223 capacitive touch sensor in proximity mode. Detects when watch is placed on charger by sensing capacitance change.

**Note:** The Samsung charger has internal metal coils. Position the TTP-223 to the side to detect the watch band, or test if it works through the charger base.

## LED Effect
- Color: Orange/Amber `{255, 80, 0}`
- Effect: Breathing (slow fade in/out)
- Brightness: 150/255

## Required Library
- Adafruit NeoPixel (install via Arduino Library Manager)

## Files
- `galaxy_watch_dock/galaxy_watch_dock.ino` - Main Arduino code
- `WIRING.txt` - Detailed wiring guide
- `PROJECT_LOG.md` - Project history and notes

## Upload Instructions
1. Open `galaxy_watch_dock.ino` in Arduino IDE
2. Tools → Board → Arduino Nano
3. Tools → Processor → ATmega328P
4. Tools → Port → (your COM port)
5. Click Upload

## Customization
In the .ino file:
- `baseColor[3]` - RGB color values
- `BRIGHTNESS` - 0-255
- `BREATH_SPEED` - ms between steps (lower = faster)
