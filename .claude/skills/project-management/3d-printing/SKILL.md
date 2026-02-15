---
name: 3d-printing
description: Manages 3D printing workflow including modeling (OpenSCAD, FreeCAD), slicing, printer setup, calibration, material settings, and troubleshooting. Use when working with STL files, 3D models, slicers, printer issues, or when the user mentions 3D printing, OpenSCAD, Cura, PrusaSlicer, filament, or print quality.
allowed-tools: Read, Write, Edit, Glob, Grep, Bash(openscad:*), Bash(git:*)
---

# 3D Printing Skill

This skill helps with the complete 3D printing workflow from design to finished print.

## Core Responsibilities

1. **3D Modeling** - Create and modify models (OpenSCAD, FreeCAD, Blender)
2. **STL Management** - Repair, scale, analyze STL files
3. **Slicing** - Configure slicers and generate G-code
4. **Printer Setup** - Configure and calibrate printers
5. **Material Profiles** - Optimize settings for different filaments
6. **Print Quality** - Diagnose and fix print issues
7. **Post-Processing** - Finishing techniques and assembly

## 3D Modeling

### OpenSCAD (Parametric CAD)

OpenSCAD is perfect for:
- Parametric designs (easily adjustable dimensions)
- Mechanical parts with precise measurements
- Programmatic/script-based modeling
- Version control friendly (text files)

#### Basic OpenSCAD Structure

```openscad
// Parameters (easy to adjust)
wall_thickness = 2;
box_width = 50;
box_length = 70;
box_height = 30;

// Main design
difference() {
    // Outer box
    cube([box_width, box_length, box_height]);

    // Inner hollow
    translate([wall_thickness, wall_thickness, wall_thickness])
        cube([
            box_width - 2*wall_thickness,
            box_length - 2*wall_thickness,
            box_height
        ]);
}
```

#### Common OpenSCAD Operations

```openscad
// Basic shapes
cube([x, y, z]);
sphere(r=radius);
cylinder(h=height, r=radius);
cylinder(h=height, r1=bottom_radius, r2=top_radius); // cone

// Transformations
translate([x, y, z]) object;
rotate([x_deg, y_deg, z_deg]) object;
scale([x_scale, y_scale, z_scale]) object;
mirror([x, y, z]) object;

// Boolean operations
union() { shape1; shape2; }        // Combine
difference() { shape1; shape2; }   // Subtract shape2 from shape1
intersection() { shape1; shape2; } // Keep only overlap

// Loops and arrays
for (i = [0:5]) {
    translate([i*10, 0, 0])
        cube([5, 5, 5]);
}

// Modules (functions)
module rounded_cube(size, radius) {
    minkowski() {
        cube([size[0]-radius*2, size[1]-radius*2, size[2]-radius*2]);
        sphere(r=radius);
    }
}
```

#### Rendering and Export

```bash
# Preview (F5 in OpenSCAD GUI)
openscad file.scad -o preview.png

# Render to STL (F6 in OpenSCAD GUI)
openscad -o output.stl input.scad

# Render with parameters
openscad -o output.stl -D 'width=100' -D 'height=50' input.scad

# High-quality render
openscad -o output.stl --render input.scad
```

#### OpenSCAD Best Practices

```openscad
// 1. Use parameters at the top
/* [Basic Dimensions] */
width = 50;
height = 30;
depth = 20;

/* [Advanced Settings] */
wall_thickness = 2;
corner_radius = 3;

// 2. Use modules for reusable parts
module mounting_hole(diameter=3, depth=10) {
    cylinder(h=depth, d=diameter);
}

// 3. Add tolerance for prints
tolerance = 0.2;  // Typical 0.1-0.3mm for FDM

// 4. Design with print orientation in mind
// Avoid overhangs > 45 degrees without supports

// 5. Use $fn for smooth curves (increases render time)
$fn = 100;  // Higher = smoother, slower
// Or use $fa and $fs for adaptive resolution
```

### FreeCAD (Full-featured CAD)

```bash
# Install FreeCAD
sudo dnf install freecad

# Launch
freecad
```

**Best for:**
- Complex organic shapes
- Technical drawings
- Assemblies with multiple parts
- Sheet metal designs

### Blender (Artistic modeling)

```bash
# Install Blender
sudo dnf install blender
```

**Best for:**
- Organic/artistic models
- Sculpting
- Miniatures and figurines
- Complex surface details

### STL File Management

#### Check STL File Info

