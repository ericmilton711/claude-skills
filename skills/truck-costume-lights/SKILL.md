# Truck Costume Lights

Button-activated red strobe + siren for a kid's truck Halloween costume. Built 2026-10-31.

## Parts

- Arduino Nano
- 12V 8xAA battery pack
- Red strobe light (12V)
- Piezo buzzer
- N-channel MOSFET (any TO-220: IRF520, IRF530, IRF540, etc.)
- 1x 220 ohm resistor
- 4-pin push button
- Breadboard + jumper wires

## Wiring

### Power
- Battery (+) → Nano Vin
- Battery (-) → Nano GND

### Strobe (via MOSFET)
MOSFET pinout (TO-220, metal tab facing away, legs down, left to right):
1. **Gate** → 220 ohm resistor → Nano D3
2. **Drain** → Strobe light (-)
3. **Source** → Battery (-)

Strobe light (+) → Battery (+)

**Important:** Strobe (-) must go to MOSFET Drain, NOT directly to battery ground. The MOSFET is the switch.

### Buzzer
- Buzzer (+) → Nano D5
- Buzzer (-) → Nano GND

### Button
- One leg → Nano D7
- Opposite leg → Nano GND
- No resistor needed (code uses INPUT_PULLUP)
- Place across breadboard center gap

## Behavior

- **Button released:** Everything off
- **Button held:** Strobe flashes + siren wails (600-1200 Hz sweep, like a fire truck)

## Sketch

```cpp
// Truck costume strobe + siren - Halloween 2026
// Nano + 12V battery + red strobe via MOSFET on pin 3
// Piezo buzzer on pin 5

const int STROBE_PIN = 3;
const int BUZZER_PIN = 5;
const int BUTTON_PIN = 7;

void setup() {
  pinMode(STROBE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Button held: strobe + siren
    digitalWrite(STROBE_PIN, HIGH);
    for (int freq = 600; freq < 1200; freq += 15) {
      tone(BUZZER_PIN, freq);
      delay(8);
    }
    digitalWrite(STROBE_PIN, LOW);
    delay(40);
    digitalWrite(STROBE_PIN, HIGH);
    for (int freq = 1200; freq > 600; freq -= 15) {
      tone(BUZZER_PIN, freq);
      delay(8);
    }
    digitalWrite(STROBE_PIN, LOW);
  } else {
    // Button released: everything off
    digitalWrite(STROBE_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(50);
  }
}
```

## Flash Command

```bash
arduino-cli compile --fqbn arduino:avr:nano truck_strobe
arduino-cli upload -p COM14 --fqbn arduino:avr:nano truck_strobe
```

## Troubleshooting

- **Strobe won't turn off:** Strobe (-) is wired to battery ground instead of MOSFET Drain. Route it through the MOSFET.
- **Everything runs backwards (on when button not pressed):** Button should connect D7 to GND, not to Vin/12V.
- **Buzzer works but no strobe:** Check MOSFET Gate has the 220 ohm resistor to D3, and Drain goes to strobe (-).
