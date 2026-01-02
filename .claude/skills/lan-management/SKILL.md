---
name: lan-management
description: Manages local area networks including network discovery, DHCP, DNS, routing, firewall configuration, and troubleshooting. Use when working with network setup, IP addresses, subnets, routers, switches, access points, or when the user mentions LAN, network configuration, DHCP, DNS, routing, or network issues.
allowed-tools: Read, Write, Edit, Glob, Grep, Bash(ip:*), Bash(nmcli:*), Bash(nmap:*), Bash(ping:*), Bash(traceroute:*), Bash(dig:*), Bash(arp:*), Bash(ss:*), Bash(netstat:*), Bash(firewall-cmd:*), Bash(sudo:*), Bash(systemctl:*)
---

# LAN Management Skill

This skill helps you configure, manage, monitor, and troubleshoot local area networks (LANs).

## Core Responsibilities

1. **Network Discovery** - Scan and inventory network devices
2. **IP Management** - Configure static IPs, DHCP, subnets
3. **DNS Configuration** - Set up local DNS, resolve issues
4. **Routing** - Configure routes, gateways, multi-network setups
5. **Firewall Management** - Configure firewall rules, port forwarding
6. **Network Monitoring** - Monitor bandwidth, connections, performance
7. **Troubleshooting** - Diagnose and fix network issues
8. **WiFi Management** - Configure access points, channels, security

## Network Architecture Overview

### Typical Home/Small Office Network

```
Internet
   ↓
[Modem] (ISP connection)
   ↓
[Router/Gateway] (192.168.1.1)
   ↓
[Switch] (optional, for more ports)
   ↓
├── [WiFi Access Point(s)]
├── [Computers] (192.168.1.100-199)
├── [Servers] (192.168.1.10-50)
├── [IoT Devices] (192.168.1.200-254)
└── [Printers/NAS] (192.168.1.51-99)
```

### Common IP Ranges

- **Class A Private**: 10.0.0.0/8 (10.0.0.0 - 10.255.255.255)
- **Class B Private**: 172.16.0.0/12 (172.16.0.0 - 172.31.255.255)
- **Class C Private**: 192.168.0.0/16 (192.168.0.0 - 192.168.255.255)
- **Link-Local**: 169.254.0.0/16 (auto-assigned when DHCP fails)
- **Loopback**: 127.0.0.0/8 (localhost)

### Recommended IP Allocation

```
192.168.1.0/24 subnet example:

.1          - Router/Gateway
.2-.9       - Reserved for infrastructure (switches, APs)
.10-.50     - Static servers (NAS, media server, Pi-hole)
.51-.99     - Static devices (printers, cameras, smart home hub)
.100-.199   - DHCP pool for workstations
.200-.254   - DHCP pool for guests/IoT devices
```

## Network Discovery and Scanning

### Discover Devices on Network

```bash
# Method 1: Using nmap (most comprehensive)
sudo nmap -sn 192.168.1.0/24

# Method 2: Using arp-scan
sudo arp-scan --localnet

# Method 3: Using avahi (mDNS/Bonjour discovery)
avahi-browse -art

# Method 4: Check ARP cache
ip neigh show
arp -a

# Method 5: Ping sweep (basic)
for i in {1..254}; do ping -c 1 -W 1 192.168.1.$i >/dev/null && echo "192.168.1.$i is up"; done
```

### Detailed Device Scan

```bash
# Full port scan on specific host
sudo nmap -A 192.168.1.100

# Quick scan for common ports
sudo nmap -F 192.168.1.100

# Service version detection
sudo nmap -sV 192.168.1.100

# Operating system detection
sudo nmap -O 192.168.1.100

# Scan specific ports
sudo nmap -p 22,80,443,3389 192.168.1.100
```

### Find Specific Devices

```bash
# Find printers (port 9100, 631)
sudo nmap -p 9100,631 192.168.1.0/24

# Find web servers
sudo nmap -p 80,443,8080 192.168.1.0/24

# Find SSH servers
sudo nmap -p 22 192.168.1.0/24

# Find Windows machines (SMB)
sudo nmap -p 445 192.168.1.0/24

# Find cameras (common RTSP/ONVIF ports)
sudo nmap -p 554,8000,8080 192.168.1.0/24
```

