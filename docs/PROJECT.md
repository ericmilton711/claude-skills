# Home Network & VPN Project

## Overview
Setting up and managing home network infrastructure including VPN connectivity, DNS management, and network security.

## Goals
1. Establish secure VPN connectivity for remote access and privacy
2. Implement network-wide ad blocking and DNS filtering
3. Maintain secure and efficient network configuration
4. Document all network changes and configurations

## Current Status
**Status**: 🟢 In Progress

Last Updated: 2026-01-02

Current Phase: Infrastructure Setup

## Recent Progress
- ✅ Uninstalled ExpressVPN (2026-01-02)
- ✅ Set up WireGuard VPN with Lambert configuration (2026-01-02)
  - Server: edenredux.servegame.com:51820
  - VPN IP: 192.168.2.2
  - Split tunneling for 192.168.0.0/24 and 192.168.2.0/24
  - Updated DNS to use Pi-hole (192.168.1.7)
- ✅ Installed and configured Pi-hole (2026-01-02)
  - Web Interface: http://192.168.1.7/admin
  - Admin Password: PASSword!?1711
  - DNS Server: 192.168.1.7:53
  - Blocking 82,907 ad domains
  - Tested and working
- ✅ Configured Firefox to use Pi-hole DNS (2026-01-02)
  - Disabled DNS over HTTPS (DoH)
  - Profile: Y7khd5ii.Profile 1
  - All DNS queries now go through Pi-hole
- ✅ Blocked YouTube entirely (2026-01-02)
  - Complete domain-level block via Pi-hole
  - Includes youtube.com, googlevideo.com, youtu.be, etc.
  - Successfully preventing access
- ✅ Installed Beeper Chat (2026-01-02)
  - Version: 4.2.367 (AppImage)
  - Location: /home/mac/.local/bin/Beeper.AppImage
  - Desktop launcher configured
  - FUSE libraries installed for AppImage support

## Next Steps
1. Configure additional devices to use Pi-hole DNS (192.168.1.7)
2. Optional: Set router DNS to Pi-hole for network-wide coverage
3. Optional: Add custom blocklists to Pi-hole
4. Monitor Pi-hole statistics and performance
5. Set up Beeper chat accounts and bridges

## Tech Stack
- **VPN**: WireGuard
- **DNS/Ad Blocking**: Pi-hole v6.3
- **Operating System**: Fedora Linux 43
- **Network**: 192.168.1.0/24 (local), 192.168.2.0/24 (VPN)
- **Chat**: Beeper v4.2.367 (Universal chat aggregator)
- **Browser**: Firefox (DoH disabled for Pi-hole compatibility)

## Resources
- WireGuard Config: /etc/wireguard/Lambert.conf
- Pi-hole Config: /etc/pihole/pihole.toml
- Pi-hole Web Admin: http://192.168.1.7/admin (Password: PASSword!?1711)
- Beeper AppImage: /home/mac/.local/bin/Beeper.AppImage
- Firefox Profile: /home/mac/.mozilla/firefox/Y7khd5ii.Profile 1
- Network Interface: enp2s0f0 (192.168.1.7)
- VPN Interface: Lambert (192.168.2.2)

## Key Configurations
- **Firefox DNS**: DoH disabled (network.trr.mode=5) to use Pi-hole
- **YouTube**: Completely blocked via Pi-hole domain blocking
- **WireGuard DNS**: Using Pi-hole (192.168.1.7) instead of VPN DNS
- **AppImage Support**: FUSE and fuse-libs installed

## Windows PC Network Configuration (2026-01-03)

### Pi-hole DNS Configuration
- ✅ Configured Windows PC (192.168.1.9) to use Pi-hole DNS
  - Network: MILTONHAUS2 (Wi-Fi 4 adapter)  - DNS Server: 192.168.1.7 (Pi-hole on Mac)
  - Method: PowerShell Set-DnsClientServerAddress command
  - Verified DNS resolution working through Pi-hole
  - All DNS queries now show in Pi-hole query log

### WireGuard VPN Configuration
- ✅ Installed WireGuard for Windows
  - Version: Latest (installed 2026-01-03)
  - Installation path: C:\Program Files\WireGuard- ✅ Configured Lambert VPN tunnel
  - VPN IP: 192.168.2.2/32
  - DNS: 192.168.1.7 (Pi-hole)
  - AllowedIPs: 192.168.0.0/24, 192.168.2.0/24
  - Endpoint: edenredux.servegame.com:51820
  - Same configuration as Mac
- ✅ Tunnel activated as Windows service
  - Service name: WireGuardTunnel$Lambert
  - Auto-start on boot: Enabled
  - Status: Running and verified
- ✅ Verified connectivity
  - Can access 192.168.0.x network
  - Can access 192.168.2.x network
  - Tested: http://192.168.0.100:5006 - Working perfectly
  - Latency: ~60ms average

### UniFi Network Configuration
- ✅ Configured UniFi Controller to use Pi-hole for entire network
  - Controller: 192.168.1.6:8443
  - Network: Default (MILTONHAUS2)
  - DNS Server 1: 192.168.1.7 (Pi-hole)
  - Method: PowerShell API automation
  - Status: Applied and active
  - Result: All devices on network now use Pi-hole automatically

### Documentation Created
- Created comprehensive Pi-hole setup guide
  - Location (Windows): C:\Users\ericm\pihole\PIHOLE-SETUP.md
  - Location (Mac): ~/.claude/skills/project-management/pihole-setup.md
  - Includes: Windows, Mac, iOS, Android setup instructions
  - Includes: Router configuration guide
  - Includes: Troubleshooting steps
- Created WireGuard Windows setup guide
  - Location (Windows): C:\Users\ericm\pihole\wireguard-windows-setup.md
  - Location (Mac): ~/.claude/skills/project-management/wireguard-windows-setup.md
  - Includes: Installation steps
  - Includes: Configuration details
  - Includes: Verification procedures
  - Includes: Troubleshooting guide
- Printed router setup instructions
  - File: C:\Users\ericm\pihole\Router-PiHole-Setup.txt
  - Sent to: Brother HL-L8360CDW series Printer

### Network Topology Summary
**MILTONHAUS2 Network (192.168.1.x):**
- Router/USG: 192.168.1.1
- UniFi Controller/CloudKey: 192.168.1.6
- Pi-hole (Mac): 192.168.1.7
- Windows PC: 192.168.1.9

**VPN Networks (via WireGuard):**
- VPN IP (Windows): 192.168.2.2
- VPN IP (Mac): 192.168.2.2  
- Remote Network: 192.168.0.0/24
- VPN Network: 192.168.2.0/24

### Current Status
**Windows PC:**
- ✅ DNS: Using Pi-hole (192.168.1.7)
- ✅ Ad blocking: Active
- ✅ VPN: Connected (Lambert tunnel)
- ✅ Can access all networks: 192.168.1.x, 192.168.0.x, 192.168.2.x

**Network-wide:**
- ✅ All devices on MILTONHAUS2 use Pi-hole for DNS
- ✅ Kids' devices automatically get ad blocking
- ✅ No per-device configuration needed
