// =============================================================
// Homestead Automation Enclosure  (v2 — hinge + latch)
// Wall-mount PETG enclosure for chicken-run / garden controller
// Houses: Pi 3 A+, Renogy Wanderer, 2x SSR, 2x DROK buck, 2x fan
// Print bed target: 200mm
// Lid: hinged on top long edge, snap-latch on bottom long edge
// =============================================================

$fn = 64;

// ---------- OUTER BOX ----------
box_w   = 180;   // X
box_h   = 140;   // Y (front-to-back when wall-mounted, with hinge at +Y edge)
box_d   = 80;    // Z (interior depth)
wall    = 3;
floor_t = 4;

// ---------- LID ----------
lid_t   = 4;

// ---------- WALL-MOUNT HOLES (through floor, at corners) ----------
mount_boss_od  = 14;
mount_boss_h   = 6;
mount_hole_dia = 4.5;
mount_cs_dia   = 9;
mount_cs_depth = 2.5;
mount_inset    = 12;

// ---------- ZIP-TIE ANCHORS ----------
anchor_cols   = 4;
anchor_rows   = 3;
anchor_margin = 30;
anchor_w      = 12;
anchor_l      = 10;
anchor_h      = 5;
anchor_slot_w = 3.5;
anchor_slot_l = 8;

// ---------- CABLE ENTRY ----------
cable_w = 60;
cable_h = 15;

// ---------- SIDE VENTS (narrow slits, dust-resistant) ----------
vent_rows   = 16;
vent_slot_w = 1.5;
vent_slot_l = 40;
vent_gap    = 3;

// ---------- HINGE (top long edge, pivot along X) ----------
// Pivot axis: Y = box_h + hinge_offset, Z = box_d
hinge_offset     = 4;       // pivot sits 4mm behind back wall
hinge_lug_od     = 8;       // lug OD
hinge_pin_d      = 3.3;     // clearance for 3mm steel rod or M3×180 screw
hinge_clearance  = 0.5;     // gap between adjacent lugs
hinge_segments   = 5;       // 3 box lugs + 2 lid lugs (interleaved)
// segment width = (box_w - (segments-1)*clearance) / segments
hinge_seg_w      = (box_w - (hinge_segments - 1) * 0.5) / hinge_segments;

// ---------- MAGNETIC CLOSURE (front long edge, Y=0 side) ----------
// 2 pairs of disc magnets. Pocket sized for 8mm dia x 3mm thick neodymium.
mag_d         = 8.2;     // pocket diameter (0.2 clearance for glue-in fit)
mag_t         = 3.2;     // pocket depth
mag_shelf_w   = 16;      // shelf (boss) footprint X
mag_shelf_y   = 12;      // shelf footprint Y (extends inward from front wall)
mag_shelf_h   = 10;      // shelf height below box top edge
mag_x_left    = 30;      // left magnet center X
mag_x_right   = 150;     // right magnet center X

// =============================================================
// HELPERS
// =============================================================

// One zip-tie D-loop anchor
module zip_anchor() {
    difference() {
        translate([-anchor_w/2, -anchor_l/2, 0])
            cube([anchor_w, anchor_l, anchor_h]);
        translate([-anchor_slot_w/2, -anchor_slot_l/2, -0.1])
            cube([anchor_slot_w, anchor_slot_l, anchor_h + 0.2]);
    }
}

module anchor_grid() {
    usable_w = box_w - 2*wall - 2*anchor_margin;
    usable_h = box_h - 2*wall - 2*anchor_margin;
    dx = usable_w / (anchor_cols - 1);
    dy = usable_h / (anchor_rows - 1);
    for (i = [0 : anchor_cols - 1])
        for (j = [0 : anchor_rows - 1])
            translate([wall + anchor_margin + i*dx,
                      wall + anchor_margin + j*dy,
                      floor_t])
                zip_anchor();
}

