# MIDI Piano Project with 3D Printed Components

**Project Start:** December 2025  
**Last Updated:** January 6, 2026  
**Status:** Active Development

## Project Overview

Building a functional MIDI piano controller with 3D-printed keys and frame components, using Cherry MX RGB mechanical switches for key sensing. This project combines Arduino MIDI programming with parametric 3D design in OpenSCAD.

## Project Components

### 3D Printed Parts

#### Piano Keys
- **Black Keys:**
  - black_keys_batch1.stl - First set of black keys
  - black_keys_batch2.stl - Second set of black keys
  
- **Frame Components:**
  - frame_left.stl (29.8 KB)
  - frame_middle.stl (30.5 KB)
  - frame_right.stl (64.5 KB)

#### OpenSCAD Source
- **Main Design File:** midi_piano_realistic.scad (15.3 KB)
  - Parametric design for easy adjustments
  - Generates both keys and frame components
  - Designed with Cherry MX switch mounting holes

### Electronics & Hardware

#### Key Switches: Cherry MX RGB Switches
- **Type:** Mechanical keyboard switches (NOT Hall Effect)
- **Features:**
  - Tactile feedback
  - RGB backlighting capability
  - Standard Cherry MX mounting (14mm x 14mm)
  - Hot-swappable compatible
  - Consistent actuation force
- **Advantages over Hall Effect:**
  - More affordable and readily available
  - Better tactile feedback for piano playing
  - RGB lighting for visual feedback
  - Standard mounting and keycaps

#### Arduino MIDI Controller
- midi_controller_piano.ino - Configured for Cherry MX switch matrix scanning

## File Locations

Mac (192.168.1.7):~/
- midi_piano_realistic.scad - OpenSCAD design file
- black_keys_batch1.stl, black_keys_batch2.stl - Black keys
- frame_left.stl, frame_middle.stl, frame_right.stl - Frame sections
- midi_controller_piano.ino - Arduino MIDI code
- Documents/midi_controller.ino - Arduino code backup
- Documents/MIDI.odt - Project documentation

## Development Timeline

### December 30, 2025
- Created OpenSCAD design with Cherry MX RGB switch mounts
- Generated STL files for keys and frame
- Completed Arduino MIDI controller code with switch matrix scanning

### January 6, 2026
- Updated documentation to specify Cherry MX RGB switches
- Ongoing: Printing components and testing assembly

## Technical Details

### 3D Printing Specifications
- Material: PLA (rigidity) or PETG (durability)
- Layer Height: 0.2mm
- Infill: 20% (keys), 15% (frame)
- Walls: 3 perimeters
- **Cherry MX mount holes:** Must be precise 14mm x 14mm (±0.05mm tolerance)

### Cherry MX Switch Integration
- Mount hole size: 14mm x 14mm
- Switch spacing: 19.05mm (standard keyboard)
- Actuation: Digital on/off (no analog sensing)
- Wiring: Matrix scanning with diodes for ghosting prevention
- RGB: WS2812B compatible LEDs

### MIDI Controller Hardware
- Arduino Mega (recommended for I/O pins)
- Cherry MX RGB switches (61-88 keys)
- 1N4148 diodes (one per key)
- MIDI output (5-pin DIN or USB)
- RGB LED controller

**Velocity Sensing:** Cherry MX switches are digital, so velocity requires:
1. Timing-based detection (measure key press speed)
2. Dual-switch setup (two switches per key at different heights)
3. Fixed velocity output (simpler implementation)

## Assembly Checklist

### Components Needed
- [ ] Cherry MX RGB switches (61-88 keys)
- [ ] 1N4148 diodes (one per key)
- [ ] Arduino Mega
- [ ] MIDI connector (5-pin DIN or USB)
- [ ] Wire (28 AWG)
- [ ] RGB LED controller
- [ ] 3D printed keys and frame sections

### Assembly Steps
- [ ] Verify Cherry MX mount holes in OpenSCAD design
- [ ] Print test piece with single switch mount
- [ ] Test switch fit and retention
- [ ] Complete printing all components
- [ ] Assemble frame sections
- [ ] Install switches and wire matrix
- [ ] Test MIDI functionality per key
- [ ] Implement velocity sensing algorithm
- [ ] Configure RGB lighting patterns

## Why Cherry MX RGB Instead of Hall Effect?

**Advantages:**
- Lower cost and more readily available
- Better tactile feedback (important for piano feel)
- RGB backlighting built-in
- Hot-swap compatible
- Standard keycaps available

**Trade-offs:**
- No analog sensing (requires timing for velocity)
- Velocity sensing more complex

