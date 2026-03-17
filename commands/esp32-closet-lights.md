# ESP32 LED Closet Lighting Project

Reference documentation for the ESP32-based closet LED lighting system with PIR motion sensor.

---

## Current Status / Where We Left Off

- ESP32-S3 is flashed with ESPHome firmware and connects to DIEMILTONHAUS WiFi (IP: 192.168.12.135)
- ESPHome config is in `~/claude-skills/esphome/closet-lights.yaml`
- **Stopped at:** PIR was only getting 2.2V instead of 5V — was underpowered and not triggering
- **Next step:** Fix power supply — buck converter must be set to exactly 5V **before** connecting ESP32. PIR VCC needs a solid 5V. Do NOT adjust buck converter pot while everything is connected.
- **Warning:** Buck converter was accidentally set to 7.8V — always verify output voltage with multimeter before connecting components

---

## Project Overview

- **Controller:** ESP32 DevKit V1 (or compatible)
- **LED Type:** 12V Addressable LED Strip (DI data, labeled +24VDC but runs on 12V)
- **Trigger:** PIR Motion Sensor
- **Firmware:** ESPHome
- **Integration:** Home Assistant (native, auto-discovered)
- **Features:** Motion-triggered on/off, automatic timeout, WiFi, HA control

---

## Pinout Reference

### ESP32 GPIO Assignments

| Function          | GPIO Pin | Notes                          |
|-------------------|----------|--------------------------------|
| LED Strip DI      | GPIO 4   | Via 330Ω series resistor       |
| PIR Sensor        | GPIO 14  | Input, internal pulldown       |
| Status LED        | GPIO 2   | Onboard LED (optional debug)   |

### ESP32-S3-N16R8 Variant Notes

The S3-N16R8 (16MB octal flash, 8MB octal PSRAM) works for this project but requires pin changes:

- **GPIO 27-37 are occupied internally** by octal flash and PSRAM — do not use
- **GPIO 32 conflicts** with flash — use GPIO 14 for PIR instead
- **GPIO 25 is NOT available** on S3-WROOM-1 — use GPIO 4 for LED strip DI instead
- Change board definition in ESPHome config (see below)

**S3-N16R8 Pin Assignments:**

| Function     | GPIO | Notes                          |
|--------------|------|--------------------------------|
| LED Strip DI | 4    | GPIO 25 not available on S3    |
| PIR Sensor   | 14   | Changed from 32 (flash conflict)|
| Status LED   | 2    | Verify onboard LED on your board|

**ESPHome board definition for S3:**
```yaml
esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: arduino
```

### Power Connections

| Connection          | Details                              |
|---------------------|--------------------------------------|
| ESP32 VIN           | 5V from buck converter               |
| ESP32 GND           | Common ground                        |
| LED Strip V+        | 12V power supply positive            |
| LED Strip GND       | Common ground                        |
| LED Strip DI        | GPIO4 via 330Ω resistor              |

---

## Wiring Diagram (ASCII)

```
12V PSU
+12V ─────────────────────────────────────────────┐
 GND ─────────────────────────────── GND RAIL ─────┤
                                                    │
                   ┌──── BUCK CONVERTER ────┐       │
+12V ──────────────┤ IN+             OUT+ ──┼──── ESP32 VIN (5V)
GND RAIL ──────────┤ IN-             OUT- ──┼──── GND RAIL
                   └────────────────────────┘      │
                                                    │
                   ┌──── ESP32 ────┐                │
    5V (buck) ─────┤ VIN      GND ─┼────────────────┤
       GPIO25 ─────┤               │                │
       GPIO32 ─────┤               │                │
                   └───────────────┘                │
                                                    │
GPIO4  ──[330Ω]──────────────── LED Strip DI        │
                                                    │
LED STRIP                                           │
  +24VDC (V+) ──────────────────────────── +12V     │
  DI ──────────────── GPIO25 via 330Ω               │
  GND ──────────────────────────────────────────────┤
                                                    │
PIR HC-SR501                                        │
  VCC ──────────────────────────────────── 5V       │
  OUT ──────────────────────────────────── GPIO32   │
  GND ──────────────────────────────────────────────┤
                                                    │
════════════════════ COMMON GROUND ═════════════════┘
                     (PSU negative)
```

