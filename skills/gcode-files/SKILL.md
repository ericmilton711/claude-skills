# G-code & STL Files Inventory

Eric's 3D printing file collection. Printer: **Creality Ender 3 Pro** (files prefixed `CE3PRO_` are sliced specifically for it).

---

## GitHub STL Repository

**All 202 STL source files are stored in this skill's `stl/` folder on GitHub.**

Gcode files are too large for GitHub (1.3 GB total, many over 50 MB) but can always be re-sliced from the STL source files using Cura or PrusaSlicer for the Ender 3 Pro.

GitHub warnings: `WeeksB_Box.stl` (88 MB) and `WeeksB_HanSolo.stl` (59 MB) exceed the 50 MB recommended size but are under the 100 MB hard limit. If more large STLs are added in the future, consider Git LFS ($5/month for 50 GB).

### STL Folder Structure (`stl/`)

| Folder | Files | Description |
|--------|-------|-------------|
| `Homestead Enclosure/` | 4 | Pi enclosure: base, lid, knurled knob, assembly |
| `Knife/` | 15 | Liner lock knife (V1 + V2 parts) + bass guitar pick |
| `Letters/` | 29 | Full A-Z alphabet + left/right endcaps + spacer |
| `LOTR/` | 1 | Lord of the Rings silhouette |
| `MIDI/` | 38 | MIDI keyboard: keys, frames, switch plates, assemblies (v1-v3) |
| `Misc/` | 43 | Celtic crosses, mushrooms, Falcon Heavy, Yarn Bowl, Benchys, WeeksB gifts, FuzNuz fence parts, RPi2 cases, VESA mounts, etc. |
| `Pacific Rim Robots/` | 6 | Jaegers: Gypsy Danger, Coyote Tango, Crimson Typhoon, Cherno Alpha, Striker Eureka + Kaiju |
| `Raspberry Pi 3A+ Case/` | 4 | Case with dual 30mm fans, vents, camera slot |
| `Servo Motor Mount/` | 8 | Solar tracker: base, arm, heads, GoPro mount, solar cell holder |
| `Snap-Lock Box/` | 3 | Protective box parts |
| `TF2 Sentry/` | 18 | Team Fortress 2 Level 1 Sentry gun (school project) |
| `XP-53 Peregrine (RC Plane)/` | 33 | Multi-part RC plane across 3 download sets |

**Total: 202 STL files, ~348 MB**

### Where the STLs came from (local paths, for reference)

