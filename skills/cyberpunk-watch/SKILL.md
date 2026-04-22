# Cyberpunk Watch — NØDE_01

DIY wrist watch built from spare OLED displays and ESP32-S3, with a 3D printed PETG case.

---

## Overview

- **Codename:** NØDE_01
- **Style:** Cyberpunk — neon cyan/magenta, angled case, exposed PCB window, circuit traces, HUD-style display
- **Display:** 0.96" SSD1306 128×64 I2C OLED (blue/yellow zones)
- **MCU:** ESP32-S3 (WiFi NTP time sync, deep sleep)
- **Sensor:** MPU6050 6-axis accelerometer/gyroscope (step counter, raise-to-wake)
- **Battery:** 3.7V 200mAh LiPo (402030 form factor, 4×20×31mm)
- **Charger:** TP4056 USB-C module with dual protection
- **Case:** 3D printed PETG, ~50×42×20mm
- **Strap:** 22mm NATO nylon, black

## Estimated Dimensions

| Measurement | Value |
|-------------|-------|
| Width | 50mm |
| Height | 42mm |
| Thickness | ~20mm |
| Display | 0.96" |

## Features

| Feature | Status |
|---------|--------|
| WiFi NTP time sync | Core |
| Deep sleep power mgmt | Core |
| USB-C charging | Core |
| Raise-to-wake (MPU6050) | Core |
| Step counter | Core |
| Tilt-to-scroll screens | Core |
| Status LEDs (green/red) | Core |
| Bluetooth notifications | Stretch |
| Sleep tracking | Stretch |

## Bill of Materials

### Already Have
| Item | Spec | Source |
|------|------|--------|
| SSD1306 OLED | 0.96" 128×64 I2C, blue/yellow | Electronics inventory (×5 spare) |
| ESP32-S3 | Dev board | Electronics inventory (×1 spare) |
| 3D Printer | PETG capable | Available |
| Resistors / passives | Various | Electronics inventory |
| Bluetooth module | | Electronics inventory |

### Need to Buy (~$27 total)
| Item | Spec | ~Price | Link |
|------|------|--------|------|
| LiPo battery | 3.7V 200mAh 402030 | ~$8 | [Bihuade 402030 on Amazon](https://www.amazon.com/Bihuade-Battery-Rechargeable-402030-Lithium/dp/B0GDXP6DFP) |
| TP4056 charger | USB-C, 3-pack, dual protection | ~$7 | [HiLetgo TP4056 3-pack](https://www.amazon.com/HiLetgo-Lithium-Charging-Protection-Functions/dp/B07PKND8KG) |
| MPU6050 | GY-521 6-axis IMU, I2C | ~$4 | [HiLetgo MPU-6050](https://www.amazon.com/HiLetgo-MPU-6050-Accelerometer-Gyroscope-Converter/dp/B078SS8NQV) |
| NATO strap | 22mm black nylon | ~$8 | [BARTON 22mm black](https://www.amazon.com/22mm-Black-BARTON-Stainless-Buckle/dp/B07CZR9MQL) |

## Wiring (I2C bus shared)

```
ESP32-S3          SSD1306 OLED       MPU6050
─────────         ────────────       ───────
3.3V  ──────────► VCC ◄──────────── VCC
GND   ──────────► GND ◄──────────── GND
GPIO 21 (SDA) ──► SDA ◄──────────── SDA
GPIO 22 (SCL) ──► SCL ◄──────────── SCL
                                     INT ──► GPIO 14 (wake interrupt)

TP4056 ──► LiPo battery ──► ESP32-S3 (3.3V via onboard regulator)
```

## Power Budget (200mAh battery)

| State | Current | Time/Day | mAh/Day |
|-------|---------|----------|---------|
| Deep sleep | 0.05mA | 23.5 hrs | ~1.2 |
| Screen on (raise-to-wake) | 60mA | 30 min | 30 |
| WiFi NTP sync | 200mA | 10 sec | 0.6 |
| **Total** | | | **~32** |

**Estimated battery life: 4-6 days per charge**

## How the MPU6050 Works (MEMS)

The MPU6050 has no visible moving parts — it uses MEMS (Micro-Electro-Mechanical Systems):

- **Accelerometer:** Microscopic silicon proof mass on spring-like flexures. Movement shifts the mass, changing capacitance between fixed plates. Three structures at right angles = X, Y, Z acceleration.
- **Gyroscope:** Tiny vibrating mass driven by electrostatic forces. Rotation causes Coriolis effect, pushing the mass perpendicular to vibration. Capacitance change = rotation rate. Three structures = rotation around X, Y, Z.
- **6 DOF:** 3 acceleration axes + 3 rotation axes = 6 degrees of freedom.

## Design Mockup

The HTML mockup is at `~/watch-mockup.html` — open in browser to see:
- Front view with live clock, circuit traces, PCB window, neon edge glow, status LEDs
- Side profile showing OLED / ESP32-S3 / LiPo stack
- Animated elements: glitch title, blinking colon, screen sweep, pulsing edge, breathing LEDs

## Next Steps

1. Order parts (battery, TP4056, MPU6050, strap)
2. Write ESP32 firmware (clock face, NTP sync, deep sleep, raise-to-wake)
3. Design 3D printable case with spring bar lug channels
4. Wire and test on breadboard
5. Assemble final build

---

*Created: 2026-04-21*