**Velocity Strategy:** Use timing-based detection:
- Measure time from key press start to actuation
- Map timing to MIDI velocity values (0-127)
- Requires fast scanning rate (1000+ Hz)

## Resources

### Related Skills
- 3D Printing: ~/.claude/skills/3d-printing/SKILL.md
- Project Management: ~/.claude/skills/project-management/SKILL.md

### External Links
- OpenSCAD: https://openscad.org/documentation.html
- Arduino MIDI: https://github.com/FortySevenEffects/arduino_midi_library
- Cherry MX: https://www.cherrymx.de/

## Session Log - January 6, 2026
- Created unified MIDI piano + 3D printing project documentation
- **Specified Cherry MX RGB switches (not Hall Effect)**
- Documented switch integration and wiring requirements
- Added velocity sensing implementation strategy

---

**Quick Access:**
- SSH: ssh mac@192.168.1.7
- View design: cat ~/midi_piano_realistic.scad
- View code: cat ~/midi_controller_piano.ino
- List files: ls -lh ~/*.stl ~/*.scad ~/*.ino

**Status:** In Progress - Design verification and printing phase  
**Hardware:** Cherry MX RGB switches with Arduino MIDI controller

---

## Hardware Modification - January 6, 2026 (Cherry MX Integration)

### Switch Type Change: Hall Effect → Cherry MX RGB

**Decision:** Transitioned from Hall Effect sensors to Cherry MX RGB switches while keeping existing 3D printed keys.

**Implementation:** Hybrid design using new cherry_mx_switch_holder component

### How It Works

**Key Components:**
1. **Existing keys** - No changes needed (white_keys_batch1/2/3.stl, black_keys_batch1/2.stl)
2. **Existing frame** - No changes needed (frame_left/middle/right.stl)
3. **NEW: cherry_mx_switch_holder** - 3D printed mount for Cherry MX switches (25 needed)

**Assembly:**
```
[Key with magnet] ← Already printed, no changes
      ↓
[Frame top] ← Already printed, no changes
      ↓
[Cherry MX Switch] ← Snaps into new holder
      ↓
[Switch Holder] ← NEW 3D printed part (25x)
      ↓
[Frame bottom]
```

**Mechanism:**
- Keys pivot on existing pins in frame
- Magnet in key moves down when front of key is pressed
- Magnet presses Cherry MX switch stem
- Switch sends digital signal to Arduino

### Cherry MX Switch Holder Specifications

**Part Name:** cherry_mx_switch_holder  
**OpenSCAD Module:** Added to ~/midi_piano_realistic.scad (line 484)  
**File:** ~/cherry_mx_holder.stl

**Dimensions:**
- Base: 20mm × 20mm × 1.5mm (plate)
- Support walls: 5mm tall
- Total height: ~6.5mm
- Switch hole: 14mm × 14mm (standard Cherry MX)
- Mounting: 3× M3 screw holes

**Fit Check:**
- Frame interior height: 17mm
- Switch + holder height: ~13mm
- Clearance: 4mm ✓

**Print Settings:**
- Material: PLA or PETG
- Layer height: 0.2mm
- Infill: 30-50%
- Walls: 4 perimeters
- Supports: None needed
- Quantity: 25 (15 white keys + 10 black keys)

### File Modifications

**OpenSCAD Source Modified:**
- ~/midi_piano_realistic.scad
- Added: cherry_mx_switch_holder() module
- Added: Render option for part="cherry_mx_switch_holder"

**New STL to Generate:**
- cherry_mx_holder.stl (print 25 copies)

**Existing STLs (Unchanged - No Reprinting Needed):**
- white_keys_batch1.stl
- white_keys_batch2.stl
- white_keys_batch3.stl
- black_keys_batch1.stl
- black_keys_batch2.stl
- frame_left.stl
- frame_middle.stl
- frame_right.stl

### Hardware Bill of Materials

**Per Key (25 total):**
- 1× Cherry MX RGB switch
- 1× 3D printed cherry_mx_switch_holder
- 1× 1N4148 diode
- 3× M3 × 8mm screws (to mount holder to frame floor)
- Wire for matrix

**Additional:**
- 1× Arduino Mega (or similar with enough I/O pins)
- 1× MIDI output (5-pin DIN or USB MIDI)
- Dupont wires or 28 AWG wire for matrix
- Soldering supplies

### Assembly Instructions

1. **Print holders:** Print 25× cherry_mx_switch_holder.stl
2. **Install switches:** Snap Cherry MX RGB switches into 14mm square holes
3. **Position in frame:** Place holders inside frame cavity, one under each key
4. **Align:** Position so switches align with magnet locations:
   - White keys: 25mm from key front
   - Black keys: 20mm from key front
5. **Secure:** Screw holders to frame floor using M3 screws
6. **Test fit:** Install keys with pivot pins, test that magnets press switches
7. **Wire matrix:** Connect switches in row/column matrix with diodes
8. **Connect Arduino:** Wire matrix to Arduino input pins
9. **Program:** Upload MIDI controller code

### Cost Analysis

**Cherry MX Approach (Selected):**
- 25× Cherry MX RGB switches: $25-40
- Filament for 25 holders: ~$2
- Diodes and wire: ~$10
- **Total hardware: ~$37-52**

**vs. Hall Effect (Original):**
- 25× Hall Effect sensors: $25-50
- Calibration complexity: High
- Analog signal processing required

**Savings:** Similar cost but simpler implementation

### Commands Reference

**Generate STL on Mac:**
```bash
ssh mac@192.168.1.7
cd ~/
openscad -o cherry_mx_holder.stl -D part=cherry_mx_switch_holder midi_piano_realistic.scad
```

**Transfer to Windows laptop:**
```bash
scp mac@192.168.1.7:~/cherry_mx_holder.stl ~/Downloads/
```

**Slice with Cura:**
- Open UltiMaker Cura on Windows laptop
- Import cherry_mx_holder.stl
- Set to print 25 copies (arrange on build plate)
- Use recommended settings above
- Generate G-code

### Project Status

- [x] OpenSCAD modified with cherry_mx_switch_holder module
- [ ] Generate cherry_mx_holder.stl
- [ ] Transfer STL to Windows laptop
- [ ] Slice in UltiMaker Cura
- [ ] Print 25 switch holders
- [ ] Order Cherry MX RGB switches
- [ ] Order diodes and screws
- [ ] Assembly and wiring
- [ ] Arduino programming and testing

---

**Last Updated:** January 6, 2026  
**Modification Status:** ✅ Documented - Ready to generate STL

---

## Cherry MX Switch Holder Installation Guide - January 6, 2026

### Understanding How Holders Fit Into Frame

The 25 switch holders are small plates (20mm × 20mm × 6.5mm) that sit inside the hollow frame cavity, screwed to the frame floor. Same holder design works for both white and black keys - just positioned at different distances.

### Assembly Cross-Section

WHITE KEY (15 keys total):
- Magnet at 25mm from front → Holder at 25mm from frame front wall

BLACK KEY (10 keys total):  
- Magnet at 20mm from front → Holder at 20mm from frame front wall

Frame interior height: 17mm
Switch + holder height: 13mm  
Clearance: 4mm ✓

### Complete Parts List

**Already Printed (No Reprinting Needed):**
- white_keys_batch1.stl, white_keys_batch2.stl, white_keys_batch3.stl
- black_keys_batch1.stl, black_keys_batch2.stl
- frame_left.stl, frame_middle.stl, frame_right.stl

**To Print Now:**
- 25× cherry_mx_holder.stl (file ready in Downloads folder)

**To Purchase:**
- 25× Cherry MX RGB switches (~$25-40)
- 75× M3 × 8mm screws (3 per holder, ~$8)
- 25× 1N4148 diodes (~$5)
- 28 AWG wire for matrix (~$8)
- 1× Arduino Mega 2560 (~$15)

**Total Cost:** ~$60-75

### Assembly Steps

1. Print 25× cherry_mx_holder.stl with Cura
2. Flip frame upside down (hollow facing up)
3. Position holders: WHITE at 25mm, BLACK at 20mm from front wall
4. Mark 3 screw holes per holder (75 total marks)
5. Screw holders to frame floor with M3 × 8mm screws
6. Snap Cherry MX switches into 14mm square holes
7. Flip frame right-side up
8. Install keys with pivot pins
9. Test magnet alignment with switches
10. Wire matrix and connect to Arduino


---

## CORRECTION - January 6, 2026

### Microcontroller: ESP32-S3 (NOT Arduino Mega)

**Original Design:** ESP32-S3 MIDI Piano Controller

**Corrected Hardware:**
- Microcontroller: ESP32-S3 (as originally designed)
- Previous documentation incorrectly listed Arduino Mega
- ESP32-S3 advantages:
  - More GPIO pins (enough for 25-key matrix)
  - Built-in USB (native USB MIDI support)
  - WiFi/Bluetooth (wireless MIDI capability)
  - Faster processor (better for real-time MIDI)
  - Lower cost (~$5-10 vs $15 for Mega)

**Corrected Parts List:**
- 25× Cherry MX RGB switches ($25-40)
- 75× M3 × 8mm screws ($8)
- 25× 1N4148 diodes ($5)
- 1× ESP32-S3 DevKit (~$8) ← CORRECTED
- 28 AWG wire ($8)
- **Total: ~$54-69** (cheaper than Arduino Mega version)

**ESP32-S3 Pin Requirements:**
- Matrix scanning: 5 rows + 5 columns = 10 GPIO pins
- RGB LED control: 1 GPIO pin (WS2812B data)
- USB MIDI: Built-in USB (no additional pins needed)
- Plenty of pins remaining for future expansion

**Code Platform:**
- Arduino IDE with ESP32 board support
- OR PlatformIO with ESP32-S3 framework
- USB MIDI library for ESP32


### Arduino Library - MIDIUSB

**Correct Library:** MIDIUSB by Gary Grewal (version 1.0.5+)

**Installation:**
1. Open Arduino IDE
2. Tools → Manage Libraries
3. Search for "MIDIUSB"
4. Install "MIDIUSB by Gary Grewal"

**Why MIDIUSB:**
- Works with ESP32-S3 native USB
- No external MIDI circuit needed
- Simpler than hardware MIDI (no TX/RX pins)
- Plug-and-play USB MIDI device


═══════════════════════════════════════════════════════════════════════
## FINAL CORRECTED DOCUMENTATION - January 6, 2026
═══════════════════════════════════════════════════════════════════════

### Project Summary

25-Key MIDI Piano Controller with Cherry MX RGB Switches

**Microcontroller:** ESP32-S3 DevKit (NOT Arduino Mega)
**Switches:** Cherry MX RGB mechanical switches  
**Library:** MIDIUSB by Gary Grewal
**Design:** Hybrid - keep existing keys/frame, add new switch holders

### Complete Bill of Materials

**Already Printed (DO NOT REPRINT):**
- 15 white keys (3 batches)
- 10 black keys (2 batches)
- 3 frame sections

**To Print Now:**
- 25x cherry_mx_holder.stl (20x20x6.5mm each)

**To Purchase:**
- 25x Cherry MX RGB switches: $25-40
- 1x ESP32-S3 DevKit: $8
- 25x 1N4148 diodes: $5
- 75x M3x8mm screws: $8
- 28 AWG wire: $8
**TOTAL: $54-69**

### ESP32-S3 Configuration

**Why ESP32-S3:**
- Built-in USB MIDI (no external circuit needed)
- WiFi/Bluetooth for wireless MIDI
- Enough GPIO for 5x5 matrix (10 pins)
- Faster processor, lower cost

**Pin Assignment:**
- Rows: GPIO 1,2,3,4,5
- Columns: GPIO 6,7,8,9,10
- RGB LED: GPIO 11 (optional)

**Software:**
- Library: MIDIUSB by Gary Grewal (v1.0.5+)
- Install: Arduino IDE > Tools > Manage Libraries > Search MIDIUSB
- Board: ESP32S3 Dev Module
- USB Mode: USB-OTG (TinyUSB)

### Cherry MX Switch Holder

**Design:** cherry_mx_switch_holder module in ~/midi_piano_realistic.scad
**Dimensions:** 20x20x6.5mm with 14x14mm switch hole
**Mounting:** 3x M3 screws per holder
**Location:** Inside hollow frame cavity (17mm interior, 13mm needed)

**Positioning:**
- WHITE (15): 25mm from front wall, 22mm spacing
- BLACK (10): 20mm from front wall, between whites

### Assembly Process

1. Print 25x holders in UltiMaker Cura
2. Flip frame upside down
3. Position holders (25mm white, 20mm black)
4. Screw to frame floor (75 screws total)
5. Snap switches into 14mm holes
6. Flip frame up, install keys
7. Wire 5x5 matrix with diodes (cathode to column)
8. Connect to ESP32-S3
9. Install MIDIUSB library
10. Upload code and test

### Key Specifications

**WHITE (15):** 150x22x10mm, magnet at 25mm from front
**BLACK (10):** 95x13x14mm, magnet at 20mm from front, +8mm elevation
**Magnets:** 6.2mm dia, 3.2mm tall (already in printed keys)

### Files

**Windows:** C:/Users/ericm/Downloads/cherry_mx_holder.stl (73,839 bytes)
**Mac:** ~/.claude/skills/3d-printing/midi-piano-project.md
**Mac:** ~/midi_piano_realistic.scad

### Status

✅ Design complete
✅ STL generated  
✅ Documentation corrected
⏳ Print 25 holders
⏳ Purchase components
⏳ Assemble and wire

**Last Updated:** January 6, 2026
