# Chicken Lights and Garden Water Box

## Project Overview
3D-printed weather-resistant enclosure for the Raspberry Pi 3 A+ controller that runs bug-attraction LEDs in the chicken run and chicken water solenoid. Mounts inside the barn on a wall.

**Plan change (2026-04-29):** Solenoid controls chicken water from rain barrel only. Garden drip line hooks to outdoor spout (gravity PSI too low for drip line).

Related skill: `homestead-automation` — the electronics, wiring, code, and cron jobs that go inside this box.

---

## Design Summary

| Property | Value |
|----------|-------|
| Outer dimensions | 180 × 140 × 80mm (base) + 4mm lid |
| Interior usable | ~174 × 134 × 76mm |
| Material | PETG |
| Print bed target | 200 × 200mm (fits with ~10mm margin) |
| Closure | Hinge + magnetic closure (no tools, no flexing clips) |
| Wall mount | 4 corner holes through floor (M4 screws) |

---

## Features

### Base (box)
- Open-top rectangular shell, 3mm walls, 4mm floor
- 4× M4 corner mounting holes through floor (reinforced bosses inside) — screws into barn wall
- 60 × 15mm cable entry slot on front wall
- 16 narrow vent slits per side (1.5mm × 40mm) — airflow while rejecting dust
- 12-position zip-tie anchor grid (4×3) on inside floor for strapping Pi, Wanderer, SSRs, bucks anywhere
- **3 hinge lugs** on the back top edge (interleave with lid's 2 lugs)
- **2 magnet shelves** on inside of front wall at the top, each with an 8mm magnet pocket

### Lid
- 180 × 140 × 4mm flat plate matching box footprint
- 14 narrow vent slits (1.5mm × 50mm) cut through the top for heat exhaust
- **2 hinge lugs** on back edge (interleave with base)
- **2 magnet pockets** on underside at front
- No screws, no tools, no fragile snap clips

### Closure mechanism
- **Hinge:** 3mm steel rod (or M3 × 180mm screw) inserted through all 5 interleaved lugs once during assembly. Lid swings up from the top.
- **Magnets:** 4× 8mm diameter × 3mm thick neodymium disc magnets, glued with CA. Two in box shelves, two in lid underside. When closed, gravity drops the lid down, magnets hold it closed with ~3kg pull force.
- To open: lift the front edge of the lid. Magnets release cleanly.

---

## Parts & Hardware

| Item | Qty | Notes |
|------|-----|-------|
| PETG filament | ~230g | Base ~180g, lid ~50g |
| 3mm steel rod × 180mm (or M3×180 screw) | 1 | Hinge pin |
| 8mm dia × 3mm thick neodymium disc magnet | 4 | Amazon pack of 20 is ~$8 |
| M4 × 16mm flat-head screws + wall anchors | 4 | Wall mount |
| CA (super glue) | as needed | For magnets |

---

## Print Settings (PETG)

- Layer height: 0.2mm
- Walls: 4 (weather resistance)
- Infill: 25% gyroid
- Nozzle: 240°C
- Bed: 70°C
- **Base orientation:** open side up (no supports needed for main body)
- **Lid orientation:** top face up. Tree supports required under the hinge lugs and magnet pocket overhangs.
- Estimated time: ~12–14 hrs base, ~3 hrs lid

---

## Files

All paths are on Eric's Windows PC:

| File | Location |
|------|----------|
| OpenSCAD source (parametric) | `C:\Users\ericm\Documents\homestead_enclosure.scad` |
| Base STL | `C:\Users\ericm\Downloads\homestead\homestead_base.stl` |
| Lid STL | `C:\Users\ericm\Downloads\homestead\homestead_lid.stl` |
| Skill copies | `C:\Users\ericm\.claude\skills\chicken-lights-garden-water-box\` |

**Rendering STLs from SCAD:**
```bash
"/c/Program Files (x86)/OpenSCAD/openscad.exe" -D 'part="base"' \
  -o homestead_base.stl homestead_enclosure.scad
"/c/Program Files (x86)/OpenSCAD/openscad.exe" -D 'part="lid"' \
  -o homestead_lid.stl homestead_enclosure.scad
"/c/Program Files (x86)/OpenSCAD/openscad.exe" -D 'part="assembly"' \
  -o homestead_assembly.stl homestead_enclosure.scad
```

---

## Assembly Order

1. Print base (open side up, no supports)
2. Print lid (top face up, tree supports under hinge lugs)
3. Clean up hinge-lug supports carefully
4. Interleave lid lugs into base lugs, insert hinge pin
5. Glue 4 magnets — ensure poles are oriented to ATTRACT (drop one into box pocket, test with another before gluing lid pair)
6. Mount box to barn wall with 4× M4 screws through floor holes
7. Install Pi + Wanderer + SSRs + bucks inside, secured with zip ties through the anchor grid
8. Route cables out through the bottom slot

---

## Design History

- v1: 200×150×90mm with external flanges and 4× M3 corner screws — **oversized** for 200mm bed
- v2a: 180×140×80mm, internal mount bosses, corner lid screws — required screwdriver every time
- v2b: Added knurled thumbscrew knobs — **rejected** (longer screws needed, plastic would strip)
- v2c: Hinge on top edge + flex snap latch on front — **rejected** (cantilever clip fatigue over time)
- **v2d (current):** Hinge + magnetic closure — no flexing, no fatigue, one-hand open/close
