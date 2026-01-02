---
name: vpn
description: Manages VPN setup, configuration, and troubleshooting. Use when working with WireGuard, OpenVPN, IPSec, commercial VPNs, or when the user mentions VPN, tunneling, remote access, or network privacy.
allowed-tools: Read, Grep, Glob, Bash(wg:*), Bash(wg-quick:*), Bash(openvpn:*), Bash(nmcli:*), Bash(ip:*), Bash(systemctl:*), Bash(sudo:*)
---

# VPN Management Skill

This skill helps with VPN setup, configuration, troubleshooting, and management for various VPN types including WireGuard, OpenVPN, IPSec, and commercial VPN services.

## Core Responsibilities

1. **VPN Setup** - Configure WireGuard, OpenVPN, IPSec VPN connections
2. **Connection Management** - Start, stop, monitor VPN connections
3. **Troubleshooting** - Diagnose connection issues, routing problems, DNS leaks
4. **Security** - Ensure proper encryption, kill switches, DNS configuration
5. **Performance** - Optimize VPN performance and diagnose speed issues
6. **Multi-VPN** - Manage multiple VPN configurations and split tunneling

## VPN Types and Use Cases

### WireGuard (Modern, Fast, Simple)
**Best for:**
- Site-to-site connections
- Remote access VPN
- Mobile devices (low battery usage)
- Self-hosted VPN server

**Advantages:**
- Very fast (kernel-level implementation)
- Simple configuration
- Strong cryptography (ChaCha20, Curve25519)
- Small codebase (easier to audit)
- Built into Linux kernel 5.6+

### OpenVPN (Traditional, Flexible)
**Best for:**
- Legacy systems
- Complex routing scenarios
- Corporate environments
- When you need TCP mode (restrictive firewalls)

**Advantages:**
- Very mature and well-tested
- Works on all platforms
- Highly configurable
- Can run on TCP port 443 (bypass firewalls)

### IPSec/IKEv2 (Native, Enterprise)
**Best for:**
- Enterprise environments
- Native OS integration
- High security requirements
- Mobile roaming (network switching)

**Advantages:**
- Native support in most operating systems
- Fast reconnection when switching networks
- Strong security with proper configuration
- FIPS compliance available

### Commercial VPN Services
**Common providers:**
- Mullvad, ProtonVPN, IVPN (privacy-focused)
- NordVPN, ExpressVPN, Surfshark (feature-rich)

**Best for:**
- Privacy/anonymity
- Geo-restriction bypass
- Public WiFi protection
- Simple setup without self-hosting

## WireGuard Setup and Management

### Server Setup (Self-Hosted)

1. **Install WireGuard**
   ```bash
   # Fedora/RHEL
   sudo dnf install wireguard-tools

   # Debian/Ubuntu
   sudo apt install wireguard
   ```

2. **Generate Server Keys**
   ```bash
   # Create config directory
   sudo mkdir -p /etc/wireguard
   sudo chmod 700 /etc/wireguard

   # Generate server keys
   wg genkey | sudo tee /etc/wireguard/server_private.key
   sudo chmod 600 /etc/wireguard/server_private.key
   sudo cat /etc/wireguard/server_private.key | wg pubkey | sudo tee /etc/wireguard/server_public.key
   ```

3. **Create Server Configuration**
   ```bash
   sudo nano /etc/wireguard/wg0.conf
   ```

   ```ini
   [Interface]
   PrivateKey = <server_private_key>
   Address = 10.0.0.1/24
   ListenPort = 51820

   # Enable IP forwarding and NAT
   PostUp = iptables -A FORWARD -i wg0 -j ACCEPT; iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
   PostDown = iptables -D FORWARD -i wg0 -j ACCEPT; iptables -t nat -D POSTROUTING -o eth0 -j MASQUERADE

   # Client 1
   [Peer]
   PublicKey = <client1_public_key>
   AllowedIPs = 10.0.0.2/32

   # Client 2
   [Peer]
   PublicKey = <client2_public_key>
   AllowedIPs = 10.0.0.3/32
   ```

4. **Enable IP Forwarding**
   ```bash
   echo "net.ipv4.ip_forward=1" | sudo tee -a /etc/sysctl.conf
   sudo sysctl -p
   ```

