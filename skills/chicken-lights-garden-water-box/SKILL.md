# Chicken Lights and Garden Water Box

## Project Overview
3D-printed weather-resistant enclosure for the chicken run LED timer system and chicken water solenoid. Mounts inside the barn on a wall.

**Plan change (2026-04-29):** Solenoid controls chicken water from rain barrel only. Garden drip line hooks to outdoor spout (gravity PSI too low for drip line).

**Major update (2026-05-30):** LED control migrated from Raspberry Pi to Arduino Nano + DS3231 RTC + IRLB8721 MOSFET. Added SSD1306 OLED display and battery voltage monitoring.

Related skill: `homestead-automation` — the electronics, wiring, code, and cron jobs that go inside this box.

---

## LED Timer System

### Schedule

| Window | Time | LEDs |
|--------|------|------|
| Morning | 5:00 AM - 6:59 AM | ON |
| Midday | 7:00 AM - 1:59 PM | OFF |
| Afternoon/Evening | 2:00 PM - 11:58 PM | ON |
| Night | 11:59 PM - 4:59 AM | OFF |

### Hardware

| Component | Spec | Role |
|-----------|------|------|
| Arduino Nano | CH340 USB, ATmega328 (new bootloader) | Timer controller |
| DS3231 RTC | "For Pi" module, CR2032 backup | Precision clock |
| IRLB8721 | N-Channel MOSFET, TO-220, from GuuYeBe kit | LED circuit switch |
| SSD1306 OLED | 0.96" 128x64, I2C, address 0x3C | Status display |
| 5x CHANZON UV LEDs | 3W 365nm | Bug attraction |
| 5x Blue LEDs | 1W 460-465nm | Bug attraction |
| DROK 5A Buck | 24V buck, steps to LED voltage | LED power |
| D-Planet 5A Buck | 12V to 5V | Nano power |
| SOLPERK 20W Panel | With 8A PWM charge controller | Solar charging |
| DieHard Marine 24M | 12V flooded, 50Ah | Main power |
| 10kΩ resistor | Gate pulldown | Keeps LEDs off if Nano loses power |
| 10kΩ + 4.7kΩ resistors | Voltage divider on A0 | Battery voltage monitoring |

### Wiring

#### Nano Pin Connections

| Nano Pin | Connects To |
|----------|-------------|
| 5V | D-Planet buck output, DS3231 VCC, OLED VCC |
| GND | Common ground bus |
| D9 | IRLB8721 Gate (Pin 1, left) |
| A0 | Voltage divider junction (10kΩ/4.7kΩ) |
| A4 (SDA) | DS3231 SDA, OLED SDA (shared I2C bus) |
| A5 (SCL) | DS3231 SCL, OLED SCL (shared I2C bus) |

#### IRLB8721 Pinout (label facing you, legs down)

```
  Pin 1       Pin 2       Pin 3
  GATE        DRAIN       SOURCE
  (left)      (middle)    (right)
  ↑           ↑           ↑
  Nano D9     LED return  GND
  + 10kΩ to GND
```

#### Voltage Divider (Battery Monitor)

```
12V Battery + ---[ 10kΩ ]---+---[ 4.7kΩ ]--- GND
                            |
                         to Nano A0
```

Max voltage at A0 with 14.4V charging: 4.6V (safe for Nano).

#### Solar Panels

Two 20W 12V panels wired in **parallel** (positive to positive, negative to negative) into the SOLPERK 8A charge controller. Controller charges the DieHard Marine 12V battery.

For long cable runs (100ft roof to controller), use **10AWG** solar extension cable to minimize voltage drop.

#### Power Flow

```
Solar Panels (parallel) → SOLPERK 8A Controller → 12V Marine Battery
                                                       │
                                    ┌──────────────────┤
                                    │                  │
                              D-Planet Buck       DROK Buck
                              (12V → 5V)         (to LED voltage)
                                    │                  │
                              Arduino Nano          10 LEDs
                              DS3231 RTC               │
                              OLED Display        IRLB8721 Drain
                                    │                  │
                                  D9 → Gate      Source → GND
```

### OLED Display

Shows three lines:
- **Time** (24-hour format, large text) with AM/PM
- **LED status** (ON/OFF, large text)
- **Battery voltage** (e.g., "Battery: 12.4V")

Updates every 30 seconds. Uses dynamic Vcc measurement for accurate readings regardless of power source.

### Sketches

#### Clock Setting Sketch (flash once to set DS3231 time)

Located at: `C:\Users\ericm\Desktop\set_clock\set_clock.ino`

Update the time values in the sketch before uploading. The DS3231 coin cell retains the time between uploads.