## IP Address Configuration

### Check Current Network Configuration

```bash
# Show all interfaces
ip addr show
ip a

# Show specific interface
ip addr show eth0
ip addr show wlan0

# Show routing table
ip route show
route -n

# Show DNS servers
cat /etc/resolv.conf
resolvectl status
```

### Configure Static IP (NetworkManager)

```bash
# Method 1: Using nmcli
nmcli connection show  # List connections

# Set static IP
nmcli connection modify "Wired connection 1" \
  ipv4.method manual \
  ipv4.addresses 192.168.1.100/24 \
  ipv4.gateway 192.168.1.1 \
  ipv4.dns "1.1.1.1,1.0.0.1"

# Apply changes
nmcli connection down "Wired connection 1"
nmcli connection up "Wired connection 1"

# Verify
ip addr show
```

### Configure Static IP (Manual - systemd-networkd)

```bash
# Create config file
sudo nano /etc/systemd/network/20-wired.network
```

```ini
[Match]
Name=eth0

[Network]
Address=192.168.1.100/24
Gateway=192.168.1.1
DNS=1.1.1.1
DNS=1.0.0.1
```

```bash
# Enable and restart
sudo systemctl enable systemd-networkd
sudo systemctl restart systemd-networkd
```

### Configure DHCP (NetworkManager)

```bash
# Set to DHCP
nmcli connection modify "Wired connection 1" \
  ipv4.method auto

nmcli connection down "Wired connection 1"
nmcli connection up "Wired connection 1"
```

### Reserve DHCP Address (Router Configuration)

