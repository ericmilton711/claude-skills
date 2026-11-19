;FLAVOR:Marlin
;TIME:0
;Filament used: 0m
;Layer height: 0.16
;MINX:2.14748e+06
;MINY:2.14748e+06
;MINZ:2.14748e+06
;MAXX:-2.14748e+06
;MAXY:-2.14748e+06
;MAXZ:-2.14748e+06
;TARGET_MACHINE.NAME:Creality Ender-3 Pro
;Generated with Cura_SteamEngine 5.10.2
; Ender 3 Pro Start G-code
G90 ; Use absolute positioning
M82 ; Set extruder to absolute mode
M104 S200 ; Start heating nozzle
M140 S60 ; Start heating bed
M190 S60 ; Wait for bed temp
M109 S200 ; Wait for nozzle temp
G28 ; Home all axes
G92 E0 ; Reset extruder position
G1 Z2.0 F3000 ; Raise Z
G1 X10.1 Y20 Z0.28 F5000.0 ; Move to start
G1 X10.1 Y200.0 Z0.28 F1500.0 E15 ; Prime line
G1 X10.4 Y200.0 Z0.28 F5000.0 ; Move over
G1 X10.4 Y20 Z0.28 F1500.0 E30 ; Second line
G92 E0 ; Reset extruder
G1 Z2.0 F3000 ; Raise Z
M82 ;absolute extrusion mode
G92 E0
G92 E0
G1 F1500 E-6.5
;LAYER_COUNT:0
M140 S0
M107
G91 ;Relative positioning
G1 E-2 F2700 ;Retract a bit
G1 E-2 Z0.2 F2400 ;Retract and raise Z
G1 X5 Y5 F3000 ;Wipe out
G1 Z10 ;Raise Z more
G90 ;Absolute positioning

G1 X0 Y220.0 ;Present print
M106 S0 ;Turn-off fan
M104 S0 ;Turn-off hotend
M140 S0 ;Turn-off bed

M84 X Y E ;Disable all steppers but Z

M82 ;absolute extrusion mode
;End of Gcode
;SETTING_3 {"global_quality": "[general]\\nversion = 4\\nname = Dynamic Quality 
;SETTING_3 #2\\ndefinition = creality_ender3pro\\n\\n[metadata]\\ntype = quality
;SETTING_3 _changes\\nquality_type = adaptive\\nsetting_version = 25\\n\\n[value
;SETTING_3 s]\\nadhesion_type = skirt\\nsupport_enable = False\\nsupport_structu
;SETTING_3 re = tree\\n\\n", "extruder_quality": ["[general]\\nversion = 4\\nnam
;SETTING_3 e = Dynamic Quality #2\\ndefinition = creality_ender3pro\\n\\n[metada
;SETTING_3 ta]\\ntype = quality_changes\\nquality_type = adaptive\\nintent_categ
;SETTING_3 ory = default\\nposition = 0\\nsetting_version = 25\\n\\n[values]\\ni
;SETTING_3 nfill_pattern = zigzag\\ninfill_sparse_density = 30\\nsupport_type = 
;SETTING_3 buildplate\\ntop_bottom_thickness = 1\\nwall_thickness = 1.0\\n\\n"]}
