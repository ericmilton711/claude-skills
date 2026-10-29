# CARESCAPE ONE Service Training - Verified Answers

GE Healthcare CARESCAPE ONE (CS ONE) patient monitor training reference.
Answers verified against the User Manual (ManualsLib, SW v3.0) and GE parts store.

## Quick Reference - End of Training Exam (21 Questions)

Answers are listed by topic since the quiz shuffles question order.

| Topic | Answer | Source |
|-------|--------|--------|
| IPX ingress protection rating | **C) IPX1** | Manual p.251: IP41 (water protection = level 1) |
| Display functional testing checks | **A, B** (text readable + images clear) | Low resolution/brightness would be faults |
| Parameters needing minor assembly | **A, C** (ECG + Pressure) | NIBP is built into CS ONE, not a separate parameter |
| Power frequency setup steps | **B, C, D** (Config > Power Frequency, select freq, Close) | Not "Configuration > Frequency" (A is wrong path) |
| Power LED during battery swap | **A, B** (amber/yellow then green) | Manual p.42: yellow = booting, green = ready |
| Suitable for public mains | **B) CISPR 11** | Manual p.389: CISPR 11, Group 1, Class A |
| NIBP calibration tools | **A, C, D** (manometer, rigid cylinder, NIBP cuff) | Defib sync cable (B) is unrelated |
| Temperature measurement range | **B) 0 and 45°C** | Manual p.320: 0°C to 45°C (32°F to 113°F) |
| Green+yellow LEDs blinking 1-5 sec | **A) Less than 20 minutes** | Manual p.42 battery LED table |
| ESD precautions | **C, D** (handle PCBs by edges, don't flex boards) | A and B describe bad practices |
| Environmental requirements | **B, C, D** (temperature, humidity, atm. pressure) | Manual p.251: 0-40°C, 5-95% RH, 1075-616 hPa |
| Dock F0 product code | **D) SNB** | SNA = CS ONE monitor, SNB = Dock F0 |
| Periodic safety/performance check | **B) Planned Maintenance Check** | Manual p.381: PM every 24 months |
| Log files encrypted with default pw | **True** | Diagnostics tab log download |
| Electrical safety tests | **A, B** (verify outlet wiring + cord undamaged) | Never use "any available cord" (C), not DC (D) |
| Backup steps | **B, D** (Config > Backup > Download, save to storage) | Not Restore (A), not cloud (C) |
| Password setup rules | **A, C** (A-Z/a-z/0-9/space, 8-16 chars) | B850 service manual confirms same platform rules |
| Dock F0 FRUs | **A, B, C** (Interface PCB, Flex+Cover Rail, Internal Power Cable) | GE parts: 2094695-001/-002/-004. No pneumatic components (D) |
| Calibrations NOT performed | **A, C** (Invasive Pressure, ECG) | Service Calibrations ARE: Analog output, NIBP, Microstream CO2. Temp checked yearly |
| Service Calibrations tab | **C) Defaults and Service** | Confirmed: Defaults & Service tab holds service calibrations |
| Analog output voltage error | **C) Analog output malfunction 1103** | Training material listed "Analog output malfunction" (no "voltage") |

## Key Facts from Manual

### Battery LED Indicators (Manual p.42)
| LED Pattern | Meaning |
|-------------|---------|
| Solid yellow | Charging |
| Slow-blink yellow (1-5 sec) | Battery not present or can't communicate |
| Fast-blink yellow | Charge capacity < 60% of design |
| Solid green | Running on battery |
| Slow-blink green+yellow (1-5 sec) | **≤ 20 minutes remaining** |
| Fast-blink green+yellow | **≤ 5 minutes remaining** |

### On/Standby Key LED
| Color | Meaning |
|-------|---------|
| Yellow | External power connected + off, OR booting/standby |
| Green | Ready for operation |
| Unlit | No power |

### Ingress Protection Ratings
- CS ONE monitor: **IP41**
- Dock F0: **IP42**
- CARESCAPE Parameters (ECG, Pressure, Temp, etc.): **IP47**

### Environmental Specs
- Operating temp: 0°C to 40°C
- Operating humidity: 5-95% RH non-condensing
- Operating altitude: -500m to 4000m (1075-616 hPa)

### Temperature Parameter
- Measurement range: 0°C to 45°C
- Accuracy: ±0.1°C (18-45°C), ±0.2°C (0-18°C)
- 2 channels (T1, T2)

### Service Calibrations (performed on CS ONE)
1. Analog output calibration
2. NIBP calibration
3. Microstream CO2 calibration

### Service Applications
| Interface | Features | Access |
|-----------|----------|--------|
| Carescape First Use Wizard | Initial password, manual setup, settings activation | Service PC |
| Carescape Service Interface | Configure platform, transfer settings, install SW/licenses, maintenance | Service PC |
| Service Calibrations | Analog output, NIBP, Microstream CO2 | Local |

### Dock F0 FRUs (GE part numbers)
- 2094695-001: Dock Interface PCB
- 2094695-002: Cover Rail Assembly w/Flex
- 2094695-004: DC Power Supply

### EMC
- CISPR 11, Group 1, Class A
- Suitable for all establishments OTHER than domestic
- NOT for sites directly connected to public low-voltage network supplying homes

### Planned Maintenance
- Every 24 months: visual inspection, electrical safety tests, functional check, battery maintenance
- Yearly: check NIBP and temperature calibration

### Knowledge Check Answers (from training modules)
- **Monitor Setup Menu** allows access to screen setup, colors, sound volumes, parameter setup, battery status, brightness, admit/discharge, case setup, standby settings
- **Software Packages** pre-configure the monitor's behavior and appearance for a specific clinical environment
- **Profiles** are a group of unique settings suited to a particular care unit or patient demographic

## Score History
- Attempt 1: 59% (FAIL)
- Attempt 2: 80.95% (PASS) - 4 unknown wrong answers remain
