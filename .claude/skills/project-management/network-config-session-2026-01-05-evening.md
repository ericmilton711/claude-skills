# Network Configuration Session - USG Force Provision (2026-01-05 Evening)

## Session Summary
Continuation of network configuration to apply WireGuard VPN and Pi-hole DNS settings to UniFi Security Gateway.

## Completed Tasks
- Config file uploaded to Cloud Key via WinSCP
- UniFi interface explored for Force Provision location
- USG came back online successfully (ping confirmed)
- UniFi API authentication method identified (requires 2FA)
- Successfully logged into UniFi API with 2FA code
- Tested connectivity to 192.168.0.100:5006 (timed out - expected without VPN)
- Attempted manual WireGuard installation on USG
- Pi-hole YouTube blocking configured:
  - Added youtubei.googleapis.com to denylist
  - Added wildcard blocks for i.ytimg.com, s.ytimg.com, youtu.be, googlevideo.com
  - DNS reloaded to apply changes
  - Verified blocking with pihole query command
- DHCP DNS server updated on USG:
  - Changed from 192.168.1.1 (USG) to 192.168.1.7 (Pi-hole)
  - Removed old DNS server to prevent bypass
  - All network devices will now use Pi-hole for DNS queries
  - Configuration saved on USG (will persist until next provision)

## Current Status (Updated 2026-01-06)
- Force Provision USG: **BLOCKED** - Cannot find Force Provision button in web UI, API returns 403 Forbidden
- WireGuard Installation: **COMPLETED** ✓ (See wireguard-usg-installation-2026-01-06.md)
- WireGuard Configuration: **COMPLETED** ✓
- WireGuard Server Peer: **PENDING** - Server needs USG public key added
- Configure Pi-hole to block YouTube: **COMPLETED** ✓
- Update DHCP to use Pi-hole: **COMPLETED** ✓

## Important Notes
**YouTube Blocking is Now Active!**
- All devices on the network will get Pi-hole (192.168.1.7) as their DNS server
- Existing devices may need to:
  - Renew DHCP lease (disconnect/reconnect WiFi, or restart device)
  - Or wait for lease to expire and renew automatically
- To test: Try accessing youtube.com from another device - it should be blocked
- **WARNING**: This DNS change was made directly on the USG and will be overwritten if you ever force provision the USG from the UniFi controller

## Network Configuration
- UniFi Cloud Key: 192.168.1.6
- USG 3P: 192.168.1.1 (MAC: e0:63:da:c7:43:84)
- Pi-hole/Mac: 192.168.1.7
- Config file: /usr/lib/unifi/data/sites/default/config.gateway.json

## Technical Details Discovered

### UniFi API Authentication
- API endpoint: https://192.168.1.6/api/auth/login
- Requires 2FA token in addition to username/password
- Successfully authenticated with 2FA code 873168
- Force provision endpoint returns 403 Forbidden (permission issue)

### WireGuard Installation Challenges
- USG 3P runs EdgeOS v4.4.57 on mips64 architecture
- WireGuard packages tried:
  - e50-v2 package: Wrong architecture (mipsel vs mips)
  - ugw3 packages: Not found in releases (404 errors)
  - e300 packages: Not found in releases (404 errors)
- GitHub releases checked: wireguard-vyatta-ubnt (no USG packages in recent releases)
- Manual installation from source would be required

### Network Status
- USG responds to ping at 192.168.1.1
- WireGuard not installed on USG (wg command not found)
- 192.168.0.100:5006 unreachable (expected - requires VPN tunnel)

## Next Steps
1. **Force Provision Options:**
   - Try UniFi Cloud Access at https://unifi.ui.com
   - Manual web UI navigation with detailed screenshots
   - Alternative: SSH to Cloud Key and trigger provision via CLI

2. **WireGuard Installation Options:**
   - Compile WireGuard from source for mips64
   - Find older release with UGW3 packages
   - Use alternative VPN solution

3. **Pi-hole Configuration:**
   - Get sudo password for local mac user
   - Add YouTube domains to blocklist
   - Verify DNS blocking is working

## Access Credentials
- UniFi: https://192.168.1.6 (ericmilton711@gmail.com / PASSword!?1711)
- Cloud Key SSH: root@192.168.1.6 (PASSword!?1711)
- USG SSH: mlWKaph@192.168.1.1 (QmJ7bDN6Ed2)
- Local Mac sudo: mac user (645866)

## Session Notes
- UniFi controller has 2FA enabled, which complicated API automation
- USG 3P is legacy hardware (mips64) with limited community support
- WireGuard packages for USG discontinued in recent releases
- Force Provision button location unclear in current UniFi UI
- Explored UniFi Cloud Access as alternative approach
- Pi-hole blocking YouTube is independent of VPN setup and can proceed

## Resources
- WireGuard for Vyatta/EdgeOS: https://github.com/WireGuard/wireguard-vyatta-ubnt/releases
- UniFi Cloud Access: https://unifi.ui.com
- Community guides referenced for WireGuard on USG

## Summary

### Session 1 (2026-01-05 Evening)
**Successfully completed:** Pi-hole configuration to block YouTube network-wide
- Pi-hole blocklist updated with YouTube domains
- USG DHCP server configured to use Pi-hole as DNS
- All network devices will now have YouTube blocked via DNS

**Blocked:** WireGuard VPN installation
- Cannot trigger Force Provision via web UI or API
- Pre-built WireGuard packages unavailable for USG 3P (mips64)

### Session 2 (2026-01-06 Morning)
**Successfully completed:** WireGuard VPN installation and configuration
- Found correct UGW3 package (1.0.20220627-1 release)
- Installed WireGuard v1.0.20210914 on USG 3P
- Generated keypair for USG
- Configured wg0 interface with VPN server peer
- Interface active and sending keepalive packets

**Pending:** VPN server peer configuration
- Server (edenredux.servegame.com) needs USG public key added
- Public key to add: NZG/IbateRIpY/Y2Nd6j7i0uF1HjoUy0X+aZujaxXiA=
- See wireguard-usg-installation-2026-01-06.md for details

Session Date: 2026-01-05 Evening (started), 2026-01-06 Morning (continued)
Total Duration: ~1 hour 15 minutes
Last Updated: 2026-01-06 14:20