```bash
# View STL statistics (requires meshlab or similar)
# Basic info with file command
file model.stl

# Using OpenSCAD to get dimensions
openscad -o /dev/null --export-format echo model.stl 2>&1 | grep -i dimension
```

#### Repair STL Files

```bash
# Using Meshlab (GUI)
sudo dnf install meshlab
meshlab model.stl

# Using admesh (command line)
sudo dnf install admesh

# Check and repair
admesh --check model.stl
admesh --write-binary-stl=fixed.stl --remove-unconnected-facets --normal-directions --fill-holes model.stl

# Common fixes:
admesh --remove-unconnected-facets input.stl  # Remove floating triangles
admesh --fill-holes input.stl                 # Fill holes in mesh
admesh --normal-directions input.stl          # Fix normals (inside/outside)
```

#### Scale/Modify STL

```bash
# Scale in OpenSCAD
echo 'scale([2, 2, 2]) import("model.stl");' | openscad -o scaled.stl -

# Rotate
echo 'rotate([90, 0, 0]) import("model.stl");' | openscad -o rotated.stl -

# Center on build plate
echo 'translate([0, 0, 0]) import("model.stl");' | openscad -o centered.stl -
```

## Slicing Software

### Cura (Beginner-friendly)

```bash
# Install Cura
sudo dnf install cura

# Or download AppImage from Ultimaker website
chmod +x Ultimaker-Cura*.AppImage
./Ultimaker-Cura*.AppImage
```

**Key Settings:**
- Layer Height: 0.2mm (standard), 0.12mm (fine), 0.28mm (draft)
- Wall Thickness: 1.2mm (3 walls with 0.4mm nozzle)
- Infill: 15-20% (functional), 5-10% (display), 100% (mechanical)
- Print Speed: 50mm/s (general), 30mm/s (first layer)
- Supports: Enable for overhangs > 50°

### PrusaSlicer (Advanced features)

```bash
# Install PrusaSlicer
# Download from prusa3d.com or use AppImage
```

**Advanced Features:**
- Paint-on supports
- Variable layer height
- Organic supports
- Multi-material support
- Better bridging detection

### OrcaSlicer (Bambu Lab fork, very advanced)

**Features:**
- Auto-calibration
- Overhang detection
- Built-in calibration patterns
- Better multi-material handling

### Common Slicer Settings Explained

#### Layer Height
```
0.08-0.12mm: Ultra-fine (miniatures, detailed prints)
0.16mm:      Fine (good detail, reasonable speed)
0.20mm:      Standard (best balance)
0.24-0.28mm: Draft (fast, lower quality)
0.32mm+:     Very fast (vases, rough prototypes)

Rule: Max layer height = 80% of nozzle diameter
0.4mm nozzle → max 0.32mm layers
```

#### Wall/Perimeter Settings
```
Wall Count: 3-4 for structural parts, 2 for decorative
Top/Bottom Layers: 4-6 layers (0.8-1.2mm total)
Wall Thickness = Wall Count × Line Width
```

#### Infill Patterns
```
Grid:        Fast, general purpose
Gyroid:      Strong, low material, good for flexible
Honeycomb:   Very strong but slow
Cubic:       Isotropic strength (equal in all directions)
Lightning:   Fast, minimal material (for supports only)
```

#### Speed Settings
```
First Layer:     20-30mm/s (adhesion critical)
Walls:           40-60mm/s (quality important)
Infill:          60-80mm/s (speed is fine)
Travel:          120-200mm/s (non-printing moves)
```

#### Temperature Guidelines

**PLA:**
```
Nozzle: 190-220°C (start at 200°C)
Bed:    50-60°C (optional, helps adhesion)
Speed:  50-70mm/s
Cooling: 100% after first layer
```

**PETG:**
```
Nozzle: 220-250°C (start at 235°C)
Bed:    70-80°C (important for adhesion)
Speed:  40-60mm/s (slower than PLA)
Cooling: 50% max (too much causes brittleness)
Retraction: Reduce by 1-2mm vs PLA (strings easily)
```

**ABS:**
```
Nozzle: 230-250°C
Bed:    90-110°C (critical - warps without)
Enclosure: Recommended (reduces warping)
Cooling: Minimal or off (causes layer separation)
```

**TPU (Flexible):**
```
Nozzle: 210-230°C
Bed:    40-60°C
Speed:  20-30mm/s (prevents jams)
Retraction: Minimal or disabled
Direct Drive: Recommended (bowden is difficult)
```

## Printer Calibration

### Initial Setup Checklist

