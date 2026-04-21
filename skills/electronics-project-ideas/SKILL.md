# Electronics Project Ideas

150 project ideas built entirely from Eric's component inventory. Printable document at `~/Documents/150-Project-Ideas.odt`.

## Categories

1. **Homestead & Farm (1-20)** — Coop automation, irrigation, solar monitoring, RFID chicken tracking, barn alerts, 3D printed mechanisms
2. **Home Network & Monitoring (21-35)** — Pi-hole display, uptime monitor, WiFi mapper, WireGuard status, kids activity, touch-activated reboot
3. **Music & Audio (36-55)** — MIDI controllers, VU meter, tuner, spectrum analyzer, Ardour transport, solenoid drum machine, touch MIDI pads, 555 tone generator, touch theremin
4. **Test Equipment & Tools (56-75)** — Oscilloscope, frequency counter, component tester, power supply, servo/motor tester, 555 frequency tester, ESR meter, touch calibrator
5. **Input Devices & Controllers (76-95)** — Macro keypad, password vault, touch surface controller, Bluetooth media, RFID jukebox, 555 touch switch, touch lamp controller
6. **Robotics & Motorized Builds (96-115)** — RC car, line follower, touch-controlled robot, robotic arm, drawing plotter, servo lock box, camera slider, coin sorter, gear train demo
7. **Fun & Games (116-135)** — Pong, Snake, Tetris, reaction game, solenoid whack-a-mole, RFID treasure hunt, touch Bop-It, 555 LED chaser, servo puppet theater, touch music box
8. **Clocks, Timers & Desk Gadgets (136-145)** — Pomodoro, world clock, habit tracker, GitHub counter, notification bridge, RFID checklist
9. **Environmental & Science (146-150)** — Thermometer, seismograph, magnetic field mapper, light logger, everything dashboard

## Components Used Across Ideas

| Component | Used In (example projects) |
|-----------|---------------------------|
| ESP32-S3 | WiFi/BLE projects, USB HID devices, anything needing wireless |
| Arduino Nano x2 | Simpler standalone projects (games, timers, basic sensors) |
| OLED displays x5 | Every single project — the display for all builds |
| 24V battery | Portable/outdoor projects, power supply, battery tester |
| Motor drivers + 6VDC motors | Robots, conveyor, turntable, RC car, camera slider, blinds |
| Servo motors | Coop door, robotic arm, pan-tilt, valve control, lock box, puppet theater |
| Solenoids | Drum machine, launcher, whack-a-mole, doorbell, scarecrow, stamper |
| Bluetooth module | RC control, media remote, phone notifications, page turner, trivia buzzers |
| RFID module + tags | Chicken tracker, tool checkout, treasure hunt, jukebox, lock box, checklist |
| Light sensors | Line follower, daylight tracker, barn door, auto-dimmer, coin sorter |
| Touch capacitor modules | Touch MIDI pads, theremin, touch lamp, Bop-It, habit tracker, wall panels |
| Cherry MX hall effect switches | Velocity-sensitive MIDI, analog gaming, pressure input, seismograph |
| 555 timer ICs | Standalone oscillators, metronome, tone generator, LED chaser, touch switch, ESR meter |
| 3D printer | Enclosures, robot chassis, mechanical parts, game cases, mounts, gears |
| Resistors/caps/pots/MOSFETs/transistors | Supporting components for nearly every project |

## Document Details

- **File:** `~/Documents/150-Project-Ideas.odt` (and `.html` source)
- **Generated:** 2026-04-20
- **Format:** Tables with #, Project Name, Description, Key Components per entry
- **Section labels:** Bold standalone paragraphs between tables
- **Print-ready:** Formatted for Brother HL-L8360CDW or HL-L2380DW

## How to Regenerate

If the inventory changes or more ideas are needed, regenerate from the HTML source:
```bash
libreoffice --headless --convert-to odt --outdir ~/Documents/ ~/Documents/150-Project-Ideas.html
```
