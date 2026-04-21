# Electronics Project Ideas

125 project ideas built entirely from Eric's component inventory. Printable document at `~/Documents/125-Project-Ideas.odt`.

## Categories

1. **Homestead & Farm (1-20)** — Coop automation, irrigation, solar monitoring, RFID chicken tracking, barn alerts
2. **Home Network & Monitoring (21-35)** — Pi-hole display, uptime monitor, WiFi mapper, WireGuard status, kids activity
3. **Music & Audio (36-50)** — MIDI controllers, VU meter, tuner, spectrum analyzer, Ardour transport, solenoid drum machine
4. **Test Equipment & Tools (51-65)** — Oscilloscope, frequency counter, component tester, power supply, servo/motor tester
5. **Input Devices & Controllers (66-80)** — Macro keypad, password vault, Bluetooth media control, RFID jukebox, pan-tilt mount
6. **Robotics & Motorized Builds (81-95)** — RC car, line follower, robotic arm, drawing plotter, servo lock box, camera slider
7. **Fun & Games (96-110)** — Pong, Snake, Tetris, reaction game, solenoid whack-a-mole, RFID treasure hunt, servo maze
8. **Clocks, Timers & Desk Gadgets (111-120)** — Pomodoro, world clock, habit tracker, GitHub counter, notification bridge
9. **Environmental & Science (121-125)** — Thermometer, seismograph, magnetic field mapper, light logger, everything dashboard

## Components Used Across Ideas

| Component | Used In (example projects) |
|-----------|---------------------------|
| ESP32-S3 | WiFi/BLE projects, USB HID devices, anything needing wireless |
| Arduino Nano x2 | Simpler standalone projects (games, timers, basic sensors) |
| OLED displays x5 | Every single project — the display for all builds |
| 24V battery | Portable/outdoor projects, power supply, battery tester |
| Motor drivers + 6VDC motors | Robots, conveyor, turntable, RC car, camera slider |
| Servo motors | Coop door, robotic arm, pan-tilt, valve control, lock box |
| Solenoids | Drum machine, launcher, whack-a-mole, doorbell, scarecrow |
| Bluetooth module | RC control, media remote, phone notifications, page turner |
| RFID module + tags | Chicken tracker, tool checkout, treasure hunt, jukebox, lock box |
| Light sensors | Line follower, daylight tracker, barn door, auto-dimmer |
| Cherry MX hall effect switches | Velocity-sensitive MIDI, analog gaming, pressure input, all button-based projects |
| Resistors/caps/pots/MOSFETs/transistors | Supporting components for nearly every project |

## Document Details

- **File:** `~/Documents/125-Project-Ideas.odt` (and `.html` source)
- **Generated:** 2026-04-20
- **Format:** Tables with #, Project Name, Description, Key Components per entry
- **Print-ready:** Formatted for Brother HL-L8360CDW or HL-L2380DW

## How to Regenerate

If the inventory changes or more ideas are needed, regenerate from the HTML source using:
```bash
libreoffice --headless --convert-to odt --outdir ~/Documents/ ~/Documents/125-Project-Ideas.html
```