```
✅ Bed leveling (manual or auto)
✅ Z-offset calibration
✅ E-steps calibration (extruder)
✅ Flow rate calibration
✅ Temperature tower test
✅ Retraction tuning
✅ Linear advance/pressure advance (if supported)
```

### Bed Leveling

**Paper Method:**
```
1. Home all axes (G28)
2. Disable steppers (M84)
3. Move nozzle to each corner + center
4. Adjust bed screws until paper has slight drag
5. Repeat 2-3 times (adjusting one corner affects others)
```

**Auto Bed Leveling (BLTouch, CR Touch, etc.):**
```gcode
M851 Z0        ; Reset Z-offset
G28            ; Home
M280 P0 S10    ; Deploy probe
G29            ; Run auto leveling
M500           ; Save to EEPROM
```

### Z-Offset Calibration

```
1. Heat bed and nozzle to printing temps
2. Home printer (G28)
3. Move to center: G1 X100 Y100 F3000
4. Lower nozzle close to bed: G1 Z0.2
5. Adjust Z-offset until paper has slight drag
6. Test with single layer square print
7. Adjust if:
   - Too high: gaps between lines
   - Too low: nozzle scrapes, hard to remove
8. Save: M500 (Marlin) or Z_OFFSET in config
```

### E-steps Calibration (Extruder)

```
1. Mark filament 120mm above extruder entry
2. Heat nozzle to printing temp
3. Extrude 100mm: G1 E100 F100
4. Measure remaining distance to mark
5. Calculate:
   New E-steps = Current E-steps × (100 / Actual)
   Example: 100 e-steps, extruded 95mm
   New = 100 × (100/95) = 105.26
6. Set: M92 E105.26
7. Save: M500
8. Test again to verify
```

### Flow Rate Calibration

```
1. Print single-wall cube (no infill, 1 wall)
2. Measure wall thickness with calipers
3. Calculate:
   Flow % = (Expected / Actual) × Current Flow %
   Example: Expected 0.4mm, measured 0.45mm, current 100%
   Flow = (0.4/0.45) × 100 = 88.9%
4. Set in slicer and test
```

### Temperature Tower

```
1. Download temperature tower STL
2. Slice with different temps per section
3. Print and evaluate:
   - Bridging quality
   - Stringing
   - Layer adhesion
   - Surface finish
4. Choose best temperature
```

### Retraction Tuning

```
Print retraction tower to find optimal:
- Retraction Distance: 0.5-6mm (Bowden), 0.5-2mm (Direct)
- Retraction Speed: 25-45mm/s

Settings:
- Too little: stringing, oozing
- Too much: clogs, gaps, under-extrusion
```

## Common Print Issues & Solutions

### First Layer Problems

**Not Sticking:**
```
Solutions:
- Level bed more carefully
- Decrease Z-offset (nozzle closer)
- Increase bed temperature (+5-10°C)
- Clean bed with IPA (isopropyl alcohol)
- Use adhesion aids: glue stick, hairspray, tape
- Slow down first layer (20mm/s)
- Increase first layer line width (120%)
```

**Nozzle Too Close:**
```
Symptoms: Filament builds up, scraping sounds, hard to remove
Solutions:
- Increase Z-offset (+0.05mm increments)
- Re-level bed
```

### Warping/Lifting Corners

**PLA Warping:**
```
Solutions:
- Increase bed temp to 60°C
- Use brim or raft
- Ensure bed is clean and level
- Reduce cooling fan for first few layers
- Use enclosure or reduce drafts
```

**ABS Warping (common):**
```
Solutions:
- Bed temp 100-110°C
- Enclosure required
- Brim/raft mandatory for large prints
- Disable cooling fan or max 25%
- ABS slurry (ABS dissolved in acetone) on bed
```

### Stringing/Oozing

```
Solutions:
- Increase retraction distance (+0.5mm)
- Increase retraction speed (+5mm/s)
- Lower print temperature (-5°C)
- Increase travel speed
- Enable "combing" mode (travel within print)
- Dry filament (moisture causes oozing)
```

### Under-Extrusion

```
Symptoms: Gaps in layers, weak prints, missing infill
Solutions:
- Check if filament is tangled
- Increase flow rate (+5%)
- Check nozzle for clogs
- Verify E-steps calibration
- Increase print temperature (+5-10°C)
- Check for extruder arm cracks (common on Ender 3)
- Reduce print speed
```

### Over-Extrusion

```
Symptoms: Blobs, rough surface, elephants foot
Solutions:
- Reduce flow rate (-5%)
- Verify E-steps calibration
- Reduce print temperature
- Enable horizontal expansion compensation (slicer)
```