Most routers allow DHCP reservations in their web interface:
1. Find device MAC address: `ip link show` or `arp -a`
2. Log into router admin panel (usually http://192.168.1.1)
3. Navigate to DHCP settings
4. Add reservation: MAC address → specific IP
5. Save and reboot router if needed

## DNS Configuration

### Change DNS Servers

```bash
# Method 1: NetworkManager
nmcli connection modify "Wired connection 1" \
  ipv4.dns "1.1.1.1,1.0.0.1"

nmcli connection up "Wired connection 1"

# Method 2: Edit resolv.conf (temporary - gets overwritten)
sudo nano /etc/resolv.conf
# Add:
# nameserver 1.1.1.1
# nameserver 1.0.0.1

# Method 3: systemd-resolved
sudo nano /etc/systemd/resolved.conf
# [Resolve]
# DNS=1.1.1.1 1.0.0.1
sudo systemctl restart systemd-resolved
```

### Popular DNS Servers

```
Cloudflare:     1.1.1.1, 1.0.0.1
Google:         8.8.8.8, 8.8.4.4
Quad9:          9.9.9.9, 149.112.112.112
OpenDNS:        208.67.222.222, 208.67.220.220
```

### Local DNS (Pi-hole / hosts file)

```bash
# Edit hosts file for local domain names
sudo nano /etc/hosts

# Add entries:
192.168.1.10    nas.local
192.168.1.20    server.local
192.168.1.30    printer.local
```

### DNS Troubleshooting

```bash
# Test DNS resolution
dig google.com
nslookup google.com
host google.com

# Show which DNS server answered
dig google.com +short

# Reverse DNS lookup
dig -x 192.168.1.1

# Check DNS configuration
resolvectl status

# Flush DNS cache (systemd-resolved)
sudo resolvectl flush-caches

# Test specific DNS server
dig @1.1.1.1 google.com
```

## Routing Configuration

### View Routing Table

```bash
# Show routes
ip route show
route -n

# Show routes for specific interface
ip route show dev eth0
```

### Add Static Route

```bash
# Add route to specific network
sudo ip route add 10.0.0.0/24 via 192.168.1.254

# Add default gateway
sudo ip route add default via 192.168.1.1

# Make persistent (NetworkManager)
nmcli connection modify "Wired connection 1" \
  +ipv4.routes "10.0.0.0/24 192.168.1.254"
```

### Delete Route

```bash
# Delete specific route
sudo ip route del 10.0.0.0/24 via 192.168.1.254

# Delete default gateway
sudo ip route del default via 192.168.1.1
```

### Multi-Gateway Setup (Policy Routing)

```bash
# Example: Route specific traffic through different gateways
# All traffic to 10.x.x.x goes through 192.168.1.254
sudo ip route add 10.0.0.0/8 via 192.168.1.254

# Default traffic goes through 192.168.1.1
sudo ip route add default via 192.168.1.1
```

## Firewall Configuration (firewalld)

### Check Firewall Status

```bash
# Check if firewalld is running
sudo systemctl status firewalld

# Show active zones
sudo firewall-cmd --get-active-zones

# Show all rules in default zone
sudo firewall-cmd --list-all

# Show rules in specific zone
sudo firewall-cmd --zone=public --list-all
```

### Open Ports

```bash
# Open single port
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload

# Open port range
sudo firewall-cmd --permanent --add-port=8000-8100/tcp
sudo firewall-cmd --reload

# Open service (predefined rules)
sudo firewall-cmd --permanent --add-service=http
sudo firewall-cmd --permanent --add-service=https
sudo firewall-cmd --permanent --add-service=ssh
sudo firewall-cmd --reload

# List available services
sudo firewall-cmd --get-services
```

### Close Ports

```bash
# Close port
sudo firewall-cmd --permanent --remove-port=8080/tcp
sudo firewall-cmd --reload

# Remove service
sudo firewall-cmd --permanent --remove-service=http
sudo firewall-cmd --reload
```

### Port Forwarding

```bash
# Forward external port to internal host
# Example: Forward port 8080 to 192.168.1.100:80

# Enable masquerading (NAT)
sudo firewall-cmd --permanent --zone=public --add-masquerade

# Add port forward
sudo firewall-cmd --permanent --zone=public \
  --add-forward-port=port=8080:proto=tcp:toaddr=192.168.1.100:toport=80

sudo firewall-cmd --reload

# Verify
sudo firewall-cmd --zone=public --list-forward-ports
```

### Allow Traffic from Specific IP/Subnet

```bash
# Create rich rule to allow from specific source
sudo firewall-cmd --permanent --zone=public \
  --add-rich-rule='rule family="ipv4" source address="192.168.1.0/24" accept'

# Allow specific IP to specific port
sudo firewall-cmd --permanent --zone=public \
  --add-rich-rule='rule family="ipv4" source address="192.168.1.50" port port="22" protocol="tcp" accept'

sudo firewall-cmd --reload
```

### Block Traffic

```bash
# Block specific IP
sudo firewall-cmd --permanent --zone=public \
  --add-rich-rule='rule family="ipv4" source address="192.168.1.100" reject'

# Block specific IP range
sudo firewall-cmd --permanent --zone=public \
  --add-rich-rule='rule family="ipv4" source address="192.168.1.200/29" reject'

sudo firewall-cmd --reload
```

## Network Monitoring

### Monitor Active Connections

```bash
# Show all listening ports
sudo ss -tulpn
sudo netstat -tulpn

# Show established connections
ss -tn
netstat -tn

# Show connections for specific port
ss -tnp | grep :80

# Watch connections in real-time
watch -n 1 'ss -s'
```

### Monitor Bandwidth Usage

```bash
# Using iftop (install if needed: sudo dnf install iftop)
sudo iftop -i eth0

# Using nethogs (per-process bandwidth)
sudo nethogs eth0

# Using nload (simple bandwidth monitor)
nload eth0

# Using vnstat (long-term statistics)
vnstat -i eth0
vnstat -h  # hourly stats
vnstat -d  # daily stats
```

### Monitor Network Traffic (tcpdump)

```bash
# Capture all traffic on interface
sudo tcpdump -i eth0

# Capture specific port
sudo tcpdump -i eth0 port 80

# Capture to file
sudo tcpdump -i eth0 -w capture.pcap

# Read from file
sudo tcpdump -r capture.pcap

# Filter by host
sudo tcpdump -i eth0 host 192.168.1.100

# HTTP traffic
sudo tcpdump -i eth0 -A 'tcp port 80'
```

### Check Interface Statistics

```bash
# Show interface statistics
ip -s link show eth0

# Detailed stats
cat /proc/net/dev

# Watch in real-time
watch -n 1 'ip -s link show eth0'
```

## WiFi Management

### Scan WiFi Networks

```bash
# Scan for networks
sudo nmcli device wifi list

# Rescan
sudo nmcli device wifi rescan

# Connect to WiFi
nmcli device wifi connect "SSID" password "password"

# Show saved WiFi networks
nmcli connection show
```

### WiFi Configuration

```bash
# Show current WiFi connection
nmcli connection show --active

# Disconnect WiFi
nmcli device disconnect wlan0

# Connect to saved network
nmcli connection up "WiFi-Name"

# Forget network
nmcli connection delete "WiFi-Name"
```

### WiFi Access Point Setup

```bash
# Create WiFi hotspot (requires WiFi adapter support)
nmcli device wifi hotspot ssid "MyHotspot" password "mypassword"

# Stop hotspot
nmcli connection down Hotspot
```

### WiFi Channel Analysis

```bash
# Install wavemon
sudo dnf install wavemon

# Run wavemon (ncurses WiFi monitor)
sudo wavemon

# Best channels (less congestion):
# 2.4GHz: 1, 6, 11 (non-overlapping)
# 5GHz: Many non-overlapping channels available
```

## Network Troubleshooting

### Connection Issues

**Check physical connection:**
```bash
# Check if interface is up
ip link show

# Bring interface up
sudo ip link set eth0 up

# Check cable connection (if supported)
ethtool eth0 | grep "Link detected"
```

**Check IP configuration:**
```bash
# Verify IP address assigned
ip addr show

# Check if DHCP working
sudo dhclient -v eth0

# Release and renew DHCP
sudo dhclient -r eth0  # release
sudo dhclient eth0     # renew
```

**Check gateway reachability:**
```bash
# Ping gateway
ping -c 4 192.168.1.1

# Ping external IP (tests internet without DNS)
ping -c 4 8.8.8.8

# Ping domain (tests DNS + internet)
ping -c 4 google.com
```

### DNS Issues

```bash
# Test DNS resolution
dig google.com
nslookup google.com

# Try different DNS server
dig @1.1.1.1 google.com

# Check resolv.conf
cat /etc/resolv.conf

# Flush DNS cache
sudo resolvectl flush-caches
```

### Routing Issues

```bash
# Check routing table
ip route show

# Trace route to destination
traceroute google.com
traceroute 192.168.1.1

# Check if packet forwarding enabled (for routers)
cat /proc/sys/net/ipv4/ip_forward
# Should be 1 for router, 0 for endpoint
```

### Network Performance Issues

**Test bandwidth:**
```bash
# Using iperf3 (requires server on other end)
# Server side:
iperf3 -s

# Client side:
iperf3 -c 192.168.1.100

# Test LAN speed
iperf3 -c 192.168.1.100 -t 30  # 30 second test
```

**Check for packet loss:**
```bash
# Ping with statistics
ping -c 100 192.168.1.1

# MTR (combines ping + traceroute)
mtr google.com
```

**Check MTU issues:**
```bash
# Test different MTU sizes
ping -M do -s 1472 google.com  # 1500 MTU
ping -M do -s 1400 google.com  # 1428 MTU

# Set MTU
sudo ip link set eth0 mtu 1400
```

### Duplicate IP Detection

```bash
# Check for IP conflicts
sudo arping -D -I eth0 192.168.1.100

# Scan network for duplicates
sudo arp-scan --localnet | sort
```

### Port Connectivity Testing

```bash
# Test if port is open
nc -zv 192.168.1.100 22

# Test from remote machine
telnet 192.168.1.100 22

# Check local listening ports
sudo ss -tulpn | grep :22
```

## VLAN Configuration

### Create VLAN Interface

```bash
# Create VLAN 10 on eth0
sudo nmcli connection add type vlan \
  con-name vlan10 \
  ifname eth0.10 \
  dev eth0 \
  id 10 \
  ipv4.method manual \
  ipv4.addresses 192.168.10.1/24

# Bring up VLAN
nmcli connection up vlan10

# Verify
ip addr show eth0.10
```

### Delete VLAN

```bash
nmcli connection delete vlan10
```

## Network Bridges

### Create Network Bridge

```bash
# Create bridge
sudo nmcli connection add type bridge \
  con-name br0 \
  ifname br0

# Add interface to bridge
sudo nmcli connection add type ethernet \
  slave-type bridge \
  con-name bridge-eth0 \
  ifname eth0 \
  master br0

# Configure bridge IP
sudo nmcli connection modify br0 \
  ipv4.method manual \
  ipv4.addresses 192.168.1.100/24 \
  ipv4.gateway 192.168.1.1

# Bring up
sudo nmcli connection up br0
```

## Network Documentation Template

Create `network-topology.md`:

```markdown
# Network Topology

## Network Overview
- **ISP**: Comcast
- **Public IP**: X.X.X.X (DHCP from ISP)
- **Internal Network**: 192.168.1.0/24
- **Gateway**: 192.168.1.1

## Network Diagram
```
Internet → Modem → Router (192.168.1.1)
                    ├── Switch (192.168.1.2)
                    ├── WiFi AP (192.168.1.3)
                    └── Devices
```

## Device Inventory

| Hostname | IP Address | MAC Address | Type | Notes |
|----------|------------|-------------|------|-------|
| router | 192.168.1.1 | AA:BB:CC:DD:EE:01 | Router | Main gateway |
| switch | 192.168.1.2 | AA:BB:CC:DD:EE:02 | Switch | 24-port |
| ap-main | 192.168.1.3 | AA:BB:CC:DD:EE:03 | Access Point | 5GHz/2.4GHz |
| nas | 192.168.1.10 | AA:BB:CC:DD:EE:04 | NAS | Synology |
| server | 192.168.1.20 | AA:BB:CC:DD:EE:05 | Server | Ubuntu 24.04 |
| printer | 192.168.1.51 | AA:BB:CC:DD:EE:06 | Printer | HP LaserJet |
| desktop | 192.168.1.100 | AA:BB:CC:DD:EE:07 | Workstation | DHCP reserved |

## DHCP Configuration
- **Range**: 192.168.1.100 - 192.168.1.199
- **Lease Time**: 24 hours
- **DNS**: 1.1.1.1, 1.0.0.1
- **Gateway**: 192.168.1.1

## WiFi Networks
- **Main SSID**: HomeNetwork
  - Security: WPA3-Personal
  - Frequency: 5GHz + 2.4GHz
  - Channel 2.4GHz: 6
  - Channel 5GHz: 36

- **Guest SSID**: HomeNetwork-Guest
  - Security: WPA2-Personal
  - Isolated from main network
  - Bandwidth limit: 10 Mbps

## Port Forwarding Rules
| External Port | Internal IP | Internal Port | Protocol | Service |
|---------------|-------------|---------------|----------|---------|
| 22 | 192.168.1.20 | 22 | TCP | SSH |
| 443 | 192.168.1.20 | 443 | TCP | HTTPS |

## Firewall Rules
- Default: DROP all incoming
- Allow: SSH from 192.168.1.0/24
- Allow: HTTP/HTTPS from anywhere
- Allow: Established connections

## DNS Records (Local)
- nas.local → 192.168.1.10
- server.local → 192.168.1.20
- printer.local → 192.168.1.51
```

## Quick Reference Commands

### Discovery
```bash
sudo nmap -sn 192.168.1.0/24      # Scan network
ip neigh show                      # Show ARP cache
avahi-browse -art                  # mDNS discovery
```

### Configuration
```bash
ip addr show                       # Show IPs
ip route show                      # Show routes
nmcli connection show              # Show connections
sudo firewall-cmd --list-all      # Show firewall rules
```

### Troubleshooting
```bash
ping -c 4 192.168.1.1             # Test gateway
traceroute google.com              # Trace route
dig google.com                     # Test DNS
sudo ss -tulpn                     # Show listening ports
```

### Monitoring
```bash
sudo iftop -i eth0                # Bandwidth monitor
sudo nethogs eth0                  # Per-process bandwidth
sudo tcpdump -i eth0 port 80      # Capture traffic
```

## Success Criteria

A well-managed LAN has:
- ✅ All devices have static IPs or DHCP reservations
- ✅ Network topology documented
- ✅ DNS configured correctly (local and external)
- ✅ Firewall rules documented and minimal
- ✅ Regular network scans for unauthorized devices
- ✅ WiFi on optimal channels with WPA3 security
- ✅ Port forwarding rules documented
- ✅ Monitoring in place for critical services
- ✅ Backup router configuration
- ✅ Network diagram up to date
