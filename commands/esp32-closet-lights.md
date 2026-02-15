# ESP32 LED Closet Lighting Project

Reference documentation for the ESP32-based closet LED lighting system with PIR motion sensor.

---

## Project Overview

- **Controller:** ESP32 DevKit V1 (or compatible)
- **LED Type:** 12V RGB LED Strip (non-addressable, common anode)
- **Trigger:** PIR Motion Sensor
- **Features:** Basic on/off operation, automatic timeout

---

## Pinout Reference

### ESP32 GPIO Assignments

| Function          | GPIO Pin | Notes                          |
|-------------------|----------|--------------------------------|
| Red Channel PWM   | GPIO 25  | To 2N3904 base via 1k resistor |
| Green Channel PWM | GPIO 26  | To 2N3904 base via 1k resistor |
| Blue Channel PWM  | GPIO 27  | To 2N3904 base via 1k resistor |
| PIR Sensor        | GPIO 32  | Input, internal pulldown       |
| Status LED        | GPIO 2   | Onboard LED (optional debug)   |

### Power Connections

| Connection        | Details                              |
|-------------------|--------------------------------------|
| ESP32 VIN         | 5V from buck converter               |
| ESP32 GND         | Common ground                        |
| LED Strip V+      | 12V power supply positive            |
| LED Strip R/G/B   | To 2N3904 collectors                 |

---

## Wiring Diagram (ASCII)

```
                    12V PSU
                   +      -
                   |      |
                   |      +----+----+----+-----> Common GND
                   |           |    |    |
              +----+           |    |    |
              |                |    |    |
         [12V RGB Strip]       |    |    |
          R    G    B   V+     |    |    |
          |    |    |    |     |    |    |
          |    |    |    +-----+    |    |
          |    |    |               |    |
       [C] 2N3904 2N3904 2N3904     |    |
       [E]   |       |       |      |    |
          +--+-------+-------+------+    |
          |                              |
          +---> GND                      |
                                         |
    [B] GPIO25  GPIO26  GPIO27           |
        (via 1k resistors)               |
          |       |       |              |
          +-------+-------+              |
                  |                      |
            [ESP32 DevKit]               |
              VIN   GND                  |
               |     |                   |
               |     +-------------------+
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
  GPIO --[1k resistor]-- BASE
                        2N3904 (flat side facing you)
                        E  B  C
                        |     |
                       GND    LED Strip color wire (R, G, or B)
```

---

## Parts List

| Component                  | Quantity | Notes                           |
|----------------------------|----------|---------------------------------|
| ESP32 DevKit V1            | 1        | Or ESP32-WROOM-32               |
| 12V RGB LED Strip          | 1        | Short strip only (see note)     |
| 2N3904 NPN Transistor      | 3        | One per color channel           |
| 1k Ohm Resistors           | 3        | Base resistors for transistors  |
| HC-SR501 PIR Sensor        | 1        | Adjustable sensitivity/delay    |
| 12V Power Supply           | 1        | Size for LED strip amperage     |
| LM2596 Buck Converter      | 1        | 12V to 5V for ESP32             |
| Prototype PCB / Perfboard  | 1        | For clean assembly              |
| JST Connectors             | As needed| For modular wiring              |
| Heat Shrink Tubing         | As needed| Wire protection                 |

**Note:** 2N3904 transistors are limited to 200mA per channel. This works for short LED strips (~10-15 segments). For longer strips, upgrade to IRLB8721 or IRLZ44N MOSFETs.

---

## Basic Arduino Code

```cpp
// ESP32 Closet LED Controller
// Trigger: PIR Motion Sensor

#define PIN_RED    25
#define PIN_GREEN  26
#define PIN_BLUE   27
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

  // Configure PWM channels
  ledcSetup(0, PWM_FREQ, PWM_RES);  // Red
  ledcSetup(1, PWM_FREQ, PWM_RES);  // Green
  ledcSetup(2, PWM_FREQ, PWM_RES);  // Blue

  ledcAttachPin(PIN_RED, 0);
  ledcAttachPin(PIN_GREEN, 1);
  ledcAttachPin(PIN_BLUE, 2);

  // Configure inputs
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_STATUS, OUTPUT);

  // Start with lights off
  setColor(0, 0, 0);

  Serial.println("Closet LED Controller Ready");
}

void loop() {
  bool motionDetected = digitalRead(PIN_PIR) == HIGH;

  // Trigger lights on motion
  if (motionDetected) {
    if (!lightsOn) {
      lightsOn = true;
      setColor(255, 255, 255);  // White light
      digitalWrite(PIN_STATUS, HIGH);
      Serial.println("Lights ON");
    }
    lastTriggerTime = millis();
  }

  // Auto-off after timeout
  if (lightsOn) {
    if (millis() - lastTriggerTime > LIGHT_TIMEOUT_MS) {
      lightsOn = false;
      setColor(0, 0, 0);
      digitalWrite(PIN_STATUS, LOW);
      Serial.println("Lights OFF (timeout)");
    }
  }

  delay(100);  // Small debounce
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(0, r);
  ledcWrite(1, g);
  ledcWrite(2, b);
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
2. Keep ESP32 and transistors in ventilated enclosure
3. Use appropriate wire gauge for LED strip current (18-20 AWG typ)
4. Ensure 12V PSU is rated for total LED strip draw + margin

---

## Troubleshooting

| Issue                    | Check                                       |
|--------------------------|---------------------------------------------|
| No lights                | 12V PSU, transistor connections, GPIO output|
| Flickering               | PWM frequency, loose connections            |
| One color missing        | Individual transistor, LED strip channel    |
| PIR not triggering       | Sensitivity pot, power to sensor (5V)       |
| Transistor getting hot   | Strip drawing too much current (>200mA)     |
| ESP32 not booting        | Buck converter output (5V), connections     |

---

## Testing Components with Power Supply Trainer

### Testing the LED Strip

1. Set power supply to **12V**
2. Connect **black wire** (V+) to **positive (+)** terminal
3. Touch color wires to **ground (-)** to test each channel:

| Touch to GND | Result       |
|--------------|--------------|
| Red wire     | Red lights   |
| Green wire   | Green lights |
| Blue wire    | Blue lights  |
| All three    | White light  |

```
Power Supply Trainer
+12V        GND
 |           |
 |           +--- touch R, G, or B wires here to test
 |
 +--- Black wire (V+)
```

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