### Layer Shifting

```
Causes:
- Belt too loose or tight
- Stepper driver overheating
- Print speed too fast for acceleration
- Mechanical obstruction

Solutions:
- Check belt tension (should twang like guitar string)
- Add cooling to stepper drivers
- Reduce print speed and acceleration
- Check for debris on rails/rods
- Tighten pulley set screws
```

### Stringing Between Parts

```
Solutions:
- Enable Z-hop (lift nozzle during travel)
- Increase retraction
- Print one object at a time
- Reduce temperature
- Dry filament
```

### Poor Overhangs

```
Solutions:
- Add supports (auto or manual)
- Increase cooling (part cooling fan to 100%)
- Reduce print temperature slightly
- Slow down overhang speed
- Rotate part to minimize overhangs
- Design with chamfers instead of overhangs
```

### Layer Separation/Delamination

```
Solutions:
- Increase print temperature (+5-10°C)
- Reduce cooling (especially ABS)
- Check for drafts/cold room
- Increase flow rate slightly
- Clean nozzle
- Use enclosure (ABS/ASA)
```

### Clogged Nozzle

```
Prevention:
- Don't print below minimum temp for filament
- Retract filament when leaving printer heated
- Use quality filament (cheap = debris)
- Dry filament properly

Clearing:
1. Heat to printing temp + 10°C
2. Remove filament
3. Push cleaning filament through
4. Or: Cold pull method
   - Heat to 220°C
   - Insert filament, push until flows
   - Cool to 90°C (still soft)
   - Pull hard - should bring debris out
5. Or: Atomic pull (nylon works best)
6. Last resort: Remove nozzle, soak in acetone, poke with needle
```

### Ringing/Ghosting (Ripples)

```
Solutions:
- Reduce print speed
- Reduce acceleration and jerk settings
- Tighten belts
- Add dampening feet to printer
- Enable input shaping (Klipper firmware)
- Reinforce printer frame
```

## Filament Storage & Maintenance

### Drying Filament

```
Moisture affects: PLA, PETG, Nylon, TPU (ABS less affected)

Symptoms of wet filament:
- Popping/hissing during printing
- Stringing
- Rough surface finish
- Brittle prints
- Steam coming from nozzle

Drying methods:
1. Food dehydrator (best)
   PLA: 40-45°C for 4-6 hours
   PETG: 60-65°C for 4-6 hours

2. Oven (risky - watch carefully)
   Use same temps, check frequently

3. Filament dryer box
   Commercial: PrintDry, Sunlu
   DIY: Storage box + desiccant + low-temp heating
```

### Storage

```
- Sealed bags with desiccant
- Vacuum bags (best for long-term)
- Dry boxes with humidity indicator
- Store in cool, dry place
- Keep target humidity: <20% for best results
```

## Firmware & Control

### Marlin G-code Commands

```gcode
; Homing and positioning
G28          ; Home all axes
G28 Z        ; Home Z only
G1 X50 Y50   ; Move to position
G1 Z10 F300  ; Move Z at 300mm/min

; Temperature
M104 S200    ; Set nozzle temp (don't wait)
M109 S200    ; Set nozzle temp (wait)
M140 S60     ; Set bed temp (don't wait)
M190 S60     ; Set bed temp (wait)

; Motors
M84          ; Disable steppers
M17          ; Enable steppers

; Extrusion
G92 E0       ; Reset extruder position
G1 E10 F300  ; Extrude 10mm at 300mm/min

; Settings
M503         ; View current settings
M500         ; Save to EEPROM
M501         ; Load from EEPROM
M502         ; Reset to defaults

; Info
M115         ; Firmware info
M105         ; Temperature report

; Calibration
M92 E105.26  ; Set E-steps
M851 Z-1.5   ; Set Z-offset
G29          ; Auto bed level (if enabled)
```

### OctoPrint Setup

```bash
# Install on Raspberry Pi
sudo apt update
sudo apt install octoprint

# Access via browser
http://octopi.local

# Features:
- Remote monitoring
- Webcam streaming
- Time-lapse recording
- GCode viewer
- Plugin ecosystem
```

## Design Tips for Printability

### Overhangs
```
0-45°:  No supports needed
45-60°: May print okay with good cooling
60-90°: Supports required

Design alternatives:
- Use chamfers instead of overhangs
- Split model and print separately
- Rotate orientation
- Add integrated supports
```

