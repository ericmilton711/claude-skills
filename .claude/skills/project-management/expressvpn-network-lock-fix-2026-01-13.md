# ExpressVPN Network Lock Fix

**Date**: 2026-01-13
**Issue**: Internet only works when ExpressVPN is connected

## Problem

User reported that internet connectivity was only available when ExpressVPN was active. Without VPN, no websites could be accessed.

## Diagnosis

### Network State Analysis

**VPN Status**:
- ExpressVPN connected to `usa-new-jersey-3`
- Protocol: LightwayUdp
- Network Lock: enabled when connected

**Routing Table** showed VPN capturing all traffic:
```
0.0.0.0/1 via 100.64.100.5 dev tun0
128.0.0.0/1 via 100.64.100.5 dev tun0
```

These two routes (`0.0.0.0/1` and `128.0.0.0/1`) together capture ALL internet traffic and force it through the VPN tunnel (`tun0`).

**DNS Configuration**:
- VPN DNS: `100.64.100.1` via tun0
- Local DNS: `192.168.1.7` via enp2s0f0

### Root Cause

**Network Lock** (ExpressVPN's kill switch feature) was enabled. This feature intentionally blocks all internet traffic when the VPN is disconnected to prevent data leaks and protect privacy.

## Solution

Disabled Network Lock to allow internet access without VPN:

```bash
sudo /opt/expressvpn/bin/expressvpnctl set networklock false
```

### Verification

```bash
/opt/expressvpn/bin/expressvpnctl get networklock
# Output: false
```

## Result

Internet now works with or without ExpressVPN connected.

## Notes

- ExpressVPN CLI tool location: `/opt/expressvpn/bin/expressvpnctl`
- The `expressvpn` command doesn't exist; use `expressvpnctl` instead
- Changing Network Lock setting requires sudo/root privileges
- Network Lock can be re-enabled with: `sudo /opt/expressvpn/bin/expressvpnctl set networklock true`

## Trade-offs

**With Network Lock enabled**:
- Maximum privacy protection
- Prevents accidental data leaks if VPN drops
- Requires VPN for any internet access

**With Network Lock disabled**:
- Internet works without VPN
- Risk of data exposure if VPN disconnects unexpectedly
- More flexibility in connectivity

---

## Update: ExpressVPN Uninstalled

**Date**: 2026-01-13

### Problem
Even with Network Lock disabled, ExpressVPN's firewall rules (`evpn.a.100.blockAll` and others in iptables/nftables) were still blocking internet traffic when VPN was disconnected.

### Solution
Uninstalled ExpressVPN completely:
```bash
sudo bash -c 'SUDO_USER="" /opt/expressvpn/bin/expressvpn-uninstall.sh'
```

### Additional Changes Made
1. **DNS changed** from local Pi-hole (192.168.1.7) to:
   - Primary: Raspberry Pi (192.168.1.104)
   - Fallback: USG (192.168.1.1)

   Command used:
   ```bash
   sudo nmcli connection modify "Profile 1" ipv4.dns "192.168.1.104,192.168.1.1" ipv4.ignore-auto-dns yes
   sudo nmcli connection down "Profile 1" && sudo nmcli connection up "Profile 1"
   ```

2. **Local Pi-hole stopped and disabled**:
   ```bash
   sudo systemctl stop pihole-FTL
   sudo systemctl disable pihole-FTL
   ```

### Result
- Firewall rules cleared (no more evpn.* chains)
- Internet works without VPN
- DNS handled by Raspberry Pi and USG
