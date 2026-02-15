# MIDI Keyboard 3D Printing Skill

## Overview
3D printable MIDI keyboard controller with 2 octaves (14 white keys, 10 black keys) designed for Cherry MX hall effect switches.

## Project Location
`C:\Users\ericm\OneDrive\Desktop\MIDI\`

## Design Features
- **Keys**: Pivot at the back through slots in back panel (like real electronic keyboards)
- **White keys**: Have notches cut out for black keys (C/F = right notch, D/G/A = both sides, E/B = left notch)
- **Black keys**: Sit in the notches, pivot through higher slots
- **Switch plates**: Separate flat plates with 14mm square holes for Cherry MX switches
- **Frame**: Split into 2 halves to fit 200mm x 200mm print bed

## Key Specifications
- 14 white keys (2 octaves, C to B)
- 10 black keys
- 24 total Cherry MX switch positions
- Key spacing: 15mm center-to-center
- White key dimensions: 14mm wide x 80mm long x 10mm thick
- Black key dimensions: 10mm wide x 50mm long x 8mm thick
- Notch width: 12mm (clearance for black keys)
- Notch length: 55mm (allows black keys to press down)

## Print Files (10 total)

### Frame (2 pieces)
- `frame_left.stl` - Left half with support posts
- `frame_right.stl` - Right half with support posts

### Switch Plates (2 pieces)
- `white_switch_plate.stl` - 14 holes for white key switches (1.5mm thick)
- `black_switch_plate.stl` - 10 holes for black key switches (1.5mm thick)

### Back Panels (2 pieces)
- `panel_left.stl` - Left half with pivot slots
- `panel_right.stl` - Right half with pivot slots

### Keys (4 batches)
- `white_keys_batch1.stl` - 7 white keys (first octave C-B)
- `white_keys_batch2.stl` - 7 white keys (second octave C-B)
- `black_keys_batch1.stl` - 5 black keys
- `black_keys_batch2.stl` - 5 black keys

## Assembly Order
1. Print all parts
2. Snap Cherry MX switches into switch plates (switches point UP)
3. Assemble frame halves together
4. Place white switch plate on lower posts in frame
5. Place black switch plate on higher posts in frame
6. Insert back panel into frame (slots face the keys)
7. Insert keys through the back panel slots (pivot tabs go through slots)
8. Keys rest on switch stems, press down to actuate

## How It Works
- Keys are levers that pivot at the BACK through slots in the back panel
- When you press the front of a key, it rotates and pushes down on the switch stem
- The switch's internal spring returns the key to rest position
- Switch plates rest on POSTS (not rails) so switch bodies can hang below

## Source Files
- `midi_keyboard_v3.scad` - Main OpenSCAD parametric model
- `export_*.scad` - Individual export files for each part

## Export Commands
```bash
# Export all parts using OpenSCAD
openscad -o frame_left.stl export_frame_left.scad
openscad -o frame_right.stl export_frame_right.scad
openscad -o white_switch_plate.stl export_white_plate.scad
openscad -o black_switch_plate.stl export_black_plate.scad
openscad -o panel_left.stl export_panel_left.scad
openscad -o panel_right.stl export_panel_right.scad
openscad -o white_keys_batch1.stl export_white_keys_1.scad
openscad -o white_keys_batch2.stl export_white_keys_2.scad
openscad -o black_keys_batch1.stl export_black_keys_1.scad
openscad -o black_keys_batch2.stl export_black_keys_2.scad
```

## Printer Requirements
- Build plate: 200mm x 200mm minimum
- Each part fits within this constraint

## Bill of Materials
- 24x Cherry MX hall effect switches
- 3D printed parts (PLA or PETG recommended)
- Electronics for MIDI output (microcontroller, wiring)
