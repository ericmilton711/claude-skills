# WireGuard Installation on USG 3P - 2026-01-06

## Session Summary
Successfully installed WireGuard on USG 3P after researching package availability and finding the correct UGW3 package.

## Problem Statement
- USG 3P (mips64 architecture) needed WireGuard VPN to access remote network (192.168.0.x)
- Initial attempts failed due to:
  - Recent GitHub releases missing UGW3 packages
  - Wrong architecture packages (mipsel vs mips)
  - Force Provision blocked (cannot apply config.gateway.json)

## Solution Found
German Ubiquiti forum documentation pointed to the correct package release.

### Package Details
- **Release Version:** 1.0.20220627-1 (July 2, 2022)
- **Package File:** ugw3-v1-v1.0.20220627-v1.0.20210914.deb
- **Download URL:** https://github.com/WireGuard/wireguard-vyatta-ubnt/releases/download/1.0.20220627-1/ugw3-v1-v1.0.20220627-v1.0.20210914.deb
- **Package Size:** 163.1 KB
- **WireGuard Tools Version:** v1.0.20210914

## Installation Steps

### 1. Download Package
```bash
curl -sL https://github.com/WireGuard/wireguard-vyatta-ubnt/releases/download/1.0.20220627-1/ugw3-v1-v1.0.20220627-v1.0.20210914.deb -o /tmp/wg.deb
```

### 2. Install Package
```bash
sudo dpkg -i /tmp/wg.deb
```

### 3. Verification
```bash
wg version
# Output: wireguard-tools v1.0.20210914
```

## Installation Result
✓ WireGuard successfully installed on USG 3P
✓ Binary location: /usr/bin/wg
✓ Installation includes Vyatta interface diversions for proper EdgeOS integration

## Next Steps - Configuration Needed

### Required Information
To complete the WireGuard configuration, the following details are needed:

1. **Interface Configuration:**
   - Interface name (typically wg0)
   - Local IP address for WireGuard interface
   - Listen port (default: 51820)

2. **Keys:**
   - Private key for this USG
   - Public key for remote peer/server

3. **Peer Configuration:**
   - Peer public key
   - Peer endpoint (IP:port)
   - Allowed IPs (what networks to route through VPN)
   - Keepalive interval

4. **Routing:**
   - Remote networks to access (e.g., 192.168.0.0/24)
   - Whether to route all traffic or specific subnets

### Configuration Methods

**Option A: Manual Configuration (Current)**
Configure via EdgeOS CLI - settings persist until reboot/provision

**Option B: config.gateway.json (Recommended)**
Add configuration to UniFi controller's config.gateway.json file
- Survives reboots and firmware updates
- Requires Force Provision to apply
- Already uploaded to Cloud Key (needs to be applied)

## Important Notes
- Manual CLI configuration does NOT survive USG reboots or re-provisioning
- For persistent configuration, must use config.gateway.json on UniFi controller
- The config.gateway.json file was previously uploaded to Cloud Key at:
  `/usr/lib/unifi/data/sites/default/config.gateway.json`
- Force Provision step still blocked due to:
  - Cannot locate Force Provision button in web UI
  - API force-provision returns 403 Forbidden

## System Information
- **Device:** USG 3P (UniFi Security Gateway)
- **IP Address:** 192.168.1.1
- **MAC Address:** e0:63:da:c7:43:84
- **EdgeOS Version:** v4.4.57.5578372.230112.0823
- **Architecture:** mips64

## Access Credentials
- **USG SSH:** mlWKaph@192.168.1.1 (QmJ7bDN6Ed2)

