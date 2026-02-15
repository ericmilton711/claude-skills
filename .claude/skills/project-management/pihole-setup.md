# Pi-hole Setup Documentation

## Overview
This document describes how to configure devices on the MILTONHAUS2 network to use Pi-hole for network-wide ad blocking.

**Pi-hole Location:** Mac at IP address `192.168.1.7`
**Network:** MILTONHAUS2 (192.168.1.x subnet)
**Router Gateway:** 192.168.1.1

---

## Option 1: Configure Individual Devices

Use this method to configure Pi-hole DNS on specific laptops/devices.

### Windows PC Configuration

1. **Open PowerShell as Administrator:**
   - Press Windows key
   - Type "PowerShell"
   - Right-click "Windows PowerShell"
   - Select "Run as administrator"
   - Click "Yes" on UAC prompt

2. **Set DNS to Pi-hole:**
   ```powershell
   Set-DnsClientServerAddress -InterfaceAlias 'Wi-Fi 4' -ServerAddresses 192.168.1.7
   ```

   > **Note:** Replace 'Wi-Fi 4' with your network adapter name if different.
   > To find your adapter name, run: `Get-NetAdapter`

3. **Verify DNS Configuration:**
   ```powershell
   Get-DnsClientServerAddress -InterfaceAlias 'Wi-Fi 4' -AddressFamily IPv4
   ```
   Should show `192.168.1.7` as the DNS server.

4. **Test DNS Resolution:**
   ```cmd
   nslookup google.com
   ```
   Should show `Server: pi.hole` and `Address: 192.168.1.7`

### Alternative: Manual GUI Configuration (Windows)

1. Press `Windows + R`, type `ncpa.cpl`, press Enter
2. Right-click your network adapter (e.g., "Wi-Fi 4") → Properties
3. Double-click "Internet Protocol Version 4 (TCP/IPv4)"
4. Select "Use the following DNS server addresses"
5. Enter `192.168.1.7` as Preferred DNS server
6. Click OK to save

### macOS Configuration

1. Open System Settings → Network
2. Select your connection (Wi-Fi or Ethernet)
3. Click "Details"
4. Go to DNS tab
5. Click "+" and add `192.168.1.7`
6. Remove any other DNS servers
7. Click OK

### iOS/Android Configuration

1. Go to Wi-Fi settings
2. Tap on MILTONHAUS2 network
3. Configure DNS manually
4. Set DNS to `192.168.1.7`
5. Save settings

---

## Option 2: Router-Wide Configuration (RECOMMENDED)

**This is the easiest option for multiple devices!**

Configure your router once, and ALL devices on the network automatically use Pi-hole.

### Steps:

1. Access your router admin interface:
   - Open browser to `http://192.168.1.1`
   - Login with router credentials

2. Find DNS settings (location varies by router):
   - Usually under: LAN Settings, DHCP Settings, or DNS Settings
   - Look for "Primary DNS" or "DNS Server"

3. Set DNS to Pi-hole:
   - Primary DNS: `192.168.1.7`
   - Secondary DNS: Leave blank or use `1.1.1.1` as fallback

4. Save and reboot router if required

5. **Reconnect devices** or wait for DHCP lease renewal

### Benefits:
- No need to configure each device individually
- New devices automatically get ad blocking
- Works on smart TVs, IoT devices, gaming consoles, etc.

---

## Accessing Pi-hole Admin Interface

**Web Interface:** http://192.168.1.7/admin
**Or use:** http://pi.hole/admin (if DNS is configured)

### What You Can See:
- Total queries blocked
- Blocklist statistics
- Query log (see which devices are making DNS requests)
- Whitelist/blacklist management
- DNS settings and configuration

---

## Verification & Testing

### Check if Pi-hole is Working:

1. **Check DNS server:**
   ```cmd
   nslookup google.com
   ```
   Should show `Server: pi.hole` at `192.168.1.7`

2. **Visit Pi-hole admin:**
   - Go to http://192.168.1.7/admin
   - Check if queries are being logged
   - Look for your device IP in query log

3. **Test ad blocking:**
   - Visit a website with ads
   - Check Pi-hole query log for blocked domains
   - Ads should be blocked/missing

### Troubleshooting:

**DNS not resolving:**
- Verify Pi-hole Mac is powered on and connected
- Ping `192.168.1.7` to check connectivity
- Check DNS settings are correct

**Ads still showing:**
- Clear browser cache
- Some ads use first-party domains (harder to block)
- Check Pi-hole blocklists are enabled

**Slow internet:**
- Check Pi-hole is responding (ping 192.168.1.7)
- May need to set secondary DNS as fallback

---

## Network Information

| Device | IP Address | Notes |
|--------|------------|-------|
| Router/Gateway | 192.168.1.1 | Network gateway |
| Pi-hole (Mac) | 192.168.1.7 | DNS server |
| Windows PC | 192.168.1.9 | Example client |

**Network:** 192.168.1.0/24 (255.255.255.0)
**SSID:** MILTONHAUS2

---

## Summary

- **Pi-hole installed on:** Mac (192.168.1.7)
- **For individual devices:** Configure DNS to 192.168.1.7
- **For whole network:** Configure router DNS to 192.168.1.7 (recommended)
- **Admin interface:** http://192.168.1.7/admin

**Setup completed:** January 3, 2026
