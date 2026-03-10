# ESP32 LED Closet Lighting Project

Reference documentation for the ESP32-based closet LED lighting system with PIR motion sensor.

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
| LED Strip DI      | GPIO 25  | Via 330Ω series resistor       |
| PIR Sensor        | GPIO 32  | Input, internal pulldown       |
| Status LED        | GPIO 2   | Onboard LED (optional debug)   |

### Power Connections

| Connection          | Details                              |
|---------------------|--------------------------------------|
| ESP32 VIN           | 5V from buck converter               |
| ESP32 GND           | Common ground                        |
| LED Strip V+        | 12V power supply positive            |
| LED Strip GND       | Common ground                        |
| LED Strip DI        | GPIO25 via 330Ω resistor             |

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
GPIO25 ──[330Ω]──────────────── LED Strip DI        │
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
| Prototype PCB / Perfboard   | 1        | For clean assembly              |
| JST Connectors              | As needed| For modular wiring              |
| Heat Shrink Tubing          | As needed| Wire protection                 |

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
    pin: GPIO25
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
