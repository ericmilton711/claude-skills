# Device Inventory - WireGuard & Pi-hole Compatibility

**Date Created:** January 7, 2026
**Last Updated:** January 7, 2026

## Overview
This document provides a comprehensive overview of all network devices and their compatibility with WireGuard VPN and Pi-hole ad blocking.

---

## Device Inventory

### 1. USG 3P (UniFi Security Gateway)
- **IP Address:** 192.168.1.1
- **Role:** Network Router/Gateway
- **OS:** EdgeOS (Vyatta-based)
- **Architecture:** mips64

**WireGuard Status:**
- ✓ **Installed:** Yes (v1.0.20210914)
- ✓ **Configured:** Yes (wg0 interface)
- ✓ **Status:** Active and sending keepalive
- **VPN IP:** 192.168.2.3/32
- **Notes:** 95% complete - needs peer added on remote server

**Pi-hole Status:**
- ⚠️ **Compatible:** Technically yes, but NOT recommended
- **Reason:** Limited router resources, better dedicated to routing tasks
- **Recommendation:** Use dedicated device instead

**Documentation:**
- `/home/mac/.claude/skills/project-management/wireguard-usg-installation-2026-01-06.md`
- `/home/mac/WIREGUARD-STATUS.txt`

---

### 2. Mac (Primary Workstation)
- **IP Address:** 192.168.1.7
- **Role:** Workstation / DNS Server
- **OS:** macOS (Linux-based)

**WireGuard Status:**
- ✓ **Installed:** Yes
- ⚠️ **Status:** Sending but not receiving data (needs troubleshooting)
- **VPN IP:** 192.168.2.2 (same config as Windows)
- **Config File:** `/etc/wireguard/Lambert.conf`
- **Issue:** Mac VPN connection not fully working

**Pi-hole Status:**
- ✓ **Installed:** Yes - CURRENTLY ACTIVE
- ✓ **Status:** Running and serving DNS for entire network
- **Admin Interface:** http://192.168.1.7/admin
- **DNS Port:** 53
- **Services:** Network-wide ad blocking, DNS filtering
- **Notes:** Primary Pi-hole instance for MILTONHAUS2 network

**Documentation:**
- `/home/mac/.claude/skills/project-management/pihole-setup.md`

---

### 3. Windows PC
- **IP Address:** 192.168.1.9
- **Role:** Workstation
- **OS:** Windows (latest)

**WireGuard Status:**
- ✓ **Installed:** Yes - FULLY WORKING
- ✓ **Status:** Active VPN connection to remote network
- **VPN IP:** 192.168.2.2/32
- **VPN Name:** Lambert
- **Config File:** `C:\Users\ericm\Downloads\Lambert.conf`
- **Service:** WireGuardTunnel$Lambert
- **Access:** Can reach 192.168.0.x and 192.168.2.x networks
- **DNS:** Using Pi-hole at 192.168.1.7

**Pi-hole Status:**
- ⚠️ **Compatible:** Yes (via Docker or WSL)
- **Recommendation:** NOT recommended - wastes PC resources
- **Current Setup:** Using Mac's Pi-hole via DNS setting
- **Notes:** Desktop PC better used for other tasks

**Documentation:**
- `/home/mac/.claude/skills/project-management/wireguard-windows-setup.md`

---

### 4. Raspberry Pi 3 A+ (MILTONRP3)
- **IP Address:** 192.168.1.104
- **Role:** DNS Server + VPN Client
- **OS:** Raspberry Pi OS (Debian Trixie, Linux 6.12)
- **Architecture:** aarch64 (ARM64)
- **Connection:** WiFi (wlan0)
- **Hostname:** MILTONRP3

**WireGuard Status:**
- ✓ **Compatible:** YES
- ✓ **Installed:** Yes (wireguard-tools v1.0.20210914)
- ⚠️ **Status:** Configured but PENDING server peer addition
- **VPN IP:** 192.168.2.4/32
- **Config:** /etc/wireguard/wg0.conf
- **Service:** wg-quick@wg0 (enabled)
- **Public Key:** Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=
- **BLOCKER:** Need to add Pi peer to VPN server (requires Windows PC)

