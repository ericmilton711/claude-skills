---
name: internet-troubleshooting
description: Diagnose and fix slow internet issues on Linux (Fedora)
---

# Internet Troubleshooting Skill

## Quick Diagnostics Checklist

Run these in parallel to diagnose:

```bash
# 1. Check connection type and devices
nmcli device status

# 2. Check for VPN/tunnels that may throttle
ip link show | grep -E 'wg|tun|tap'

# 3. Speed test
curl -o /dev/null -w "Speed: %{speed_download} bytes/sec\nTime: %{time_total}s\n" https://speed.cloudflare.com/__down?bytes=10000000

# 4. Ping test (latency + packet loss)
ping -c 5 8.8.8.8

# 5. DNS check
cat /etc/resolv.conf

# 6. Test DNS response time
dig google.com @<dns-server> +time=2 +stats

# 7. Ethernet link details
ethtool <interface> | grep -iE 'speed|duplex|link'

# 8. Network errors
cat /sys/class/net/<interface>/statistics/rx_errors
cat /sys/class/net/<interface>/statistics/tx_errors
```

## Common Fixes

### Dead/Slow DNS Server
**Symptom:** Browsing feels sluggish, pages take seconds to start loading, but once they start they load okay.

**Diagnosis:** Check `/etc/resolv.conf` for DNS servers on wrong subnets or that don't respond:
```bash
dig google.com @<suspect-server> +time=2
```

**Fix:** Switch to Cloudflare + Google DNS:
```bash
sudo nmcli connection modify "<profile-name>" ipv4.dns "1.1.1.1 8.8.8.8" ipv4.ignore-auto-dns yes
sudo nmcli connection down "<profile-name>" && sudo nmcli connection up "<profile-name>"
```
This is permanent and only affects the local machine.

### WireGuard/VPN Throttling
**Symptom:** Speed is significantly slower with VPN active.

**Diagnosis:** Compare speed with and without VPN:
```bash
sudo nmcli connection down wg0
# re-run speed test
```

**Fix:** Disable VPN when not needed, or switch to a closer/faster VPN endpoint.

### T-Mobile Home Internet Specific
- Expected speeds: 33-245 Mbps depending on plan/signal
- Gateway usually at 192.168.12.1
- Check signal metrics (SINR, RSRP) at http://192.168.12.1
- Good 5G: SINR > 10, RSRP > -90
- Position gateway near a window, elevated, away from electronics

## Post-Fix Verification

```bash
# Verify DNS settings persisted
cat /etc/resolv.conf

# Run speed test
curl -o /dev/null -w "Speed: %{speed_download} bytes/sec\n" https://speed.cloudflare.com/__down?bytes=10000000

# Should see 1.1.1.1 and 8.8.8.8 as nameservers
# Speed should be consistent with ISP plan
```

## Eric's Setup (2026-03-16)
- **ISP:** T-Mobile Home Internet
- **Connection:** Wired ethernet (enp2s0f0) at 1000Mbps to T-Mobile gateway
- **Gateway IP:** 192.168.12.1
- **DNS:** Fixed to 1.1.1.1 / 8.8.8.8 (was using dead 192.168.1.104)
- **Speed after fix:** ~126 Mbps