5. **Start Server**
   ```bash
   sudo systemctl enable wg-quick@wg0
   sudo systemctl start wg-quick@wg0
   sudo systemctl status wg-quick@wg0
   ```

6. **Open Firewall**
   ```bash
   sudo firewall-cmd --permanent --add-port=51820/udp
   sudo firewall-cmd --reload
   ```

### Client Setup (WireGuard)

1. **Generate Client Keys**
   ```bash
   wg genkey | tee client_private.key | wg pubkey > client_public.key
   chmod 600 client_private.key
   ```

2. **Create Client Configuration**
   ```bash
   sudo nano /etc/wireguard/wg0.conf
   ```

   ```ini
   [Interface]
   PrivateKey = <client_private_key>
   Address = 10.0.0.2/24
   DNS = 1.1.1.1, 1.0.0.1

   [Peer]
   PublicKey = <server_public_key>
   Endpoint = vpn.example.com:51820
   AllowedIPs = 0.0.0.0/0, ::/0  # Route all traffic through VPN
   # AllowedIPs = 10.0.0.0/24     # Only route VPN subnet (split tunnel)
   PersistentKeepalive = 25       # Keep connection alive through NAT
   ```

3. **Connect to VPN**
   ```bash
   # One-time connection
   sudo wg-quick up wg0

   # Enable on boot
   sudo systemctl enable wg-quick@wg0
   sudo systemctl start wg-quick@wg0
   ```

4. **Check Status**
   ```bash
   sudo wg show
   sudo wg show wg0
   ```

### WireGuard Management Commands

```bash
# Start VPN
sudo wg-quick up wg0

# Stop VPN
sudo wg-quick down wg0

# Show status
sudo wg show

# Show specific interface
sudo wg show wg0

# Reload configuration (down then up)
sudo wg-quick down wg0 && sudo wg-quick up wg0

# View logs
sudo journalctl -u wg-quick@wg0 -f
```

## OpenVPN Setup and Management

### Client Setup (OpenVPN)

1. **Install OpenVPN**
   ```bash
   sudo dnf install openvpn
   ```

2. **Import Configuration** (from VPN provider or server admin)
   ```bash
   # Place .ovpn file in standard location
   sudo cp client.ovpn /etc/openvpn/client/

   # Or use NetworkManager
   nmcli connection import type openvpn file client.ovpn
   ```

3. **Connect via Command Line**
   ```bash
   sudo openvpn --config /etc/openvpn/client/client.ovpn

   # Or with auth file (username/password)
   sudo openvpn --config client.ovpn --auth-user-pass auth.txt
   ```

4. **Connect via NetworkManager**
   ```bash
   # List connections
   nmcli connection show

   # Connect
   nmcli connection up <vpn-name>

   # Disconnect
   nmcli connection down <vpn-name>
   ```

5. **Enable on Boot**
   ```bash
   sudo systemctl enable openvpn-client@client
   sudo systemctl start openvpn-client@client
   ```

### OpenVPN Server Setup (Basic)

1. **Install and Initialize**
   ```bash
   sudo dnf install openvpn easy-rsa

   # Copy Easy-RSA
   make-cadir ~/openvpn-ca
   cd ~/openvpn-ca
   ```

2. **Configure and Build CA**
   ```bash
   nano vars  # Edit settings
   ./easyrsa init-pki
   ./easyrsa build-ca
   ./easyrsa gen-dh
   ./easyrsa build-server-full server nopass
   ./easyrsa build-client-full client1 nopass
   openvpn --genkey secret ta.key
   ```

3. **Create Server Configuration**
   ```bash
   sudo nano /etc/openvpn/server/server.conf
   ```

## Commercial VPN Setup

### Mullvad VPN (Privacy-Focused)

```bash
# Install Mullvad app
sudo dnf install mullvad-vpn

# Or use WireGuard configuration
# Download config from Mullvad website
sudo wg-quick up mullvad-<location>
```

### ProtonVPN

```bash
# Install ProtonVPN CLI
sudo dnf install protonvpn-cli

# Login
protonvpn-cli login

# Connect to fastest server
protonvpn-cli connect --fastest

# Connect to specific country
protonvpn-cli connect --cc US

# Disconnect
protonvpn-cli disconnect
```

## VPN Troubleshooting

### Connection Issues