**Pi-hole Status:**
- ✓ **Compatible:** YES
- ✓ **Installed:** Yes (v6.3 Core, v6.4 Web, v6.4.1 FTL)
- ✓ **Status:** RUNNING and serving DNS
- **Admin Interface:** http://192.168.1.104/admin
- **DNS Port:** 53
- **Upstream DNS:** 1.1.1.1, 8.8.8.8
- **Blocklist:** 75,488 domains blocked
- **Notes:** Ready to be primary DNS server for network

**SSH Access:**
- **User:** miltonrp3
- **Password:** raspberry123
- **SSH Key:** Configured (passwordless from Mac)

**Documentation:**
- `/home/mac/.claude/skills/project-management/raspberry-pi-wireguard-pihole-setup.md`
- `/home/mac/.claude/skills/project-management/raspberry-pi-setup/raspberry-pi-3-vnc-setup.md`

---

### 5. Remote VPN Server (edenredux.servegame.com)
- **IP Address:** 174.54.51.209
- **Location:** Remote network (192.168.0.x)
- **Role:** VPN Server
- **OS:** Linux (likely)

**WireGuard Status:**
- ✓ **Installed:** Yes - ACTIVE SERVER
- ✓ **Status:** Running and accepting connections
- **Listen Port:** 51820
- **Public Key:** uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
- **Current Peers:**
  - Windows PC (192.168.2.2) - Working
  - Mac (192.168.2.2) - Needs fixing
  - USG 3P (192.168.2.3) - **Needs to be added**
- **Access:** Only accessible via WireGuard VPN (no public SSH)

**Pi-hole Status:**
- ⚠️ **Compatible:** Yes, but NOT recommended
- **Reason:** Would only serve remote 192.168.0.x network
- **Current Setup:** Remote devices can use 192.168.1.7 via VPN tunnel
- **Recommendation:** Not needed on this server

---

## Summary Tables

### WireGuard Compatibility Matrix

| Device | Compatible | Installed | Status | VPN IP | Notes |
|--------|-----------|-----------|---------|---------|-------|
| USG 3P | ✓ Yes | ✓ Yes | ⚠️ 95% Complete | 192.168.2.3/32 | Needs peer added on server |
| Mac | ✓ Yes | ⚠️ Not configured | - | - | Not currently in use |
| Windows PC | ✓ Yes | ✓ Yes | ✓ Working | 192.168.2.2/32 | Fully functional |
| **Raspberry Pi** | ✓ Yes | ✓ Yes | ⚠️ Pending | 192.168.2.4/32 | **Needs peer added on server** |
| VPN Server | ✓ Yes | ✓ Yes | ✓ Active | N/A (server) | Currently serving network |

### Pi-hole Compatibility Matrix

| Device | Compatible | Recommended | Installed | Status | Notes |
|--------|-----------|-------------|-----------|---------|-------|
| USG 3P | ⚠️ Yes | ❌ No | ❌ No | N/A | Router should focus on routing |
| Mac | ✓ Yes | ⚠️ Backup | ✓ Yes | ✓ Active | Can be backup to Pi |
| Windows PC | ⚠️ Yes | ❌ No | ❌ No | N/A | Wastes desktop resources |
| **Raspberry Pi** | ✓ Yes | ✓ **PRIMARY** | ✓ Yes | ✓ **RUNNING** | **Now serving DNS!** |
| VPN Server | ⚠️ Yes | ❌ No | ❌ No | N/A | Only serves remote network |

---

## Network Architecture

### Current Setup (2026-01-09)

```
Internet
   |
   |
[USG 3P Router] 192.168.1.1
   |           WireGuard: 192.168.2.3 (95% complete)
   |
   +-- [Raspberry Pi] 192.168.1.104  ← NEW PRIMARY DNS + VPN
   |         Pi-hole: ✓ RUNNING (can be primary DNS)
   |         WireGuard: 192.168.2.4 (pending server peer)
   |
   +-- [Mac] 192.168.1.7
   |         Pi-hole: ✓ RUNNING (backup DNS)
   |         WireGuard: Not configured
   |
   +-- [Windows PC] 192.168.1.9
             WireGuard: ✓ WORKING (use to complete Pi setup)
             DNS: Using Pi-hole

VPN Tunnel via WireGuard:
   192.168.1.x <---> [VPN Server] <---> 192.168.0.x
                 edenredux.servegame.com
                 174.54.51.209:51820
```