```cpp
#include <Wire.h>

#define DS3231_ADDR 0x68

byte decToBcd(byte val) { return ((val / 10) << 4) + (val % 10); }

void setup() {
  Wire.begin();
  Wire.beginTransmission(DS3231_ADDR);
  Wire.write(0x00);
  Wire.write(decToBcd(0));   // seconds
  Wire.write(decToBcd(39));  // minutes - UPDATE THIS
  Wire.write(decToBcd(13));  // hours (24hr) - UPDATE THIS
  Wire.write(decToBcd(7));   // day of week (1=Sun)
  Wire.write(decToBcd(30));  // date - UPDATE THIS
  Wire.write(decToBcd(5));   // month - UPDATE THIS
  Wire.write(decToBcd(26));  // year - UPDATE THIS
  Wire.endTransmission();
}

void loop() {}
```

#### Main Timer Sketch

Located at: `C:\Users\ericm\Desktop\led_timer\led_timer.ino`

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/wdt.h>

#define DS3231_ADDR 0x68
#define LED_PIN 9
#define BATT_PIN A0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const float DIVIDER_RATIO = 14.7 / 4.7;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

byte bcdToDec(byte val) { return ((val >> 4) * 10) + (val & 0x0F); }

long readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  long result = ADCL;
  result |= ADCH << 8;
  return 1125300L / result;
}

bool ledsOn = false;

void setup() {
  wdt_enable(WDTO_8S);
  Wire.begin();
  Wire.setWireTimeout(3000, true);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  wdt_reset();
}

void loop() {
  wdt_reset();

  Wire.beginTransmission(DS3231_ADDR);
  Wire.write(0x01);
  byte err = Wire.endTransmission();

  byte minutes = 0;
  byte hours = 0;
  bool rtcOk = false;

  if (err == 0) {
    byte count = Wire.requestFrom(DS3231_ADDR, 2);
    if (count == 2) {
      minutes = bcdToDec(Wire.read());
      hours = bcdToDec(Wire.read() & 0x3F);
      if (hours <= 23 && minutes <= 59) {
        rtcOk = true;
      }
    }
  }

  // Fail-safe: RTC error means LEDs OFF
  if (!rtcOk) {
    digitalWrite(LED_PIN, LOW);
    ledsOn = false;
    delay(5000);
    return;
  }

  // Schedule: 5:00-6:59 ON, 7:00-13:59 OFF, 14:00-23:58 ON, 23:59-4:59 OFF
  if ((hours >= 5 && hours < 7) || (hours >= 14 && (hours < 23 || (hours == 23 && minutes <= 58)))) {
    digitalWrite(LED_PIN, HIGH);
    ledsOn = true;
  } else {
    digitalWrite(LED_PIN, LOW);
    ledsOn = false;
  }

  float vcc = readVcc() / 1000.0;
  float vbat = analogRead(BATT_PIN) * (vcc / 1023.0) * DIVIDER_RATIO;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 0);
  if (hours < 10) display.print("0");
  display.print(hours);
  display.print(":");
  if (minutes < 10) display.print("0");
  display.print(minutes);

  display.setTextSize(1);
  display.setCursor(95, 5);
  display.print(hours < 12 ? "AM" : "PM");

  display.setTextSize(2);
  display.setCursor(10, 25);
  display.print("LEDs:");
  display.print(ledsOn ? "ON" : "OFF");

  display.setTextSize(1);
  display.setCursor(10, 50);
  display.print("Battery: ");
  display.print(vbat, 1);
  display.print("V");

  display.display();
  wdt_reset();
  delay(30000);
}
```

### Flashing the Nano

```powershell
# Detect port
& "$env:USERPROFILE\.local\bin\arduino-cli.exe" board list

# Compile and flash (verify COM port before running)
& "$env:USERPROFILE\.local\bin\arduino-cli.exe" compile --fqbn arduino:avr:nano:cpu=atmega328 "$env:USERPROFILE\Desktop\led_timer"
& "$env:USERPROFILE\.local\bin\arduino-cli.exe" upload --fqbn arduino:avr:nano:cpu=atmega328 --port COM14 "$env:USERPROFILE\Desktop\led_timer"
```

### Required Libraries

- Adafruit SSD1306
- Adafruit GFX Library
- Adafruit BusIO (auto-installed with SSD1306)

Install via: `arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library"`

### Diagrams

- Wiring diagram: `C:\Users\ericm\Desktop\chicken_led_timer_wiring.html`
- Breadboard layout: `C:\Users\ericm\Desktop\breadboard_layout.html`

### Parts to Order

