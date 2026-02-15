# Galaxy Watch Charging Dock LED Project

## Project Overview
A charging dock for Samsung Galaxy Watch with LED illumination that activates when the watch is placed on the charger. Features a semi-transparent 3D printed block that glows from beneath with an orange/amber breathing effect.

## Date Started
February 2026

---

## Current Configuration

| Setting | Value |
|---------|-------|
| Detection | TTP-223 Capacitive Touch Sensor (proximity mode) |
| LED Effect | Breathing (slow fade in/out) |
| Color | Orange/Amber `{255, 80, 0}` |
| Brightness | 150/255 |

---

## Parts List

### Have
- Arduino Nano
- WS2812B 7-LED Ring
- TTP-223 Capacitive Touch Module
- Jumper wires
- Samsung Galaxy Watch magnetic charger
- USB-C cord with power adapter
- Semi-transparent 3D printed block

### Need
- (None - all parts available)

---

## Wiring Summary

```
Arduino Nano    Component        Pin
============    =========        ===
5V              LED Ring         VCC
5V              TTP-223          VCC
GND             LED Ring         GND
GND             TTP-223          GND
D6              LED Ring         DIN
D2              TTP-223          SIG
```

Total wires needed: 5 jumper wires (can share 5V and GND)

---

## Physical Layout
```
+------------------------+
|   Galaxy Watch         |  <- Watch sits on top
+------------------------+
|   Magnetic Charger     |  <- Samsung charger puck
+------------------------+
|   TTP-223 Sensor       |  <- Detects watch placement
+------------------------+
|   3D Printed Block     |  <- Semi-transparent diffuser
+------------------------+
|   WS2812B LED Ring     |  <- LEDs face upward
+------------------------+
|   Arduino Nano         |  <- Controller (hidden)
+------------------------+
```

---

## How It Works

1. TTP-223 capacitive sensor detects when watch is placed above it
2. Arduino reads sensor state (HIGH = detected, LOW = not detected)
3. When watch detected, LEDs display breathing orange glow
4. When watch removed, LEDs turn off
5. Debouncing prevents flicker during transitions

---

## Files

```
~/.claude/skills/galaxy-watch-dock/
├── galaxy_watch_dock/
│   └── galaxy_watch_dock.ino    <- Main Arduino code
├── WIRING.txt                    <- Detailed wiring guide
└── PROJECT_LOG.md                <- This file
```

---

## Setup Steps

1. **Install Arduino IDE** from arduino.cc

2. **Install library**: Sketch → Include Library → Manage Libraries → Search "Adafruit NeoPixel" → Install

3. **Wire components** per WIRING.txt

4. **Upload code**:
   - Tools → Board → Arduino Nano
   - Tools → Processor → ATmega328P (try "Old Bootloader" if fails)
   - Tools → Port → (your COM port)
   - Click Upload

5. **Position sensor**: Mount TTP-223 facing up, under where watch sits

6. **Test**: Place watch on charger, LEDs should breathe. Remove watch, LEDs off.

---

## TTP-223 Tips

- Can sense through ~3-5mm of non-metallic material
- Some modules have A/B jumper for toggle vs momentary mode
- Position B (momentary) is better for proximity detection
- May need adjustment if charger magnets interfere
- Open Serial Monitor (115200 baud) to debug detection

---

## Customization Options

| Parameter | Default | Description |
|-----------|---------|-------------|
| `baseColor[3]` | `{255, 80, 0}` | RGB color (orange/amber) |
| `BRIGHTNESS` | 150 | Overall brightness (0-255) |
| `BREATH_SPEED` | 10 | Ms between steps (lower = faster) |
| `SENSOR_PIN` | D2 | Arduino pin for TTP-223 |
| `LED_PIN` | D6 | Arduino pin for LED ring |

---

## Project History

### February 2026 - Initial Planning
- Explored INA219 current sensing (too complex, requires cable cutting)
- Explored reed switch (uncertain due to permanent magnets)
- Explored IR proximity sensor (reliable but needs ordering)
- Ordered WS2812B LED ring and Samsung charger

### February 2026 - Reconfigured
- Switched to TTP-223 capacitive touch sensor (already owned)
- Simplified wiring (5 wires total vs 8+ for INA219)
- No additional parts needed
- Updated all code and documentation
