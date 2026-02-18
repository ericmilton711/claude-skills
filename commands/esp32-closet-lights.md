# ESP32 LED Closet Lighting Project

Reference documentation for the ESP32-based closet LED lighting system with PIR motion sensor.

---

## Project Overview

- **Controller:** ESP32 DevKit V1 (or compatible)
- **LED Type:** 12V LED Strip (single color)
- **Trigger:** PIR Motion Sensor
- **Features:** Basic on/off operation, automatic timeout

---

## Pinout Reference

### ESP32 GPIO Assignments

| Function          | GPIO Pin | Notes                          |
|-------------------|----------|--------------------------------|
| LED Channel PWM   | GPIO 25  | To 2N3904 base via 1k resistor |
| PIR Sensor        | GPIO 32  | Input, internal pulldown       |
| Status LED        | GPIO 2   | Onboard LED (optional debug)   |

### Power Connections

| Connection          | Details                              |
|---------------------|--------------------------------------|
| ESP32 VIN           | 5V from buck converter               |
| ESP32 GND           | Common ground                        |
| LED Strip V+        | 12V power supply positive            |
| LED Strip control   | To 2N3904 collector                  |

---

## Wiring Diagram (ASCII)

```
                    12V PSU
                   +      -
                   |      |
                   |      +---------> Common GND
                   |           |
              +----+           |
              |                |
         [12V LED Strip]       |
          ctrl   V+            |
          |       |            |
          |       +------------+
          |
       [C] 2N3904
       [E]   |
          +--+-------------------> GND
          |
    [B] GPIO25
        (via 1k resistor)
          |
    [ESP32 DevKit]
      VIN   GND
       |     |
       |     +--------------------> GND
       |
  [Buck Converter]
   IN+  IN-  OUT+  OUT-
    |    |    5V   GND
    |    |
   12V  GND (from PSU)


    PIR SENSOR (HC-SR501)
    +-----------+
    | VCC OUT GND|
    +--+---+---+-+
       |   |   |
      5V  GPIO32  GND
           |
           +-------> ESP32
```

### 2N3904 Wiring Detail

```
  GPIO25 --[1k resistor]-- BASE
                          2N3904 (flat side facing you)
                          E  B  C
                          |     |
                         GND    LED Strip control wire
```

---

## Parts List

| Component                  | Quantity | Notes                           |
|----------------------------|----------|---------------------------------|
| ESP32 DevKit V1            | 1        | Or ESP32-WROOM-32               |
| 12V LED Strip (single color)| 1       | Short strip only (see note)     |
| 2N3904 NPN Transistor      | 1        | For LED control channel         |
| 1k Ohm Resistor            | 1        | Base resistor for transistor    |
| HC-SR501 PIR Sensor        | 1        | Adjustable sensitivity/delay    |
| 12V Power Supply           | 1        | Size for LED strip amperage     |
| LM2596 Buck Converter      | 1        | 12V to 5V for ESP32             |
| Prototype PCB / Perfboard  | 1        | For clean assembly              |
| JST Connectors             | As needed| For modular wiring              |
| Heat Shrink Tubing         | As needed| Wire protection                 |

**Note:** 2N3904 transistors are limited to 200mA. This works for short LED strips (~10-15 segments). For longer strips, upgrade to IRLB8721 or IRLZ44N MOSFETs.

---

## Basic Arduino Code

```cpp
// ESP32 Closet LED Controller
// Trigger: PIR Motion Sensor

#define PIN_LED    25
#define PIN_PIR    32
#define PIN_STATUS 2

// PWM Configuration
#define PWM_FREQ   5000
#define PWM_RES    8

// Timing
#define LIGHT_TIMEOUT_MS  60000  // 60 seconds auto-off

unsigned long lastTriggerTime = 0;
bool lightsOn = false;

void setup() {
  Serial.begin(115200);

  // Configure PWM channel
  ledcSetup(0, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_LED, 0);

  // Configure inputs
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_STATUS, OUTPUT);

  // Start with lights off
  setLight(0);

  Serial.println("Closet LED Controller Ready");
}

void loop() {
  bool motionDetected = digitalRead(PIN_PIR) == HIGH;

  // Trigger lights on motion
  if (motionDetected) {
    if (!lightsOn) {
      lightsOn = true;
      setLight(255);  // Full brightness
      digitalWrite(PIN_STATUS, HIGH);
      Serial.println("Lights ON");
    }
    lastTriggerTime = millis();
  }

  // Auto-off after timeout
  if (lightsOn) {
    if (millis() - lastTriggerTime > LIGHT_TIMEOUT_MS) {
      lightsOn = false;
      setLight(0);
      digitalWrite(PIN_STATUS, LOW);
      Serial.println("Lights OFF (timeout)");
    }
  }

  delay(100);  // Small debounce
}

void setLight(uint8_t brightness) {
  ledcWrite(0, brightness);
}
```

---

## Sensor Configuration

### PIR Sensor (HC-SR501)
- **Sensitivity:** Adjust via potentiometer (clockwise = more sensitive)
- **Time Delay:** Set to minimum (jumper to "L" or lowest pot setting)
- **Trigger Mode:** Use "H" (repeatable trigger) - keeps retriggering while motion continues

---

## Mounting Tips

1. Mount PIR sensor at ~6ft height, angled toward closet entry
2. Keep ESP32 and transistor in ventilated enclosure
3. Use appropriate wire gauge for LED strip current (18-20 AWG typ)
4. Ensure 12V PSU is rated for total LED strip draw + margin

---

## Troubleshooting

| Issue                    | Check                                       |
|--------------------------|---------------------------------------------|
| No lights                | 12V PSU, transistor connections, GPIO output|
| Flickering               | PWM frequency, loose connections            |
| PIR not triggering       | Sensitivity pot, power to sensor (5V)       |
| Transistor getting hot   | Strip drawing too much current (>200mA)     |
| ESP32 not booting        | Buck converter output (5V), connections     |

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
