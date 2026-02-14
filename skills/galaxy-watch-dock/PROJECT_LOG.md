# Galaxy Watch Charging Dock LED Project

## Project Overview
A charging dock for Samsung Galaxy Watch with LED illumination that activates when the watch is placed on the charger. Features a semi-transparent 3D printed block that glows from beneath with an orange/amber breathing effect.

## Date Started
February 2026

---

## Parts Purchased

### Already Owned
- **Arduino Nano** - microcontroller
- **Breadboard** - for prototyping
- **USB Hub** - power source for components
- **Semi-transparent 3D printed block** - diffuses LED light

### Ordered from Amazon
1. **WS2812B 7-LED Ring** (DIYmall)
   - 7 individually addressable RGB LEDs
   - Link: https://www.amazon.com/DIYmall-Integrated-Individually-Addressable-Raspberry/dp/B0C7C9HP9R

2. **Samsung Galaxy Watch Charger** (25W dual USB-C)
   - Magnetic wireless charging
   - Link: https://www.amazon.com/Samsung-Charger-Wireless-Charging-MagneticWireless/dp/B0DTSQR99X

---

## Detection Method Options Explored

### Option 1: INA219 Current Sensor (Complex)
- Measures current flowing to Samsung charger
- Detects when watch is drawing power (200-500mA)
- **Pros:** Accurate, detects actual charging
- **Cons:** Requires cutting Samsung cable or USB breakout boards, complex wiring
- **Status:** Code written but deemed too complex

### Option 2: Reed Switch (Simple but Uncertain)
- Detects magnetic field from charger
- **Pros:** Simple wiring (3 wires), cheap (~$2)
- **Cons:** Samsung charger has permanent magnets - may trigger constantly
- **Link:** https://www.amazon.com/Sensor-Module-Magnetic-Normally-Arduino/dp/B07S58HH3Q
- **Status:** Uncertain if it will work - needs testing

### Option 3: IR Proximity Sensor (Recommended)
- Detects physical presence of watch above charger
- **Pros:** Reliable, simple, doesn't depend on magnets or current
- **Cons:** Needs correct positioning
- **Status:** Not yet explored in detail

### Option 4: Always On
- LEDs breathe whenever Arduino is powered
- **Pros:** Simplest possible
- **Cons:** Not automatic
- **Status:** Fallback option

---

## Code Files Created

### Main Code: `galaxy_watch_dock.ino`
- Location: `C:\Users\ericm\Skills\galaxy-watch-dock\galaxy_watch_dock\`
- Features:
  - INA219 current detection with hysteresis (50mA on / 30mA off)
  - 5-sample averaging for stable readings
  - Orange/amber breathing effect
  - LEDs off when not charging, breathing pulse when charging

### Test Code: `led_test.ino` (Deleted)
- Was a simple breathing test without sensor
- Deleted because Arduino IDE combines all .ino files in same folder

---

## Wiring Documentation

### Basic LED Ring Wiring
```
Arduino Nano          WS2812B LED Ring
============          ================
    5V  ──┬─────────► VCC
          │
        [+│-]         100-1000µF capacitor (recommended)
          │
    GND ──┴─────────► GND
    D6  ────────────► DIN
```

### INA219 Wiring (if using current sensing)
```
Arduino Nano          INA219 Sensor
============          =============
    5V  ────────────► VCC
    GND ────────────► GND
    A4  ────────────► SDA
    A5  ────────────► SCL

INA219 VIN+ ◄──── USB 5V Power
INA219 VIN- ────► Samsung Charger 5V (red wire)
```

### Reed Switch Wiring (if using magnetic detection)
```
Reed Switch Module    Arduino Nano
==================    ============
     VCC  ──────────►  5V
     GND  ──────────►  GND
     DO   ──────────►  D2
```

---

## Physical Layout
```
+------------------------+
|   Galaxy Watch         |  ← Watch sits on top
+------------------------+
|   Magnetic Charger     |  ← Samsung charger puck
+------------------------+
|   3D Printed Block     |  ← Semi-transparent, glows from below
+------------------------+
|   WS2812B LED Ring     |  ← LEDs face upward into block
+------------------------+
|   Arduino Nano         |  ← Hidden in base/underneath
+------------------------+
```

---

## LED Effect Settings
- **Color:** Orange/Amber `{255, 80, 0}`
- **Effect:** Slow breathing (fade in/out)
- **Speed:** 10ms between brightness steps
- **Brightness:** 150 (out of 255)

---

## Libraries Required
1. **Adafruit NeoPixel** - for WS2812B LED control
2. **Adafruit INA219** - for current sensing (if using that method)

Install via: Arduino IDE → Sketch → Include Library → Manage Libraries

---

## Next Steps
1. Decide on detection method:
   - Test reed switch with Samsung charger magnets
   - OR get IR proximity sensor for reliable detection
   - OR simplify to always-on or button toggle
2. Order chosen sensor
3. Wire up and test
4. Build final enclosure

---

## Parts Still Needed
- [ ] Detection sensor (reed switch, IR proximity, or other)
- [ ] 100-1000µF capacitor (recommended for LED power smoothing)
- [ ] USB-C breakout boards (only if using INA219 without cutting cable)

---

## Conversation Summary

### Key Decisions Made
1. Arduino Nano chosen as microcontroller (simple, cheap)
2. Orange/amber color chosen for LEDs (not blue)
3. Slow breathing effect preferred
4. INA219 approach deemed too complex
5. Reed switch may not work due to permanent magnets in charger
6. IR proximity sensor recommended as most reliable option

### Files in Project Folder
```
C:\Users\ericm\Skills\galaxy-watch-dock\
├── galaxy_watch_dock\
│   └── galaxy_watch_dock.ino    ← Main Arduino code (INA219 version)
├── WIRING.txt                    ← Detailed wiring instructions
└── PROJECT_LOG.md                ← This file
```