| Part | Purpose | Link |
|------|---------|------|
| Second 20W 12V solar panel | More charging capacity | [Newpowa 20W 12V](https://www.amazon.com/Newpowa-High-Efficiency-Monocrystalline-Designed-Battery/dp/B00W813E4I) |
| 10AWG solar extension cable 100ft | Roof to controller run | [10AWG 100ft MC4](https://www.amazon.com/Extension-Universal-Compatible-Weatherproof-UV-Resistant/dp/B0DDS13P6N) |

### Build History

| Date | Event |
|------|-------|
| 2026-05-30 | Built and deployed Nano + DS3231 + IRLB8721 + OLED timer. All LEDs on and working. |
| 2026-05-31 | LEDs stuck ON overnight (likely I2C bus lockup froze Nano mid-ON). Battery drained. Added watchdog timer (8s auto-reset), I2C bus timeout, RTC read validation with fail-safe LEDs OFF. Re-flashed. |

---

## Enclosure (3D Printed)

### Design Summary

| Property | Value |
|----------|-------|
| Outer dimensions | 180 x 140 x 80mm (base) + 4mm lid |
| Interior usable | ~174 x 134 x 76mm |
| Material | PETG |
| Print bed target | 200 x 200mm (fits with ~10mm margin) |
| Closure | Hinge + magnetic closure (no tools, no flexing clips) |
| Wall mount | 4 corner holes through floor (M4 screws) |

### Features

#### Base (box)
- Open-top rectangular shell, 3mm walls, 4mm floor
- 4x M4 corner mounting holes through floor (reinforced bosses inside) -- screws into barn wall
- 60 x 15mm cable entry slot on front wall
- 16 narrow vent slits per side (1.5mm x 40mm) -- airflow while rejecting dust
- 12-position zip-tie anchor grid (4x3) on inside floor for strapping Pi, Wanderer, SSRs, bucks anywhere
- **3 hinge lugs** on the back top edge (interleave with lid's 2 lugs)
- **2 magnet shelves** on inside of front wall at the top, each with an 8mm magnet pocket

#### Lid
- 180 x 140 x 4mm flat plate matching box footprint
- 14 narrow vent slits (1.5mm x 50mm) cut through the top for heat exhaust
- **2 hinge lugs** on back edge (interleave with base)
- **2 magnet pockets** on underside at front
- No screws, no tools, no fragile snap clips

#### Closure mechanism
- **Hinge:** 3mm steel rod (or M3 x 180mm screw) inserted through all 5 interleaved lugs once during assembly. Lid swings up from the top.
- **Magnets:** 4x 8mm diameter x 3mm thick neodymium disc magnets, glued with CA. Two in box shelves, two in lid underside. When closed, gravity drops the lid down, magnets hold it closed with ~3kg pull force.
- To open: lift the front edge of the lid. Magnets release cleanly.

### Parts & Hardware

| Item | Qty | Notes |
|------|-----|-------|
| PETG filament | ~230g | Base ~180g, lid ~50g |
| 3mm steel rod x 180mm (or M3x180 screw) | 1 | Hinge pin |
| 8mm dia x 3mm thick neodymium disc magnet | 4 | Amazon pack of 20 is ~$8 |
| M4 x 16mm flat-head screws + wall anchors | 4 | Wall mount |
| CA (super glue) | as needed | For magnets |

### Print Settings (PETG)

- Layer height: 0.2mm
- Walls: 4 (weather resistance)
- Infill: 25% gyroid
- Nozzle: 240C
- Bed: 70C
- **Base orientation:** open side up (no supports needed for main body)
- **Lid orientation:** top face up. Tree supports required under the hinge lugs and magnet pocket overhangs.
- Estimated time: ~12-14 hrs base, ~3 hrs lid

### Files

All paths are on Eric's Windows PC:

| File | Location |
|------|----------|
| OpenSCAD source (parametric) | `C:\Users\ericm\Documents\homestead_enclosure.scad` |
| Base STL | `C:\Users\ericm\Downloads\homestead\homestead_base.stl` |
| Lid STL | `C:\Users\ericm\Downloads\homestead\homestead_lid.stl` |
| Skill copies | `C:\Users\ericm\.claude\skills\chicken-lights-garden-water-box\` |

**Rendering STLs from SCAD:**
```bash
"/c/Program Files (x86)/OpenSCAD/openscad.exe" -D 'part="base"' \
  -o homestead_base.stl homestead_enclosure.scad
"/c/Program Files (x86)/OpenSCAD/openscad.exe" -D 'part="lid"' \
  -o homestead_lid.stl homestead_enclosure.scad
"/c/Program Files (x86)/OpenSCAD/openscad.exe" -D 'part="assembly"' \
  -o homestead_assembly.stl homestead_enclosure.scad
```

### Assembly Order

1. Print base (open side up, no supports)
2. Print lid (top face up, tree supports under hinge lugs)
3. Clean up hinge-lug supports carefully
4. Interleave lid lugs into base lugs, insert hinge pin
5. Glue 4 magnets -- ensure poles are oriented to ATTRACT (drop one into box pocket, test with another before gluing lid pair)
6. Mount box to barn wall with 4x M4 screws through floor holes
7. Install Nano + buck converters + MOSFET + RTC + OLED inside, secured with zip ties through the anchor grid
8. Route cables out through the bottom slot

### Design History

- v1: 200x150x90mm with external flanges and 4x M3 corner screws -- **oversized** for 200mm bed
- v2a: 180x140x80mm, internal mount bosses, corner lid screws -- required screwdriver every time
- v2b: Added knurled thumbscrew knobs -- **rejected** (longer screws needed, plastic would strip)
- v2c: Hinge on top edge + flex snap latch on front -- **rejected** (cantilever clip fatigue over time)
- **v2d (current):** Hinge + magnetic closure -- no flexing, no fatigue, one-hand open/close