**VPN won't connect:**
```bash
# Check if VPN service is running
sudo systemctl status wg-quick@wg0
sudo systemctl status openvpn-client@client

# Check interface exists
ip addr show wg0

# Check routing table
ip route show

# Check firewall
sudo firewall-cmd --list-all

# Check logs
sudo journalctl -u wg-quick@wg0 -n 50
sudo journalctl -u openvpn-client@client -n 50

# Test server reachability
ping <vpn-server-ip>
nc -zvu <vpn-server-ip> 51820  # WireGuard
nc -zv <vpn-server-ip> 1194    # OpenVPN
```

**Connection drops frequently:**
```bash
# For WireGuard, add PersistentKeepalive
# In [Peer] section:
PersistentKeepalive = 25

# Check for DNS issues
resolvectl status

# Monitor connection
watch -n 1 'sudo wg show'
```

### DNS Leaks

**Check for DNS leaks:**
```bash
# Check current DNS servers
resolvectl status

# Test DNS (should show VPN DNS, not ISP)
dig whoami.akamai.net +short

# Online leak test
curl -s https://www.dnsleaktest.com/
```

**Fix DNS leaks:**
```bash
# WireGuard: Add to [Interface]
DNS = 1.1.1.1, 1.0.0.1

# OpenVPN: Add to .ovpn file
dhcp-option DNS 1.1.1.1
dhcp-option DNS 1.0.0.1

# Or use systemd-resolved
sudo resolvectl dns wg0 1.1.1.1 1.0.0.1
```

### IP Leak / Kill Switch

**Verify IP address:**
```bash
# Check your public IP (should be VPN IP)
curl ifconfig.me
curl ipinfo.io

# Compare with VPN disconnected
```

**Implement kill switch (WireGuard):**
```bash
# In /etc/wireguard/wg0.conf [Interface]:
PostUp = iptables -I OUTPUT ! -o %i -m mark ! --mark $(wg show %i fwmark) -m addrtype ! --dst-type LOCAL -j REJECT
PreDown = iptables -D OUTPUT ! -o %i -m mark ! --mark $(wg show %i fwmark) -m addrtype ! --dst-type LOCAL -j REJECT
```

**Implement kill switch (OpenVPN):**
```bash
# Add to .ovpn file:
pull-filter ignore redirect-gateway
route-nopull

# Or use ufw
sudo ufw default deny outgoing
sudo ufw allow out on tun0
sudo ufw allow out to <vpn-server-ip> port 1194 proto udp
```

### Performance Issues

**Test VPN speed:**
```bash
# Without VPN
speedtest-cli

# With VPN
sudo wg-quick up wg0
speedtest-cli
```

**Optimize WireGuard:**
```bash
# Change MTU if needed (in [Interface])
MTU = 1420  # Try 1380 or 1280 if issues

# Check current MTU
ip link show wg0
```

**Optimize OpenVPN:**
```bash
# In .ovpn file:
sndbuf 524288
rcvbuf 524288
push "sndbuf 524288"
push "rcvbuf 524288"

# Use UDP instead of TCP if possible
proto udp

# Enable compression (if supported)
compress lz4-v2
```

### Split Tunneling

**Route only specific traffic through VPN:**

```bash
# WireGuard: Modify AllowedIPs
# Only route specific subnet through VPN
AllowedIPs = 192.168.100.0/24

# Or specific IPs
AllowedIPs = 192.168.100.5/32, 192.168.100.10/32
```

**Route specific applications:**
```bash
# Use network namespaces
sudo ip netns add vpn
sudo ip netns exec vpn wg-quick up wg0
sudo ip netns exec vpn sudo -u $USER firefox
```

## Security Best Practices

### Credential Management
- Store VPN configs in encrypted format (GPG)
- Use password manager for VPN credentials
- Never commit `.ovpn` files or WireGuard keys to git
- Use `chmod 600` on all VPN configuration files

### Configuration Security
```bash
# Secure WireGuard configs
sudo chmod 600 /etc/wireguard/*.conf
sudo chown root:root /etc/wireguard/*.conf

# Secure OpenVPN configs
sudo chmod 600 /etc/openvpn/client/*.ovpn
sudo chmod 600 /etc/openvpn/client/*.key
```

