# MILTONHAUS Network Rebuild Plan

**Last Updated:** 2026-03-20
**Status:** Phase 1 complete. Network currently broken — MILTONHAUS2 (old SSID) broadcasting, very slow internet, UniFi controller IP unknown. Need to diagnose.

---

## Goals

1. Parental controls — kids cannot access YouTube or unapproved sites
2. Adults (Eric + spouse) have unrestricted internet access
3. Kids can access homeschool platform at `192.168.0.100:5006` (Lambert network)
4. Clean, maintainable setup with dedicated devices for dedicated roles

---

## Target SSIDs (not yet fully configured)

| SSID | Users | Restrictions |
|---|---|---|
| **DIEMILTONHAUS** | Adults, guests | None (rename from MILTONHAUS_ADMIN) |
| **MILTONHAUS** | Kids' school laptops only | Pi-hole whitelist |

- **MILTONHAUS2** = old/legacy SSID still broadcasting — delete once UniFi access restored
- **DIEMILTONFAM** = T-Mobile home gateway (separate ISP, 192.168.12.x) — leave alone, not part of UniFi setup

---

## Hardware

| Device | Role | Status |
|---|---|---|
| **USG 3P** | Router + VLAN enforcement + firewall | Active — 192.168.1.1 |
| **Pi 4** (ethernet) | Pi-hole (kids DNS) + WireGuard VPN gateway | Status unknown |
| **Lenovo ThinkCentre M900 Tiny** | Fedora Server — UniFi controller | UP AND RUNNING (Phase 1 complete) |
| **Cloud Key** | Retired — do NOT plug back in | Retired |
| **Pi 3 A+** | Repurposed for other projects | Freed up |
| **Windows PC** | Client + management | WiFi only — 192.168.12.220 on DIEMILTONFAM |
| **Mac** | Client only | 192.168.1.7 |

**Lenovo ThinkCentre M900 Tiny specs:** Intel i5-6500T, 8GB RAM, 256GB SSD, Fedora Server
**UniFi controller IP:** Unknown — needs to be found (try 192.168.1.2–192.168.1.5 at port 8443)

---

## Current Network Issues (as of 2026-03-20)

- MILTONHAUS network has very slow internet — cause unknown
- UniFi controller shows **offline** in unifi.ui.com — likely because MILTONHAUS internet is broken
- Old SSID **MILTONHAUS2** is broadcasting instead of planned MILTONHAUS/DIEMILTONHAUS
- Phone connects to MILTONHAUS2 but drops back to DIEMILTONFAM (no internet on MILTONHAUS2)
- ThinkCentre's UniFi controller IP is unknown — couldn't reach it during troubleshooting
- Windows PC ethernet shows **disconnected** — not causing the network issue

**Next session plan:**
1. Find ThinkCentre's IP (connect phone to MILTONHAUS2, force stay connected, try 192.168.1.2–1.5:8443)
2. Access UniFi controller locally
3. Rename MILTONHAUS_ADMIN → DIEMILTONHAUS
4. Delete MILTONHAUS2
5. Diagnose slow internet (check USG WAN status, check for loops)

---

## Physical Wiring Diagram

```
Modem/ISP
    |
    | (WAN port)
  USG 3P
    | (LAN1 port)
    |
5-port 1GB Switch
    |           |           |           |
  UniFi AP    Mac       Lenovo M900   Pi 4
 (PoE port) (ethernet)  (Fedora/     (Pi-hole +
                         UniFi)       WireGuard)

Windows PC → WiFi only (do NOT plug into switch unless needed)
Pi 3 A+    → WiFi (repurposed)
```

**Switch port assignments:**
| Port | Device |
|---|---|
| 1 | USG 3P (LAN1) |
| 2 | UniFi AP Long Range (PoE or via PoE injector) |
| 3 | Mac |
| 4 | Lenovo ThinkCentre M900 Tiny (Fedora server) |
| 5 | Pi 4 (Pi-hole + WireGuard) |

> **Note:** All 5 switch ports are used. Do not plug Windows PC into switch without disconnecting another device first.

---

## Network Architecture

### Two VLANs

| Network | SSID | Users | DNS | Internet |
|---|---|---|---|---|
| **VLAN 1** | DIEMILTONHAUS | Eric, Spouse | Unrestricted | Full |
| **VLAN 10** | MILTONHAUS | Kids | Pi 4 Pi-hole (whitelist) | Restricted |

### IP Scheme

| Device | IP | VLAN |
|---|---|---|
| USG 3P | 192.168.1.1 | 1 (Management) |
| Pi 4 | 192.168.1.x (static) | 1 (serves kids VLAN) |
| Lenovo M900 Tiny | 192.168.1.x (static — unknown, find via scan) | 1 (Management) |
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

### Phase 1 — Lenovo ThinkCentre M900 Tiny (Fedora Server + UniFi Controller) — COMPLETE

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
   - `DIEMILTONHAUS` → assigned to VLAN 1 (no restrictions)
   - Delete `MILTONHAUS2` (old legacy SSID)
   - Delete `MILTONHAUS_ADMIN` if it exists

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
| Cloud Key | SSH (retired — do not use) | `root@192.168.1.6` / `PASSword!?1711` |
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