---

## Recommendations

### Immediate Actions (When Windows PC is Available)

1. **Complete Raspberry Pi WireGuard Setup:**
   - Turn on Windows PC (192.168.1.9)
   - SSH via Windows: `ssh mac@192.168.0.1` (password: 645866)
   - Add Pi peer to server config:
     ```ini
     [Peer]
     # Raspberry Pi (MILTONRP3)
     PublicKey = Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=
     AllowedIPs = 192.168.2.4/32
     ```
   - Restart WireGuard on server
   - Test from Pi: `ping 192.168.0.100`

2. **Complete USG WireGuard Setup:**
   - While Windows is on, also add USG peer:
     ```ini
     [Peer]
     # USG 3P
     PublicKey = NZG/IbateRIpY/Y2Nd6j7i0uF1HjoUy0X+aZujaxXiA=
     AllowedIPs = 192.168.2.3/32, 192.168.1.0/24
     ```
   - Test from USG: `ping 192.168.0.100`

3. **Switch Network to Pi's Pi-hole:**
   - Update router DNS from 192.168.1.7 (Mac) to 192.168.1.104 (Pi)
   - Or configure both for redundancy

### Completed ✓

1. ~~**Migrate Pi-hole to Raspberry Pi**~~ - DONE (2026-01-09)
   - Pi-hole v6.3 installed and running
   - 75,488 domains blocked
   - Ready to serve as primary DNS

2. ~~**Install WireGuard on Raspberry Pi**~~ - DONE (2026-01-09)
   - WireGuard configured with VPN IP 192.168.2.4
   - Just needs peer added to server

### Future Enhancements

1. **Network Redundancy:**
   - Primary DNS: 192.168.1.104 (Raspberry Pi)
   - Backup DNS: 192.168.1.7 (Mac)
   - Configure router with both DNS servers

2. **Disable Mac Pi-hole (Optional):**
   - Once Pi is confirmed stable, can disable Mac's Pi-hole
   - Frees up Mac resources

---

## Device Credentials Reference

**Note:** Full credentials stored in `/home/mac/.claude/skills/credentials/`

- **USG 3P SSH:** mlWKaph@192.168.1.1
- **Remote Router SSH:** mac@192.168.0.1
- **Windows PC User:** ericm
- **Network:** MILTONHAUS2

---

## Related Documentation

### WireGuard
- `/home/mac/.claude/skills/project-management/wireguard-usg-installation-2026-01-06.md`
- `/home/mac/.claude/skills/project-management/wireguard-windows-setup.md`
- `/home/mac/WIREGUARD-STATUS.txt`
- `/home/mac/.claude/skills/vpn/SKILL.md`

### Pi-hole
- `/home/mac/.claude/skills/project-management/pihole-setup.md`
- `/home/mac/pihole-windows-setup.md`

### Network Management
- `/home/mac/.claude/skills/lan-management/SKILL.md`
- `/home/mac/.claude/skills/project-management/network-config-session-2026-01-05-evening.md`

### Raspberry Pi
- `/home/mac/.claude/skills/project-management/raspberry-pi-setup/raspberry-pi-3-vnc-setup.md`

---

## Status Summary

### Working
- ✓ Windows PC WireGuard VPN
- ✓ Mac Pi-hole DNS server
- ✓ USG WireGuard installed and configured
- ✓ VPN Server running and accessible

### Needs Attention
- ⚠️ USG peer needs to be added to VPN server (requires Windows PC online)
- ⚠️ Mac WireGuard not receiving data (connection issue)
- ⚠️ Raspberry Pi pending initial setup

### Future Work
- Consider migrating Pi-hole to Raspberry Pi
- Optional: Install WireGuard on Raspberry Pi
- Set up network redundancy with dual Pi-hole instances

---

**Document Owner:** Claude & Eric
**Review Schedule:** Update when network topology changes
**Last Review:** January 7, 2026
