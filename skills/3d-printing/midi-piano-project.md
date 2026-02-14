# MIDI Piano Project - Design Documentation

## Project Overview
25-key (2 octave) MIDI controller with 3D printed keys and frame, using Cherry MX switches.

## Key Counts
- **White keys**: 14 (C D E F G A B × 2 octaves)
- **Black keys**: 10 (C# D# F# G# A# × 2 octaves)
- **Total switches**: 24 (14 white + 10 black)

## Piano Key Layout
```
Black keys:  C# D#    F# G# A#   C# D#    F# G# A#
White keys: C  D  E  F  G  A  B  C  D  E  F  G  A  B
Index:      0  1  2  3  4  5  6  7  8  9  10 11 12 13
```

Black key positions (between white keys):
- 0-1 (C#), 1-2 (D#), 3-4 (F#), 4-5 (G#), 5-6 (A#)
- 7-8 (C#), 8-9 (D#), 10-11 (F#), 11-12 (G#), 12-13 (A#)

## Frame Sections (3 pieces for 200mm print bed)
| Section | Width | White Keys | Black Keys |
|---------|-------|------------|------------|
| 0 (left) | 105mm | 5 (keys 0-4) | 3 (C#, D#, F#) |
| 1 (middle) | 100mm | 5 (keys 5-9) | 4 (G#, A#, C#, D#) |
| 2 (right) | 80mm | 4 (keys 10-13) | 3 (F#, G#, A#) |
| **Total** | **285mm** | **14** | **10** |

Note: Section 0 has 5mm extra on left for first key tab clearance (keys offset 5mm right).

## Key Dimensions
### White Keys
- Width: 18mm
- Length: 116mm (sized to reach back wall with tab)
- Height: 8mm
- Spacing: 20mm center-to-center
- Gap: 1mm between keys

### Black Keys
- Width: 9mm
- Length: 68mm
- Height: 12mm
- Elevation: 4mm above white keys

### Notches (for black key clearance)
- Depth: 70mm (must be > black_key_length for keys to pass)
- Width: 6mm per notch

## Frame Dimensions
- Section width: 100mm (section 2: 80mm)
- Section depth: 130mm
- Frame height: 35mm
- Wall thickness: 3mm

## Cherry MX Switch Integration
Switches snap directly into integrated seats in the frame (no separate holders).

### Switch Seats
- Seat size: 18mm × 18mm
- Hole size: 14mm × 14mm (Cherry MX standard)
- Plate thickness: 1.5mm (for snap-fit clips)

### Seat Positions
- **White switch Y**: 12mm from front (accessible from above)
- **Black switch Y**: 60mm from front (aligned with black keys)
- **White plate height**: 14mm from floor
- **Black plate height**: 18mm from floor

### Black Key Seat X Positions (local to each section)
- Section 0: [20, 40, 80] mm
- Section 1: [0, 20, 60, 80] mm
- Section 2: [20, 40, 60] mm

## Pivot Mechanism
Keys have **tabs** at the back that slide into **slots** in the frame back wall.

### Pivot Tabs (on keys)
- White key tab: 3mm wide × 13mm long × 6mm tall (starts 5mm inside key body)
- Black key tab: 3mm wide × 13mm long × 5mm tall (starts 5mm inside key body)
- Position: centered at back of key, mid-height, overlaps 5mm into key for solid connection

### Pivot Slots (in frame back wall)
- White slots: z = 14mm to 26mm (12mm tall)
- Black slots: z = 22mm to 32mm (10mm tall)
- Slot width: 4mm

## Assembly Order
1. Print frame sections (3 pieces)
2. Print keys (white and black batches)
3. Connect frame sections via alignment pegs
4. Insert Cherry MX switches into seats from above
5. Slide key tabs into back wall slots
6. Wire switch matrix
7. Connect to ESP32-S3

## Controller
- **MCU**: ESP32-S3 DevKit
- **Library**: MIDIUSB by Gary Grewal
- **Matrix**: 5×5 (25 switches, using 24)
- **Rows**: GPIO 1-5
- **Columns**: GPIO 6-10

## Files
- Main SCAD: `C:\Users\ericm\OneDrive\Desktop\MIDI\midi_piano_v2.scad`
- Assembly instructions: `C:\Users\ericm\Downloads\MIDI_Piano_Instructions_UPDATED.txt`

### STL Files for Printing (in C:\Users\ericm\Downloads\)
| File | Contents |
|------|----------|
| frame_section_left.stl | Section 0 (105×130×35mm) |
| frame_section_middle.stl | Section 1 (100×130×35mm) |
| frame_section_right.stl | Section 2 (80×130×35mm) |
| white_keys_batch_1.stl | Keys C D E F G (5 keys) |
| white_keys_batch_2.stl | Keys A B C D E (5 keys) |
| white_keys_batch_3.stl | Keys F G A B (4 keys) |
| black_keys_batch_1.stl | 5 black keys |
| black_keys_batch_2.stl | 5 black keys |

## Render Options in SCAD
```scad
// Visualization
full_assembly();      // Complete assembled view
exploded_view();      // Exploded parts view

// Print modules
print_white_batch_1();   // Keys 0-4
print_white_batch_2();   // Keys 5-9
print_white_batch_3();   // Keys 10-13
print_black_batch_1();   // 5 black keys
print_black_batch_2();   // 5 black keys
print_frame_left();      // Section 0
print_frame_middle();    // Section 1
print_frame_right();     // Section 2
```

## Design Changes Log (January 2026)
1. Changed from separate Cherry MX holders to integrated seats in frame
2. Reduced white keys from 15 to 14 (2 complete octaves)
3. Fixed black key seat positions to align with actual key centers (+10mm offset)
4. Reduced section 2 width from 100mm to 80mm for symmetry
5. Increased notch depth from 45mm to 70mm for black key clearance
6. Changed from pivot sockets (holes) to pivot tabs on keys
7. Added slots in frame back wall for key tabs
8. Raised slot positions to match key rest heights
9. Increased frame height from 28mm to 35mm for slot clearance
10. Increased white key length from 108mm to 116mm to reach back wall
11. Fixed pivot tabs to start 5mm inside key body for solid connection
12. Extended section 0 to 105mm with 5mm offset for first key tab clearance
13. Moved black switch seats from Y=32mm to Y=60mm to align with black keys
