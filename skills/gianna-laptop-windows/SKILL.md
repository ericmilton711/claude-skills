# Gianna's Laptop — Windows Reinstall

**Last Updated:** 2026-04-04
**Status:** ⬜ Blocked — needs repair shop to clear BIOS password

---

## Hardware

- **Brand/Model:** Acer Aspire A515-46 series (note: originally thought to be Asus — it's Acer)
- **Serial Number:** NXABRAA00715113E9D7600
- **Current OS:** Fedora Linux
- **Target OS:** Windows (original OS)
- **Network IP:** 192.168.12.226
- **Boot Media:** Windows Media Install on mini SD card

---

## The Problem — BIOS Password

The BIOS asks for a supervisor password before allowing boot order changes or external boot.

### What Was Tried
- Blank password (Enter) — failed
- `admin` — failed
- `1234` — failed
- `0000` — failed
- CMOS battery removal — **did not work** (modern UEFI stores password in EEPROM, not CMOS)
- F12 boot menu — Fedora boots instead (BIOS password blocks it too)

### Why Nothing Worked
The A515-46 uses **Insyde H2O BIOS**. On this model:
- BIOS supervisor password is stored in EEPROM/flash, not CMOS — battery removal won't clear it
- No hash code is displayed after failed attempts (intentional on post-2019 Acer models)
- Serial-number-based master password generators don't work without a hash
- Gianna did not set this password — origin unknown

### BIOS Entry / Boot Keys
- **F2** — BIOS setup (prompts for password)
- **F12** — one-time boot menu (also blocked by password)

---

## Solution — DIY BIOS Reflash (CH341A Programmer)

### What to Order
- **CH341A USB programmer + SOIC8 clip kit** (~$10-15 on Amazon)
- Search: "CH341A programmer SOIC8 clip kit"
- Make sure the SOIC8 clip is included — some listings sell the programmer alone

### How It Works
The clip attaches directly to the BIOS chip on the motherboard (no soldering).
The CH341A connects to a PC via USB and reflashes the chip, wiping the password.

### Steps (once programmer arrives)
- TBD — will walk through when hardware is in hand

---

## After BIOS is Cleared — Windows Install Steps

1. Insert mini SD card with Windows Media Install
2. Power on, tap **F12** immediately → select SD card from boot menu
3. Follow Windows setup wizard
4. After Windows is installed, set up Pi-hole and WireGuard (see `skills/kids-laptops-pihole/SKILL.md`)

---

## Pi-hole Setup (after Windows install)

- Pi-hole group pending for Gianna's laptop
- IP: 192.168.12.226
- Follow the same pattern as Kids1/Kids2 in `skills/kids-laptops-pihole/SKILL.md`
- Determine whitelist needs (may differ from Kids1/Kids2)

---

## Related

- Kids laptops Pi-hole: `skills/kids-laptops-pihole/SKILL.md`
- All devices: `skills/miltonhaus-devices/SKILL.md`
