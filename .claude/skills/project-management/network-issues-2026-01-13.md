# Network Issues - 2026-01-13

## Summary

Session to fix internet connectivity issues led to discovery of multiple network problems.

---

## Issue 1: Internet Only Works with ExpressVPN (RESOLVED)

### Problem
Internet connectivity only worked when ExpressVPN was connected.

### Root Cause
ExpressVPN's firewall rules (`evpn.a.100.blockAll` in iptables/nftables) were blocking all internet traffic when VPN was disconnected, even with Network Lock disabled.

### Solution
Uninstalled ExpressVPN completely:
```bash
sudo bash -c 'SUDO_USER="" /opt/expressvpn/bin/expressvpn-uninstall.sh'
```

### Status: ✅ RESOLVED

---

## Issue 2: Local Pi-hole Causing DNS Issues (RESOLVED)

### Problem
Mac (192.168.1.7) was using itself as DNS server (local Pi-hole), which couldn't reach upstream DNS when VPN was down.

### Solution
1. Changed DNS to use Raspberry Pi and USG:
   ```bash
   sudo nmcli connection modify "Profile 1" ipv4.dns "192.168.1.104,192.168.1.1" ipv4.ignore-auto-dns yes
   sudo nmcli connection down "Profile 1" && sudo nmcli connection up "Profile 1"
   ```

2. Stopped and disabled local Pi-hole:
   ```bash
   sudo systemctl stop pihole-FTL
   sudo systemctl disable pihole-FTL
   ```

### Status: ✅ RESOLVED

---

## Issue 3: Raspberry Pi Unreachable from Mac (UNRESOLVED)

### Problem
Mac (192.168.1.7, wired Ethernet) cannot reach Raspberry Pi (192.168.1.104, WiFi).

### Symptoms
- Ping to 192.168.1.104: "Destination Host Unreachable"
- ARP entry shows: `<incomplete>` - Layer 2 connectivity failure
- DNS queries to 192.168.1.104 timeout
- Both devices on same subnet (192.168.1.0/24)

### Diagnosis
```bash
$ ping -c 2 192.168.1.104
From 192.168.1.7 icmp_seq=1 Destination Host Unreachable

$ arp -a
MILTONRP3.localdomain (192.168.1.104) at <incomplete> on enp2s0f0
```

### Likely Cause
**WiFi Client Isolation** enabled on the UniFi access point, preventing WiFi clients from communicating with wired devices.

### Proposed Solutions
1. **Disable Client Isolation** in UniFi Controller:
   - Settings → WiFi → [Network Name] → Disable "Client Device Isolation"

2. **Connect Raspberry Pi via Ethernet** instead of WiFi

### Status: ⏸️ BLOCKED - Cannot access UniFi Controller

---

## Issue 4: UniFi Controller Login Failure (UNRESOLVED)

### Problem
Cannot log into UniFi Controller to change WiFi settings.

### Details
- Need access to disable client isolation
- Blocks resolution of Issue 3

### Proposed Solutions
1. Check UniFi Controller URL/IP (usually https://192.168.1.1 or dedicated controller)
2. Try default credentials or reset controller password
3. Access controller via UniFi mobile app
4. SSH into USG/controller if available

### Status: ❌ UNRESOLVED

---

## Current Network State

### DNS Configuration
- **Primary**: 192.168.1.104 (Raspberry Pi) - NOT WORKING (unreachable)
- **Fallback**: 192.168.1.1 (USG) - WORKING
- Internet is functional using USG DNS

### Device Inventory
| Device | IP | Connection | Status |
|--------|-----|------------|--------|
| USG (Gateway) | 192.168.1.1 | Wired | ✅ Reachable |
| Mac (this machine) | 192.168.1.7 | Wired (enp2s0f0) | ✅ Online |
| Raspberry Pi | 192.168.1.104 | WiFi | ❌ Unreachable from Mac |

### Services
| Service | Status |
|---------|--------|
| ExpressVPN | Uninstalled |
| Local Pi-hole | Stopped & Disabled |
| WireGuard (wg0) | Active |

---

## Next Steps

1. [ ] Regain access to UniFi Controller
2. [ ] Disable WiFi client isolation
3. [ ] Verify Raspberry Pi reachable from Mac
4. [ ] Confirm Raspberry Pi DNS working as primary
5. [ ] Consider connecting Raspberry Pi via Ethernet for reliability