// Corner wall-mount bosses (for M4 screws through floor)
module mount_boss() {
    difference() {
        cylinder(d = mount_boss_od, h = mount_boss_h);
        translate([0, 0, -floor_t - 0.1])
            cylinder(d = mount_hole_dia, h = mount_boss_h + floor_t + 0.2);
        translate([0, 0, mount_boss_h - mount_cs_depth])
            cylinder(d1 = mount_hole_dia, d2 = mount_cs_dia, h = mount_cs_depth + 0.1);
    }
}

module mount_bosses() {
    positions = [
        [mount_inset, mount_inset],
        [box_w - mount_inset, mount_inset],
        [mount_inset, box_h - mount_inset],
        [box_w - mount_inset, box_h - mount_inset]
    ];
    for (p = positions)
        translate([p[0], p[1], floor_t - 0.01]) mount_boss();
}

module mount_floor_holes() {
    positions = [
        [mount_inset, mount_inset],
        [box_w - mount_inset, mount_inset],
        [mount_inset, box_h - mount_inset],
        [box_w - mount_inset, box_h - mount_inset]
    ];
    for (p = positions)
        translate([p[0], p[1], -0.1])
            cylinder(d = mount_hole_dia, h = floor_t + 0.2);
}

module side_vents() {
    slot_count = vent_rows;
    start_y = (box_h - (slot_count * (vent_slot_w + vent_gap) - vent_gap)) / 2;
    for (side = [0, 1]) {
        x_pos = (side == 0) ? -0.1 : box_w - wall - 0.1;
        for (i = [0 : slot_count - 1]) {
            y_pos = start_y + i * (vent_slot_w + vent_gap);
            translate([x_pos, y_pos, floor_t + 15])
                cube([wall + 0.2, vent_slot_w, vent_slot_l]);
        }
    }
}

module cable_slot() {
    translate([(box_w - cable_w)/2, -0.1, floor_t + 10])
        cube([cable_w, wall + 0.2, cable_h]);
}

// =============================================================
// HINGE LUGS
// =============================================================
// A single lug: cylinder at pivot axis + gusset connecting to its parent.
//   parent_side = "box": gusset goes down to top of box back wall
//   parent_side = "lid": gusset goes forward to lid rear edge
//
// Local coords: lug extruded along X (x=0 to x=seg_w).
// Pivot axis at Y=hinge_offset, Z=0  (caller translates into place).

module hinge_lug_box(seg_w) {
    lug_r = hinge_lug_od / 2;
    rotate([0, 90, 0])
        difference() {
            union() {
                // cylindrical body
                cylinder(d = hinge_lug_od, h = seg_w);
                // gusset: solid block from cylinder down to box top wall
                translate([-lug_r, -hinge_offset, 0])
                    cube([hinge_lug_od, hinge_offset + lug_r, seg_w]);
            }
            // pin hole
            translate([0, 0, -0.1])
                cylinder(d = hinge_pin_d, h = seg_w + 0.2);
        }
}

module hinge_lug_lid(seg_w) {
    // Same shape but gusset extends forward (-Y dir) to connect to lid back edge.
    // Caller places this at Z = pivot Z, Y = pivot Y offset backward
    lug_r = hinge_lug_od / 2;
    rotate([0, 90, 0])
        difference() {
            union() {
                cylinder(d = hinge_lug_od, h = seg_w);
                // gusset goes forward toward lid body (in -Y direction, upward in Z)
                translate([-lug_r, -(hinge_offset + lug_r), 0])
                    cube([hinge_lug_od, hinge_offset + lug_r, seg_w]);
            }
            translate([0, 0, -0.1])
                cylinder(d = hinge_pin_d, h = seg_w + 0.2);
        }
}

// Box gets segments 0, 2, 4 (three)
module box_hinge_lugs() {
    for (i = [0, 2, 4]) {
        x0 = i * (hinge_seg_w + hinge_clearance);
        seg = hinge_seg_w;
        translate([x0, box_h + hinge_offset, box_d])
            hinge_lug_box(seg);
    }
}

