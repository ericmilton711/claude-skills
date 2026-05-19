# 3D Printed Garden Stakes for Wife

**Status:** Designed and rendered. Ready to print.
**Created:** 2026-05-19

## Overview

3D printable garden label stakes with recessed text for paint-filling. Gift for Rosemary's garden.

## Current Stakes

| Plant | STL File | Lines |
|-------|----------|-------|
| Pineapple Tomatoes | `Garden-Stake-Pineapple-Tomatoes.stl` | PINEAPPLE / TOMATOES |
| Pumpkin Spice Jalapenos | `Garden-Stake-Pumpkin-Spice-Jalapenos.stl` | PUMPKIN SPICE / JALAPENOS |
| Lemon Spice Jalapenos | `Garden-Stake-Lemon-Spice-Jalapenos.stl` | LEMON SPICE / JALAPENOS |
| Fennel | `Garden-Stake-Fennel.stl` | FENNEL |
| Garlic | `Garden-Stake-Garlic.stl` | GARLIC |

## Design Specs

- **Total height:** 150mm
- **Label area:** 45mm wide x 40mm tall, rounded corners
- **Stake width:** 10mm (tapered neck from label to stake)
- **Thickness:** 3.5mm
- **Point:** 20mm pointed tip for pushing into soil
- **Text:** Recessed 1.0mm (fill with acrylic paint or paint marker for contrast)
- **Font:** Liberation Sans Bold, auto-sized to fit label width
- **Two-line support:** Long names split across two lines

## Files

- **OpenSCAD template:** `~/Desktop/garden-stake.scad`
- **STL files:** `~/Desktop/Garden-Stake-*.stl`
- **Printer:** Ender 3 Pro
- **Material:** PLA or PETG (PETG better for outdoor UV resistance)

## How to Add More Plants

1. Open `garden-stake.scad` in OpenSCAD
2. Change `line1` and `line2` at the top (leave `line2 = ""` for single-line names)
3. Render (F6) and export STL (F7)

Or ask Claude to batch-render more via the Python + OpenSCAD pipeline.

## Rendering via Command Line

Single stake:
```
"C:\Program Files (x86)\OpenSCAD\openscad.exe" -o output.stl garden-stake.scad
```

To override the plant name without editing the file:
Write a temp .scad file with the desired `line1`/`line2` values and render that. The `-D` flag has quoting issues in PowerShell.

## Print Settings (Recommended)

- **Layer height:** 0.2mm
- **Infill:** 100% (thin part, needs strength)
- **Supports:** None needed (prints flat on bed)
- **Orientation:** Flat on bed, label face up
- **Walls:** 3+
- **Top/bottom layers:** 4+

## Paint Fill Instructions

1. Print the stake
2. Apply acrylic paint or paint marker over the recessed text
3. Wipe excess off the flat surface with a damp cloth before it dries
4. Letters stay filled with color in the recesses