### Regular Audits
- Verify no DNS leaks monthly
- Check for IP leaks (kill switch working)
- Update VPN client software regularly
- Rotate WireGuard keys every 6-12 months
- Monitor VPN logs for suspicious activity

## Common VPN Configurations

### Site-to-Site VPN (WireGuard)

**Site A (10.0.1.0/24) ↔ Site B (10.0.2.0/24):**

Site A Configuration:
```ini
[Interface]
PrivateKey = <site_a_private_key>
Address = 10.254.0.1/30
ListenPort = 51820

PostUp = iptables -A FORWARD -i wg0 -j ACCEPT; iptables -A FORWARD -o wg0 -j ACCEPT
PostDown = iptables -D FORWARD -i wg0 -j ACCEPT; iptables -D FORWARD -o wg0 -j ACCEPT

[Peer]
PublicKey = <site_b_public_key>
Endpoint = site-b.example.com:51820
AllowedIPs = 10.254.0.2/32, 10.0.2.0/24
PersistentKeepalive = 25
```

Site B Configuration:
```ini
[Interface]
PrivateKey = <site_b_private_key>
Address = 10.254.0.2/30
ListenPort = 51820

PostUp = iptables -A FORWARD -i wg0 -j ACCEPT; iptables -A FORWARD -o wg0 -j ACCEPT
PostDown = iptables -D FORWARD -i wg0 -j ACCEPT; iptables -D FORWARD -o wg0 -j ACCEPT

[Peer]
PublicKey = <site_a_public_key>
Endpoint = site-a.example.com:51820
AllowedIPs = 10.254.0.1/32, 10.0.1.0/24
PersistentKeepalive = 25
```

### Road Warrior VPN (Remote Access)

Multiple clients connecting to central server for remote access.

### VPN Chaining / Multi-Hop

```bash
# Connect to VPN 1, then VPN 2 through VPN 1
sudo wg-quick up wg-vpn1
sudo wg-quick up wg-vpn2
```

## Monitoring and Logging

### WireGuard Monitoring
```bash
# Real-time status
watch -n 1 'sudo wg show'

# Traffic statistics
sudo wg show wg0 transfer

# Latest handshake
sudo wg show wg0 latest-handshakes

# Endpoints
sudo wg show wg0 endpoints
```

### OpenVPN Monitoring
```bash
# Watch logs
sudo journalctl -u openvpn-client@client -f

# Connection status via management interface
echo "status" | nc localhost 7505
```

### Connection Health Script
```bash
#!/bin/bash
# Check VPN health

# Check if VPN is up
if ip link show wg0 &>/dev/null; then
    echo "✓ VPN interface up"
else
    echo "✗ VPN interface down"
    exit 1
fi

# Check IP
PUBLIC_IP=$(curl -s ifconfig.me)
echo "Public IP: $PUBLIC_IP"

# Check DNS
DNS_SERVERS=$(resolvectl status wg0 | grep "DNS Servers")
echo "$DNS_SERVERS"

# Check for leaks
echo "Testing DNS leak..."
dig whoami.akamai.net +short
```

## Quick Reference

### WireGuard Cheat Sheet
```bash
# Start: sudo wg-quick up wg0
# Stop: sudo wg-quick down wg0
# Status: sudo wg show
# Logs: sudo journalctl -u wg-quick@wg0 -f
```

### OpenVPN Cheat Sheet
```bash
# Start: sudo systemctl start openvpn-client@client
# Stop: sudo systemctl stop openvpn-client@client
# Status: sudo systemctl status openvpn-client@client
# Logs: sudo journalctl -u openvpn-client@client -f
```

### NetworkManager VPN
```bash
# List: nmcli connection show
# Connect: nmcli connection up <vpn-name>
# Disconnect: nmcli connection down <vpn-name>
# Delete: nmcli connection delete <vpn-name>
```

## Success Criteria

A properly configured VPN has:
- ✅ Reliable connection with automatic reconnection
- ✅ No DNS leaks (verified)
- ✅ No IP leaks (verified)
- ✅ Kill switch enabled (traffic blocked if VPN drops)
- ✅ Proper routing (all or split tunnel as intended)
- ✅ Good performance (minimal speed impact)
- ✅ Secure credential storage
- ✅ Automatic connection on boot (if desired)
- ✅ Proper firewall configuration
- ✅ Regular monitoring and updates