// Lid gets segments 1, 3 (two)
module lid_hinge_lugs() {
    // These are placed in the LID local coordinate space.
    // Caller arranges relative to lid rear edge (Y = box_h).
    for (i = [1, 3]) {
        x0 = i * (hinge_seg_w + hinge_clearance);
        seg = hinge_seg_w;
        translate([x0, box_h + hinge_offset, 0])
            hinge_lug_lid(seg);
    }
}

// =============================================================
// MAGNETIC CLOSURE
// =============================================================
// Box side: 2 shelves (bosses) extending inward from front wall top,
// each with a magnet pocket drilled downward from the box top plane.
// Lid side: 2 pockets in the underside at matching positions.
// Magnets glue in with CA.

module box_magnet_shelves() {
    // Shelves sit inside the box, anchored to the front wall at the top.
    for (cx = [mag_x_left, mag_x_right]) {
        translate([cx - mag_shelf_w/2,
                  wall - 0.01,                      // flush with inside of front wall
                  box_d - mag_shelf_h])
            difference() {
                cube([mag_shelf_w, mag_shelf_y, mag_shelf_h]);
                // magnet pocket drilled from top
                translate([mag_shelf_w/2, mag_shelf_y/2, mag_shelf_h - mag_t])
                    cylinder(d = mag_d, h = mag_t + 0.1);
            }
    }
}

module lid_magnet_pockets() {
    // Cut upward into the lid from its bottom face.
    // (Caller subtracts from lid body.)
    for (cx = [mag_x_left, mag_x_right]) {
        translate([cx, wall + mag_shelf_y/2 - 0.01, -0.01])
            cylinder(d = mag_d, h = mag_t);
    }
}

// =============================================================
// MAIN BOX (base)
// =============================================================
module base() {
    difference() {
        union() {
            cube([box_w, box_h, box_d]);
            box_hinge_lugs();
            box_magnet_shelves();
        }
        // hollow interior
        translate([wall, wall, floor_t])
            cube([box_w - 2*wall, box_h - 2*wall, box_d]);
        // cable slot (front wall)
        cable_slot();
        // side vents
        side_vents();
        // mount holes through floor
        mount_floor_holes();
        // re-cut magnet pockets (in case interior hollow clipped them)
        for (cx = [mag_x_left, mag_x_right])
            translate([cx, wall + mag_shelf_y/2, box_d - mag_t])
                cylinder(d = mag_d, h = mag_t + 0.1);
    }
    // internal features
    anchor_grid();
    mount_bosses();
}

// =============================================================
// LID
// =============================================================
module lid() {
    difference() {
        union() {
            // flat plate matching box footprint
            cube([box_w, box_h, lid_t]);
            // hinge lugs (at back edge)
            lid_hinge_lugs();
        }
        // magnet pockets (underside of lid, at front)
        lid_magnet_pockets();
        // top vent slits (fully cut through)
        vent_count   = 14;
        vent_w       = 1.5;
        vent_l       = 50;
        vent_spacing = 7;
        vent_total_x = (vent_count - 1) * vent_spacing;
        vent_start_x = (box_w - vent_total_x) / 2;
        vent_start_y = (box_h - vent_l) / 2;
        for (i = [0 : vent_count - 1]) {
            translate([vent_start_x + i*vent_spacing - vent_w/2,
                      vent_start_y,
                      -0.5])
                cube([vent_w, vent_l, lid_t + 1]);
        }
    }
}

// =============================================================
// RENDER TOGGLE
// =============================================================
// part = "base" | "lid" | "assembly"
part = "base";

if (part == "base") base();
else if (part == "lid") translate([0, 0, box_d + 20]) lid();
else if (part == "assembly") {
    base();
    color("skyblue", 0.5)
        translate([0, 0, box_d]) lid();
}