### Bridging
```
Max bridge distance (without supports):
- PLA: 40-50mm
- PETG: 30-40mm
- ABS: 20-30mm

Tips:
- 100% cooling for bridges
- Slow bridge speed
- Optimal bridging angle: perpendicular to print direction
```

### Wall Thickness
```
Minimum: 2× nozzle diameter (0.8mm for 0.4mm nozzle)
Recommended: 3× nozzle diameter (1.2mm)

For strength:
- Increase walls, not infill
- 4-5 walls stronger than 2 walls + 50% infill
```

### Small Features
```
Minimum feature size:
- Holes: +0.2mm (design 3mm hole = print 3.2mm)
- Pins: -0.2mm (design 3mm pin = print 2.8mm)
- Text: Minimum 5mm tall, 1mm thick
- Layer height affects vertical detail
```

### Tolerances
```
Press fit: -0.1 to -0.2mm
Sliding fit: +0.2 to +0.3mm
Loose fit: +0.5mm

Example bearing holder:
- Bearing diameter: 22mm
- Hole size: 22.2mm (sliding fit)
```

### Print Orientation
```
Consider:
1. Minimize supports
2. Maximize layer adhesion for stress direction
3. Surface finish requirements
4. Print time

Strongest in XY plane, weakest in Z
Orient so stress is perpendicular to layers
```

## Post-Processing

### Support Removal
```
Tools:
- Flush cutters (best)
- Needle-nose pliers
- Utility knife
- Deburring tool

For difficult supports:
- Freeze print (supports become brittle)
- PVA/HIPS supports (dissolve in water/limonene)
```

### Sanding
```
Progression:
120 grit → 220 → 400 → 600 → 1000 grit (wet sand)

For PLA:
- Wet sand to avoid clogging
- Optional: Heat gun to smooth slightly (careful!)
```

### Painting
```
1. Sand to 220-400 grit
2. Prime with filler primer (Rust-Oleum, Krylon)
3. Sand primer smooth
4. Paint with acrylic or spray paint
5. Clear coat for protection
```

### Smoothing

**PLA:**
```
- Filler primer + sanding (best)
- Heat gun (risky, practice on scrap)
```

**ABS/ASA:**
```
- Acetone vapor bath (chemical smoothing)
  1. Place print on platform in container
  2. Add acetone to bottom (not touching print)
  3. Heat gently (50-60°C)
  4. Acetone vapor melts surface
  5. 2-10 minutes depending on size
  Warning: Flammable, well-ventilated area only!
```

**PETG:**
```
- Difficult to chemically smooth
- Best: Good print settings + sanding
- Dichloromethane works but very dangerous
```

### Assembly

```
- Cyanoacrylate (super glue): Fast, brittle
- 2-part epoxy: Strong, slow cure
- Solvent welding (ABS/PLA): Strongest
- Heat welding: Requires soldering iron, very strong
- Threaded inserts: M3 brass inserts + heat
```

## Project Workflow

### Complete Print Workflow

```
1. Design/Model
   - OpenSCAD/FreeCAD/Blender
   - Check dimensions, tolerances
   - Export as STL

2. Verify STL
   - Check for errors (admesh)
   - Repair if needed
   - Scale/orient as needed

3. Slice
   - Import to slicer
   - Choose material profile
   - Adjust settings for part requirements
   - Preview layers, check issues
   - Export G-code

4. Pre-print Check
   - Bed leveled
   - Nozzle clean
   - Filament loaded and dry
   - Bed cleaned (IPA)

5. Print
   - Monitor first layer closely
   - Check periodically
   - Adjust if issues appear

6. Post-processing
   - Remove supports
   - Sand/finish as needed
   - Paint if desired

7. Test and iterate
   - Check fit and function
   - Adjust model if needed
   - Document successful settings
```

## Useful Resources

### Calibration Prints
```
- XYZ calibration cube (20mm)
- Temperature tower
- Retraction tower
- Bridging test
- Overhang test
- Stringing test
```

### Communities
```
- r/3Dprinting (Reddit)
- r/FixMyPrint (troubleshooting)
- Prusa forums
- Thingiverse / Printables (models)
```

## Success Criteria

A well-tuned printer produces:
- ✅ Clean first layers (no gaps or elephants foot)
- ✅ Minimal stringing between features
- ✅ Accurate dimensions (±0.1mm for well-tuned)
- ✅ Strong layer adhesion (parts don't delaminate)
- ✅ Clean overhangs up to 50° without supports
- ✅ Consistent print quality across different models
- ✅ Reliable prints without constant monitoring
