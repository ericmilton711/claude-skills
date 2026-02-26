# MILTONHAUS Network Rebuild Plan

**Last Updated:** 2026-02-25
**Status:** Planning — Pi 4 arriving, Lenovo ThinkCentre M900 Tiny offer pending ($85)

---

## Goals

1. Parental controls — kids cannot access YouTube or unapproved sites
2. Adults (Eric + spouse) have unrestricted internet access
3. Kids can access homeschool platform at `192.168.0.100:5006` (Lambert network)
4. Clean, maintainable setup with dedicated devices for dedicated roles

---

## Hardware

| Device | Role | Status |
|---|---|---|
| **USG 3P** | Router + VLAN enforcement + firewall | Active — 192.168.1.1 |
| **Pi 4** (ethernet) | Pi-hole (kids DNS) + WireGuard VPN gateway | Arriving soon |
| **Lenovo ThinkCentre M900 Tiny** | Fedora Server — UniFi controller | Offer pending ($85, eBay item 226876134049) |
| **Cloud Key** | Retired (replaced by Lenovo) | Retire after migration |
| **Pi 3 A+** | Repurposed for other projects | Freed up |
| **Windows PC** | Client + management (SSH to Lambert) | 192.168.1.9 |
| **Mac** | Client only | 192.168.1.7 |

**Lenovo ThinkCentre M900 Tiny specs:** Intel i5-6500T, 8GB RAM, 256GB SSD, Windows 11
**Why chosen:** ~10-15W idle (very low power for 24/7 use), x86, runs Fedora Server, plenty for UniFi controller

---

## Network Architecture

### Two VLANs

| Network | SSID | Users | DNS | Internet |
|---|---|---|---|---|
| **VLAN 1** | MILTONHAUS_ADMIN | Eric, Spouse | Unrestricted | Full |
| **VLAN 10** | MILTONHAUS | Kids | Pi 4 Pi-hole (whitelist) | Restricted |

### IP Scheme

| Device | IP | VLAN |
|---|---|---|
| USG 3P | 192.168.1.1 | 1 (Management) |
| Pi 4 | 192.168.1.x (static) | 1 (serves kids VLAN) |
| Lenovo M900 Tiny | 192.168.1.x (static) | 1 (Management) |
| Kids devices | 192.168.10.x | 10 (Kids) |
| Adult devices | 192.168.1.x | 1 (Admin) |

### VPN / Lambert Network

- **Lambert VPN Server:** `edenredux.servegame.com` / `174.54.51.209:51820`
- **VPN Server Public Key:** `uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=`
- **Lambert network:** `192.168.0.x`
- **Target:** `192.168.0.100:5006` (homeschool platform)
- **Pi 4 WireGuard IP:** `192.168.2.4/32`
- **Pi 4 Public Key:** `Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=`
- **Pi 4 Private Key:** `4Ll8kfPH48svJToniFdxqorU3Hcz/lpK1AcWE0JtTEE=`

Kids' devices route `192.168.0.x` traffic through Pi 4 → WireGuard tunnel → Lambert

---

## Setup Order

### Phase 1 — Lenovo ThinkCentre M900 Tiny (Fedora Server + UniFi Controller)

1. Install Fedora Server on Lenovo ThinkCentre M900 Tiny
2. Install UniFi controller software
3. Back up current config from Cloud Key
4. Restore backup to new controller
5. Adopt USG to new controller
6. Verify everything works
7. Retire Cloud Key

**UniFi Controller install on Fedora:**
```bash
# Add MongoDB repo (UniFi dependency)
# Install UniFi via script or manual package
# Enable and start unifi service
# Access at https://192.168.1.x:8443
```

---

### Phase 2 — Pi 4 Setup (Pi-hole + WireGuard)

#### Pi-hole Installation
```bash
curl -sSL https://install.pi-hole.net | bash
```
- Interface: eth0 (wired — critical for reliability)
- Upstream DNS: 1.1.1.1, 8.8.8.8
- Static IP: assign in router DHCP reservation

**Whitelist Mode — Block Everything Except Approved Sites:**
```bash
# Block all domains
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) \
  VALUES ('.*', 3, 1, 'Block all domains');"
sudo pihole reloaddns
```

**Approved Whitelist (Kids):**
```bash
# Add whitelisted domain (repeat for each)
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) \
  VALUES ('domain.com', 2, 1, 'Description');"
sudo pihole reloaddns
```