## Resources
- [WireGuard EdgeOS/UniFi Gateway Wiki](https://github.com/WireGuard/wireguard-vyatta-ubnt/wiki/EdgeOS-and-Unifi-Gateway)
- [German Ubiquiti Forum - WireGuard Installation](https://ubiquiti-networks-forum.de/wiki/entry/115-wireguard-installation-auf-dem-usg/)
- [WireGuard Releases](https://github.com/WireGuard/wireguard-vyatta-ubnt/releases)
- [HostiFi EdgeRouter WireGuard Guide](https://www.hostifi.com/blog/edgerouter-wireguard-remote-access-vpn)

## WireGuard Configuration Completed

### Keys Generated
- **Private Key:** aOAQc2ihIWwdMCGoFqiON8T5967y+mv7uCl7jMEP204=
- **Public Key:** NZG/IbateRIpY/Y2Nd6j7i0uF1HjoUy0X+aZujaxXiA=
- **Key Location:** /config/auth/wg-private.key (chmod 600)

### Interface Configuration
```bash
set interfaces wireguard wg0 address 192.168.2.3/32
set interfaces wireguard wg0 listen-port 51820
set interfaces wireguard wg0 route-allowed-ips true
set interfaces wireguard wg0 private-key /config/auth/wg-private.key
set interfaces wireguard wg0 peer uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc= endpoint edenredux.servegame.com:51820
set interfaces wireguard wg0 peer uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc= allowed-ips 192.168.0.0/24
set interfaces wireguard wg0 peer uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc= allowed-ips 192.168.2.0/24
set interfaces wireguard wg0 peer uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc= persistent-keepalive 25
```

### Current Status
✓ WireGuard interface wg0 is UP
✓ Listening on port 51820
✓ Sending keepalive packets to peer (296 B sent)
✗ Not receiving data from peer (0 B received)

### Issue: Peer Configuration Required

**Problem:** The VPN server (edenredux.servegame.com) doesn't have the USG's public key configured, so it's dropping incoming packets from the USG.

**Solution:** Add the USG as a peer on the VPN server with the following configuration:

```ini
[Peer]
# USG 3P (192.168.1.1)
PublicKey = NZG/IbateRIpY/Y2Nd6j7i0uF1HjoUy0X+aZujaxXiA=
AllowedIPs = 192.168.2.3/32, 192.168.1.0/24
```

**Notes:**
- AllowedIPs should include 192.168.2.3/32 (USG's VPN IP)
- Also include 192.168.1.0/24 if you want the server to route traffic to the local network through the USG
- After adding this peer to the server, restart the WireGuard service on the server

### Network Configuration
- **USG VPN IP:** 192.168.2.3/32
- **VPN Server:** edenredux.servegame.com:51820 (174.54.51.209)
- **VPN Server Public Key:** uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
- **Allowed Networks:** 192.168.0.0/24, 192.168.2.0/24
- **Keepalive:** 25 seconds

### Verification Commands
```bash
# Check WireGuard status
sudo wg show

# Check interface
ip addr show wg0

# Test connectivity to remote network
ping 192.168.0.100

# Check routing
ip route | grep wg0
```

## VPN Server Access Challenge

### Current Situation
- VPN server (edenredux.servegame.com / 174.54.51.209) is not directly accessible via SSH
- Mac's WireGuard is running but not receiving data (0 B received, connection issue)
- Windows PC (192.168.1.9) is offline
- Server is at remote location (192.168.0.x network)

### Options to Add USG Peer

**Option 1: Use Windows PC (When Online)**
1. Turn on Windows PC (192.168.1.9)
2. Windows PC has working VPN connection to 192.168.0.x network
3. SSH from Windows to router at 192.168.0.1 (credentials: mac/645866)
4. Edit WireGuard configuration to add USG peer
5. Restart WireGuard service

**Option 2: Physical Access**
1. Go to remote location where edenredux.servegame.com is hosted
2. Access the router/server directly
3. Add USG peer configuration

**Option 3: Fix Mac's VPN Connection**
1. Mac has WireGuard configured (192.168.2.2) with same credentials as Windows
2. Mac is sending but not receiving data
3. Troubleshoot why Mac's connection isn't working
4. Once connected, access router at 192.168.0.1 to add USG peer

## Session Details
- **Date:** 2026-01-06
- **Time:** 14:00-14:30
- **Status:** ✓ WireGuard installed and configured on USG
- **Blocked:** Cannot access VPN server to add peer (no SSH access, Mac VPN not working, Windows offline)
- **Next:** User needs to either turn on Windows PC, fix Mac VPN, or physically access remote server to add USG peer configuration
