# Home Assistant on Lenovo ThinkCentre M900 Tiny

**Last Updated:** 2026-03-09
**Status:** ThinkCentre received — OS unknown, setup not started

---

## Purpose

Run Home Assistant (and ESPHome add-on) for smart home automation, starting with the ESP32 closet lights project.

Replaces the Pi 3 A+ which has insufficient RAM (512MB) for Home Assistant.

---

## Hardware

**Lenovo ThinkCentre M900 Tiny**
- CPU: Intel i5-6500T
- RAM: 8GB
- Storage: 256GB SSD
- Power: ~10-15W idle (suitable for 24/7 use)
- Current OS: Unknown — needs to be checked

---

## Planned Setup

1. Check current OS
2. Install Fedora Server (wipe Windows if present)
3. Install Docker
4. Install Home Assistant via Docker (or Home Assistant OS)
5. Install ESPHome add-on in HA
6. Flash ESP32 closet lights via ESPHome

---

## Network

- Will be assigned a static IP on 192.168.12.x
- Connect via ethernet to switch

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/commands/esp32-closet-lights.md`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