---

## Parts List

| Component                   | Quantity | Notes                           |
|-----------------------------|----------|---------------------------------|
| ESP32 DevKit V1             | 1        | Or ESP32-WROOM-32               |
| 12V Addressable LED Strip   | 1        | DI data pin, labeled +24VDC     |
| 330 Ohm Resistor            | 1        | Series resistor on DI line      |
| HC-SR501 PIR Sensor         | 1        | Adjustable sensitivity/delay    |
| 12V Power Supply            | 1        | Size for LED strip amperage     |
| LM2596 Buck Converter       | 1        | 12V to 5V for ESP32             |
| Breadboard                  | 1        | For initial testing/prototyping |
| Prototype PCB / Perfboard   | 1        | For final permanent assembly    |
| Shielded CAT6 Cable         | As needed| 8 conductors — runs to PIR, LED strip, and ESP32 box; bundle pairs for power |
| Heat Shrink Tubing          | As needed| Wire protection                 |

---

## CAT6 Wiring Notes

Shielded CAT6 is an excellent substitute for JST connectors. Each cable has 8 conductors (4 pairs) — more than enough for all signal and power runs.

### Conductor Allocation

| Run                    | Conductors Needed | Notes                                      |
|------------------------|-------------------|--------------------------------------------|
| PIR sensor             | 3                 | VCC, GND, OUT                              |
| LED strip DI line      | 2                 | Data + GND reference                       |
| Power (12V or 5V)      | Bundle pairs      | Use 2-4 conductors in parallel for current |

### Tips
- **Shielding** helps keep the DI data line clean from interference — connect shield to GND at one end
- **Connections:** Strip individual conductors and solder directly to perfboard — no additional connectors needed
- **Power:** CAT6 conductors are 23-24 AWG — fine for signal lines; bundle 2-4 conductors in parallel for any power runs to handle current safely

---

## ESPHome Configuration

### First-Time Setup

1. Install ESPHome: `pip install esphome`
2. Create `secrets.yaml` in your ESPHome config folder:

```yaml
wifi_ssid: "YourNetworkName"
wifi_password: "YourWiFiPassword"
api_encryption_key: "generate-at-esphome.io/encrypting"
ota_password: "choose-a-password"
fallback_password: "fallback-password"
```

3. Flash via USB the first time: `esphome run closet-lights.yaml`
4. All future updates are OTA over WiFi

---

### ESPHome YAML (`closet-lights.yaml`)

```yaml
esphome:
  name: closet-lights
  friendly_name: Closet Lights

esp32:
  board: esp32dev
  framework:
    type: arduino

logger:

api:
  encryption:
    key: !secret api_encryption_key

ota:
  - platform: esphome
    password: !secret ota_password

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  ap:
    ssid: "Closet Lights Fallback"
    password: !secret fallback_password

captive_portal:

# LED strip output (PWM via LEDC)
output:
  - platform: ledc
    pin: GPIO4
    id: led_output

# Expose as a dimmable light in Home Assistant
light:
  - platform: monochromatic
    name: "Closet Light"
    output: led_output
    id: closet_light
    restore_mode: ALWAYS_OFF

# PIR motion sensor
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO32
      mode:
        input: true
        pulldown: true
    name: "Closet PIR"
    id: closet_pir
    device_class: motion
    on_press:
      - light.turn_on: closet_light
      - script.stop: auto_off
    on_release:
      - script.execute: auto_off

# Auto-off timer — restarts if motion stops then resumes
script:
  - id: auto_off
    then:
      - delay: 60s
      - light.turn_off: closet_light

# Onboard status LED
status_led:
  pin:
    number: GPIO2
    inverted: true
```

---

### How It Works

1. Motion detected → PIR goes HIGH → light turns on, auto-off timer cancelled
2. Motion stops → PIR goes LOW → 60 second countdown starts
3. If motion detected again before 60s → timer resets, light stays on
4. After 60s with no motion → light turns off
5. Home Assistant can also turn light on/off and adjust brightness manually at any time