| Domain | Reason |
|---|---|
| `homeschoolconnections.com` | Homeschool platform |
| `caravel.software` | Homeschool assets |
| `cloudfront.net` | CDN (homeschool) |
| `amazonaws.com` | AWS (homeschool) |
| `gstatic.com` | Google static assets |
| `googleapis.com` | Google APIs |
| `google.com` | Google (limited) |
| `vimeo.com` | Educational videos |
| `vimeocdn.com` | Vimeo CDN |

**Note:** `192.168.0.100:5006` is a direct IP — bypasses DNS, always accessible.

**Pi-hole Admin:** `http://192.168.1.x/admin`

#### WireGuard on Pi 4
```bash
sudo apt install wireguard-tools

# Config at /etc/wireguard/wg0.conf
```

```ini
[Interface]
PrivateKey = 4Ll8kfPH48svJToniFdxqorU3Hcz/lpK1AcWE0JtTEE=
Address = 192.168.2.4/32
PostUp = iptables -t nat -A POSTROUTING -o wg0 -j MASQUERADE
PostDown = iptables -t nat -D POSTROUTING -o wg0 -j MASQUERADE

[Peer]
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
Endpoint = 174.54.51.209:51820
PersistentKeepalive = 25
```

```bash
sudo systemctl enable wg-quick@wg0
sudo systemctl start wg-quick@wg0
```

**Add Pi 4 peer to Lambert VPN server** (do from Windows PC while VPN is connected):
```bash
ssh mac@192.168.0.1  # password: 645866
# Add to WireGuard server config:
# [Peer]
# PublicKey = Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=
# AllowedIPs = 192.168.2.4/32
```

---

### Phase 3 — UniFi VLAN Configuration

In UniFi controller:

1. **Create Kids VLAN (VLAN 10)**
   - Network name: Kids
   - Subnet: 192.168.10.0/24
   - DHCP: enabled
   - DHCP DNS: Pi 4 IP (static)

2. **Create WiFi Networks**
   - `MILTONHAUS` → assigned to Kids VLAN 10
   - `MILTONHAUS_ADMIN` → assigned to VLAN 1 (no restrictions)

3. **Assign kids' devices** to MILTONHAUS SSID

---

### Phase 4 — USG Firewall Rules (DNS Enforcement)

These rules make parental controls **unbypassable** — even if a kid changes DNS on their device:

**Rule 1: Intercept all DNS from Kids VLAN → force to Pi-hole**
```
Source: 192.168.10.0/24
Destination Port: 53
Action: DNAT → Pi 4 IP:53
```

**Rule 2: Block DNS-over-TLS from Kids VLAN**
```
Source: 192.168.10.0/24
Destination Port: 853
Action: DROP
```

**Rule 3: Block inter-VLAN traffic (kids can't reach admin network)**
```
Source: 192.168.10.0/24
Destination: 192.168.1.0/24
Action: DROP
```

**Rule 4: Allow Kids VLAN → Pi 4 (DNS + VPN gateway)**
```
Source: 192.168.10.0/24
Destination: Pi 4 IP
Action: ACCEPT
```

---

## Credentials Reference

| Device | Access | Credentials |
|---|---|---|
| USG 3P | SSH | `mlWKaph@192.168.1.1` / `QmJ7bDN6Ed2` |
| UniFi Controller | Web | `ericmilton711@gmail.com` / `PASSword!?1711` |
| Cloud Key | SSH | `root@192.168.1.6` / `PASSword!?1711` |
| Lambert Router | SSH | `mac@192.168.0.1` / `645866` |
| Windows PC | WireGuard | Lambert tunnel — 192.168.2.2/32 |

---

## Pi-hole Management Quick Commands

```bash
# Check status
pihole status

# Add to whitelist
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) \
  VALUES ('newsite.com', 2, 1, 'Reason');"
sudo pihole reloaddns

# Remove from whitelist
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "DELETE FROM domainlist WHERE domain = 'newsite.com';"
sudo pihole reloaddns

# Temporarily disable parental controls (adults)
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "DELETE FROM domainlist WHERE domain = '.*' AND type = 3;"
sudo pihole reloaddns

# Re-enable parental controls
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) \
  VALUES ('.*', 3, 1, 'Block all domains');"
sudo pihole reloaddns

# Check WireGuard VPN status
sudo wg show
```

---

## Related Skills

- `.claude/skills/project-management/lambert-network-access.md`
- `.claude/skills/project-management/wireguard-usg-installation-2026-01-06.md`
- `.claude/skills/project-management/pihole-parental-controls.md`
- `.claude/skills/project-management/raspberry-pi-wireguard-pihole-setup.md`
- `.claude/skills/home-assistant-container-pi/SKILL.md`
