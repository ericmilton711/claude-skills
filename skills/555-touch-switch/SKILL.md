# 555 Touch Switch

Touch-activated power switch with auto-off timer and OLED display. Touch the pad, load turns ON, OLED counts down, auto-off when timer expires. Touch again while ON for immediate OFF.

## Project Origin
From the 150 Project Ideas list (#90): "555 + touch cap module creates a standalone toggle switch. Drives a MOSFET to switch any 24V load. Add Nano + OLED for status display."

## How It Works
1. Touch the TTP223 pad → Nano detects touch on D2
2. Nano pulses D3 LOW → triggers 555 monostable
3. 555 output goes HIGH → MOSFET gate driven → load turns ON
4. 555 timer counts down (set by R-C values)
5. OLED shows "ON" with countdown
6. Timer expires → 555 output LOW → MOSFET OFF → load OFF automatically
7. Touch while ON → Nano pulls D5 LOW → 555 resets → immediate OFF

## Components (all from existing inventory unless noted)
| Component | Role |
|---|---|
| Arduino Nano | Brain — touch logic, 555 control, OLED display |
| 555 Timer | Monostable auto-off timer |
| TTP223 Touch Cap Module | Capacitive touch input |
| IRLZ44N MOSFET | Logic-level N-channel, switches the load |
| OLED 128×64 (SSD1306) | I2C status display |
| 8× AA Battery Pack | 12VDC power (1.5V × 8 series) |
| 220Ω resistor | 555 output → MOSFET gate current limiter |
| 10kΩ resistor | MOSFET gate pulldown to GND |
| 10kΩ resistor | 555 Pin 2 (Trigger) pullup to 5V |
| 10nF capacitor | 555 Pin 5 (Control Voltage) → GND noise filter |
| R_timing + C_timing | Sets auto-off duration (see timer table) |
| 2× 2-pos screw terminals | PWR IN (left wall) + LOAD OUT (right wall) |

## Wiring Diagram
Full SVG wiring diagram: `555-touch-switch-wiring.svg` (in this skill directory)
Also saved at: `~/555-touch-switch-wiring.svg`

## Nano Pin Map
| Pin | Connection | Direction |
|---|---|---|
| Vin | 12V battery (via PWR IN screw terminal) | Power in |
| 5V | 5V bus → TTP223, OLED, 555 | Power out |
| GND | GND bus | Ground |
| D2 | TTP223 SIG | Input (touch detect) |
| D3 | 555 Pin 2 (Trigger) — pulses LOW to fire | Output |
| D4 | 555 Pin 3 (Output) — reads timer state | Input |
| D5 | 555 Pin 4 (Reset) — LOW to force off | Output |
| A4 | OLED SDA | I2C data |
| A5 | OLED SCL | I2C clock |

## 555 Pin Map
| Pin | Name | Connection |
|---|---|---|
| 1 | GND | GND bus |
| 2 | Trigger | Nano D3 + 10kΩ pullup to 5V |
| 3 | Output | 220Ω → MOSFET Gate + Nano D4 |
| 4 | Reset | Nano D5 |
| 5 | Control Voltage | 10nF → GND |
| 6 | Threshold | Node T (R-C junction) |
| 7 | Discharge | Node T (R-C junction) |
| 8 | VCC | 5V bus |

## Timer R-C Network
```
5V → R_timing → [Node T] → C_timing → GND
                    ↑
              Pin 6 + Pin 7
```

**T = 1.1 × R × C**

| Duration | R | C |
|---|---|---|
| ~10 sec | 91kΩ | 100µF |
| ~1 min | 560kΩ | 100µF |
| ~5 min | 2.7MΩ | 100µF |

May need to order: MΩ-range resistor for longer timers, 100µF electrolytic cap.

## MOSFET Wiring
| Pin | Connection |
|---|---|
| Gate | 555 Pin 3 (via 220Ω) + 10kΩ pulldown to GND |
| Drain | LOAD OUT screw terminal (−) |
| Source | GND bus |

## Power Architecture
```
12V Battery → [PWR IN screw terminal] → +12V bus → Nano Vin
                                        +12V bus → LOAD OUT (+) screw terminal
Nano 5V out → 5V bus → TTP223, OLED, 555
MOSFET Drain → LOAD OUT (−) screw terminal
```

## Physical Connectors
- **PWR IN** — 2-position screw terminal, mounted on left enclosure wall. Battery pack wires clamp in.
- **LOAD OUT** — 2-position screw terminal, mounted on right enclosure wall. Strip load wire, clamp in.
- Load can be any 12V DC device: LED strip, fan, lamp, solenoid, etc.

## 3D Printed Enclosure (TODO)
- Deck-of-cards size box
- OLED window on top face
- TTP223 touch pad flush-mounted on top face
- Screw terminal cutouts on left (PWR IN) and right (LOAD OUT) walls
- Screw mounting holes for Nano and components

## Status
- **2026-04-21**: Circuit designed, wiring diagram created. Ready to breadboard.