---

## Sensor Configuration

### PIR Sensor (HC-SR501)
- **Sensitivity:** Adjust via potentiometer (clockwise = more sensitive)
- **Time Delay:** Set to minimum (jumper to "L" or lowest pot setting) — ESPHome handles the timeout
- **Trigger Mode:** Use "H" (repeatable trigger) - keeps retriggering while motion continues

---

## Mounting Tips

1. Mount PIR sensor at ~6ft height, angled toward closet entry
2. Keep ESP32 and transistor in ventilated enclosure
3. Use appropriate wire gauge for LED strip current (18-20 AWG typ)
4. Ensure 12V PSU is rated for total LED strip draw + margin

---

## Troubleshooting

| Issue                        | Check                                             |
|------------------------------|---------------------------------------------------|
| No lights                    | 12V PSU, DI connection, 330Ω resistor, GPIO25     |
| Flickering / corrupt colors  | Bad DI connection, missing/wrong resistor         |
| PIR not triggering           | Sensitivity pot, power to sensor (5V)             |
| ESP32 not booting            | Buck converter output (5V), connections           |
| Not showing in Home Assistant| Check API encryption key, HA ESPHome integration  |
| OTA not working              | ESP32 must be on same WiFi network as HA          |

---

## Testing Components with Power Supply Trainer

### Testing the LED Strip

1. Set power supply to **12V**
2. Connect **V+ wire** to **positive (+)** terminal
3. Touch control wire to **ground (-)** to test:

```
Power Supply Trainer
+12V        GND
 |           |
 |           +--- touch control wire here to test
 |
 +--- V+ wire
```

If the strip lights up, it's working correctly.

---

### Testing the PIR Sensor

**Setup:**
1. Set power supply to **5V**
2. Wire a test LED with 330 ohm resistor

**Connections:**
```
+5V -------- VCC (PIR)
GND -------- GND (PIR) [direct connection]
         |
         +-- LED cathode(-) [direct to GND]

OUT (PIR) --[330Ω]-- LED anode(+)
```

**LED orientation:**
- **Anode (+)** = longer leg → connects to resistor
- **Cathode (-)** = shorter leg, flat edge → connects to GND

```
    LED
   _____
  /     \
 |       |
 |  /|   |
  \_|___/ <-- flat spot = cathode side
    |  |
    |  +-- shorter = cathode (-)
    +---- longer = anode (+)
```

**Test procedure:**
1. Power on and **wait 30-60 seconds** (don't move)
2. LED should turn off after calibration
3. Wave hand in front of sensor
4. LED should light up

---

### PIR Sensor Pin Identification (No Labels)

Looking at sensor with dome facing you:

```
      +-------------+
      |   (dome)    |
      |     PIR     |
      +-------------+
         |  |  |
        GND OUT VCC
        (-)     (+)
```

**If no labels, identify by PCB:**
- **VCC** - trace leads to voltage regulator
- **GND** - trace leads to ground plane / capacitor negative
- **OUT** - usually the middle pin

**Safe test if unsure:**
1. Set power supply to 5V with **current limit at 20mA**
2. Leave middle pin (OUT) unconnected
3. Try outer pins to +5V and GND
4. If nothing happens, swap them
5. Correct = small red power LED on board lights up

---

### PIR Potentiometer Identification

```
+---------------------------+
|                           |
|   [Sx]            [Tx]    |
|  SENSITIVITY     TIME     |
|   (range)       (delay)   |
|                           |
|        (Fresnel dome)     |
+---------------------------+
          |  |  |
         pins
```

| Pot (position)  | Clockwise         | Counter-clockwise |
|-----------------|-------------------|-------------------|
| Left (Sx)       | More range (~7m)  | Less range (~3m)  |
| Right (Tx)      | Longer (~5 min)   | Shorter (~3 sec)  |

**For testing:** Turn both pots **counter-clockwise** for shorter delay and less sensitivity.
**For final install:** Set time delay pot to minimum — ESPHome controls the 60s timeout instead.
