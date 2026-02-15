# Raspberry Pi WireGuard & Pi-hole Setup

**Date:** January 9, 2026
**Device:** Raspberry Pi 3 A+ (MILTONRP3)
**IP Address:** 192.168.1.104
**Status:** Pi-hole COMPLETE, WireGuard PENDING server config

---

## Summary

Migrated WireGuard and Pi-hole from USG/Mac to Raspberry Pi per Claude's recommendation. This consolidates VPN and DNS services on a dedicated low-power device.

---

## Completed Work

### Pi-hole Installation ✓
- **Version:** Pi-hole v6.3, Web v6.4, FTL v6.4.1
- **Interface:** wlan0 (192.168.1.104)
- **Admin URL:** http://192.168.1.104/admin
- **Upstream DNS:** 1.1.1.1, 8.8.8.8
- **Blocklist:** 75,488 domains (StevenBlack/hosts)
- **Status:** Running and serving DNS queries

### WireGuard Installation ✓
- **Version:** wireguard-tools v1.0.20210914
- **Interface:** wg0
- **VPN IP:** 192.168.2.4/32
- **Config Location:** /etc/wireguard/wg0.conf
- **Service:** wg-quick@wg0 (enabled, running)
- **Status:** Sending keepalives, waiting for server-side peer config

---

## WireGuard Configuration

### Pi Keys
```
Private Key: 4Ll8kfPH48svJToniFdxqorU3Hcz/lpK1AcWE0JtTEE=
Public Key:  Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=
```

### Pi Config (/etc/wireguard/wg0.conf)
```ini
[Interface]
PrivateKey = 4Ll8kfPH48svJToniFdxqorU3Hcz/lpK1AcWE0JtTEE=
Address = 192.168.2.4/32

[Peer]
# VPN Server - edenredux.servegame.com
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
Endpoint = 174.54.51.209:51820
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
PersistentKeepalive = 25
```

---

## REMAINING: Add Pi Peer to VPN Server

### Configuration to Add on VPN Server

Add this to the WireGuard server configuration at 192.168.0.1:

```ini
[Peer]
# Raspberry Pi (MILTONRP3) - 192.168.1.104
PublicKey = Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=
AllowedIPs = 192.168.2.4/32
```

### How to Complete This Step

**Option 1: Use Windows PC (Recommended)**
1. Turn on Windows PC (192.168.1.9)
2. Windows has working VPN connection to 192.168.0.x
3. SSH to router: `ssh mac@192.168.0.1` (password: 645866)
4. Edit WireGuard config to add Pi peer
5. Restart WireGuard service on server
6. Test from Pi: `ping 192.168.0.100`

**Option 2: Physical Access**
1. Go to remote location where edenredux.servegame.com is hosted
2. Access the router directly
3. Add Pi peer configuration
4. Restart WireGuard

---

## Credentials

| Device | IP | User | Password | SSH Key |
|--------|-----|------|----------|---------|
| Raspberry Pi | 192.168.1.104 | miltonrp3 | raspberry123 | ✓ Configured |

---

## Network Diagram After Migration

```
Internet
   |
[USG 3P Router] 192.168.1.1
   |
   +-- [Raspberry Pi] 192.168.1.104  ← NEW: Pi-hole + WireGuard
   |         Pi-hole: ACTIVE
   |         WireGuard: 192.168.2.4 (pending server config)
   |
   +-- [Mac] 192.168.1.7  ← Pi-hole can be disabled
   |         Pi-hole: Still running (backup)
   |
   +-- [Windows PC] 192.168.1.9
             WireGuard: WORKING
             Use to complete Pi VPN setup

VPN Network:
   192.168.1.x <-- WireGuard --> [VPN Server 174.54.51.209] <--> 192.168.0.x
```

---

## Verification Commands

### On Raspberry Pi
```bash
# Check Pi-hole status
pihole status

# Check WireGuard status
sudo wg show

# Test DNS
dig google.com @127.0.0.1

# Test VPN (after server config)
ping 192.168.0.100
```

### On Mac
```bash
# Test Pi's Pi-hole
dig google.com @192.168.1.104

# SSH to Pi
ssh miltonrp3@192.168.1.104
```

---

## Migration from Mac Pi-hole

Once Pi-hole on the Raspberry Pi is confirmed working:

1. **Update router DNS settings:**
   - Change DNS from 192.168.1.7 (Mac) to 192.168.1.104 (Pi)

2. **Or update individual devices:**
   - Point DNS to 192.168.1.104

3. **Optionally keep Mac Pi-hole as backup:**
   - Primary DNS: 192.168.1.104 (Pi)
   - Secondary DNS: 192.168.1.7 (Mac)

---

## Benefits of This Migration

1. **Dedicated device** - Pi runs 24/7 with minimal power
2. **Frees Mac resources** - Pi-hole no longer using Mac CPU/memory
3. **More reliable** - Mac can reboot without affecting DNS
4. **Consolidated services** - VPN and DNS on same device
5. **Lower power** - Pi 3 A+ uses ~2.5W vs Mac's higher consumption

---

**Created:** January 9, 2026
**Author:** Claude Code
**Status:** Pi-hole complete, WireGuard pending server configuration