Files were deduplicated from these locations (many had OneDrive and OldDrive_Backup copies):
- `C:\Users\ericm\Desktop\Non-School stuff\` (main source, various subfolders)
- `C:\Users\ericm\Desktop\school stuff\Team Fortress 2 Level 1 Sentry\`
- `C:\Users\ericm\Downloads\homestead\` (knurled_knob + homestead_assembly, unique to Downloads)
- `C:\Users\ericm\servo_camera_mount.stl` (root of home dir)

---

## Gcode Files (local only, NOT on GitHub)

**Location:** `C:\Users\ericm\Desktop\Non-School stuff\gcode files\`
**Total:** ~113 gcode files, ~1.3 GB
**Why not on GitHub:** Too large. Individual files up to 62 MB, total 1.3 GB. Can be re-sliced from STLs.

### Pacific Rim Jaegers
| File | Notes |
|------|-------|
| `CE3PRO_gypsy_danger_print.gcode` | Standard size |
| `BIG_gypsy_danger_print.gcode` | Scaled up |
| `coyote_tango__print.gcode` | |
| `cherno_alpha__print.gcode` | |
| `BIG_crimson_typhoon_prijtn.gcode` | Scaled up |

### XP-53 Peregrine RC Plane
Large multi-part RC plane project. G-code files at top level, source STL/STP files in subfolders organized by download set.

**Sliced G-code (top level):**
- `_aero_lever.gcode`
- `_camera_holder.gcode`
- `_chassis_rear_2.gcode`
- `chassis_front_2.gcode`
- `_dorsal_fin.gcode`
- `dorsal_stabilizer.gcode`
- `_elevator_1.gcode`, `_elevator_2.gcode`, `_elevator_3.gcode`
- `_extension.gcode`, `_extension_cover_top.gcode`
- `_lg_front_1.gcode`, `_lg_front_2.gcode`, `_lg_front_aero_cover.gcode`, `_lg_front_new.gcode`
- `_nose_cone.gcode`
- `_rear_cone.gcode`, `_rear_fin.gcode`, `_rear_fin_elevator.gcode`
- `_rear_lg_new.gcode`, `_rear_lg_suspension_1.gcode`
- `_servo_brace.gcode`
- `_stabilizer_cover_left.gcode`, `_stabilizer_cover_right.gcode`
- `_strut_bottom.gcode`, `_strut_top.gcode`
- `_wheel_cover_f.gcode`, `_wheel_cover_m.gcode`
- `_wing_2.gcode`, `left_wing_1.gcode`, `right_wing_.gcode`
- `chute_barrel.gcode`, `CE3PRO_chute_cover.gcode`, `CE3PRO_chute_servo_cover.gcode`

**Reprints (r_ prefix):**
- `r_chute_barrel.gcode`, `r_chute_cover.gcode`, `r_chute_servo_cover.gcode`
- `r_extension_cover_top.gcode`, `r_rear_lg_suspension_1.gcode`
- `r_strut_bottom.gcode`, `r_strut_top.gcode`, `r_wing_2.gcode`

### MIDI Keyboard
| File | Notes |
|------|-------|
| `MIDI frame_section_right.gcode` | Frame section |
| `frame_left.gcode` | Frame section |
| `frame_middle.gcode` | Frame section |
| `frame_right.gcode` | Frame section |
| `white_keys_batch_1.gcode` | Keys |
| `white_keys_batch_2.gcode` | Keys |
| `white_keys_batch_3.gcode` | Keys |
| `black_keys_batch_1.gcode` | Keys |
| `black_keys_batch_2.gcode` | Keys |
| `cherry_mx_holders.gcode` | Switch mounts (batch) |
| `one_cherry_mx_holder.gcode` | Single switch mount |

### Pan-Tilt Camera Mount
- `1-Pan-tilt-tapa.gcode`
- `2_Pan-tilt-base-camara.gcode`
- `3_Pan-tilt-plataforma.gcode`
- `Pan-tilt-camara.gcode`
- `pan servo motor mount.gcode`

### Flower Press
- `flowerpress_premsador.gcode`
- `base.gcode`
- `handle.gcode`
- `lateral_(x2).gcode`
- `pont.gcode`
- `cargol-screw.gcode`
- `clip_(x2).gcode`
- `brida and clips.gcode`

### Gifts & Keychains
| File | Notes |
|------|-------|
| `gift-card-1-v2.gcode` | Gift card holder |
| `gift-card-2-v2.gcode` | Gift card holder |
| `windup_updated-gift-card-1-v2.gcode` | Wind-up version |
| `windup2_updated-gift-card-2-v2.gcode` | Wind-up version |
| `gracekeychain_size1-.gcode` | Keychain, size 1 |
| `gracekeychain-size2.gcode` | Keychain, size 2 |
| `gracekeychain_size3-.gcode` | Keychain, size 3 |
| `gracekeychain-size 4.gcode` | Keychain, size 4 |
| `_gracekeychain_.gcode` | Keychain, original |
| `WeeksB_Box.gcode` | Gift box |
| `WeeksB_HanSolo and coins.gcode` | Han Solo + coins set |
| `han solo and tie pin.gcode` | Han Solo + TIE pin |
| `coins.gcode` | Coins only |
| `Celtic_cross_1.gcode` | Celtic cross |

### Drumstick Holders
- `drumstick-holders.gcode`
- `CE3PRO_drumstick-holder-45-degrees.gcode`

### Household & Functional Prints
| File | Notes |
|------|-------|
| `Raspberry_Pi_3_A+_Case_with_vents.gcode` | Pi case |
| `cabinet electronics housing.gcode` | Electronics enclosure |
| `Snap-Lock Protective Box.gcode` | Snap-lock box |
| `snap-lock clips.gcode` | Clips for above |
| `Box.gcode` | Generic box |
| `Box Lid.gcode` | Lid for above |
| `Salt and Pepper.gcode` | Salt & pepper shakers |
| `Lid.gcode` | Lid (for shakers?) |
| `Kumiko_Storage_Baskets.gcode` | Japanese-style baskets |
| `CE3PRO_LED box.gcode` | LED project box |
| `LED box.gcode` | LED project box (alt) |
| `planter light whole_set.gcode` | Planter light set |
| `T_POST_LED_HOLDER.gcode` | T-post LED mount |
| `T_Post_Fence_Holder.gcode` | T-post fence holder |
| `rim phone clip_rev3.gcode` | Phone clip for rim |
| `10mm_spacer.gcode` | Spacer |
| `CE3PRO_square.gcode` | Test square |

### Decorative Prints
| File | Notes |
|------|-------|
| `mushrooms.gcode` | Mushroom decoration |
| `Yarn_Bowl_Leaves.gcode` | Yarn bowl with leaf design |
| `gianna lamp base.gcode` | Lamp base for Gianna |
| `Bills Logo.gcode` | Buffalo Bills logo |
| `Big_Falcon_Heavy_low_poly_launch.gcode` | SpaceX Falcon Heavy |
| `head_with_and_without_horns_scaled_up_105.gcode` | Horned head sculpture |
| `dome x4.gcode` | Dome (x4) |
| `BOUTIQUE.gcode` | Sign/text |
| `MYSTICAL.gcode` | Sign/text |
| `ROSE.gcode` | Sign/text |
| `GM.gcode` | Sign/text |
| `Design2.gcode` | Design piece |
| `CE3PRO_Ornament 3DBenchy Mounted on STEMFIE Beam - SPN-ORN-0003 (stemfie.org).gcode` | Benchy ornament |
| `xbowtarget.gcode` | Crossbow target |

### Knife
- `CE3PRO_LinerLockKnfie_-_RearPinV2.gcode` — Liner lock knife rear pin

### Other Files in Folder
- `Gianna submition.JPG` — photo (not gcode)

---

## OctoPrint Integration Notes

OctoPrint does NOT natively talk to GitHub. Options for connecting them:

1. **Community plugin** that pulls files from a GitHub repo into OctoPrint's file manager
2. **Git pull script on the OctoPrint Pi** with a cron job to sync STLs locally
3. **Manual workflow**: download STLs from GitHub, slice in Cura on laptop, upload gcode to OctoPrint via web UI

Key point: OctoPrint prints **gcode**, not STL. Even with GitHub access, STLs still need to be sliced first. Typical workflow: GitHub STL -> slice on laptop -> upload gcode to OctoPrint.

---

## GitHub Size Limits Reference

- **100 MB hard limit** per file (push will be rejected)
- **50 MB warning** per file (push succeeds with warning)
- **Git LFS**: $5/month for 50 GB storage + 50 GB bandwidth (needed if gcode files ever go on GitHub)
- Current STL repo: 348 MB total, largest file 88 MB
