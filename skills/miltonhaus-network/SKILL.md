# MILTONHAUS Network Rebuild Plan

**Last Updated:** 2026-03-21
**Status:** Phase 2 in progress. ThinkCentre running all services in Docker. Pi-hole fixed. AP needs factory reset + adoption.

---

## Goals

1. Parental controls — kids cannot access YouTube or unapproved sites
2. Adults (Eric + spouse) have unrestricted internet access
3. Kids can access homeschool platform at `192.168.0.100:5006` (Lambert network)
4. Clean, maintainable setup with dedicated devices for dedicated roles

---

## Current Network State (as of 2026-03-21)

- **DIEMILTONHAUS** = T-Mobile home gateway WiFi (192.168.12.x) — Eric uses this, it works fine
- **No UniFi SSIDs broadcasting** — AP not yet adopted, no MILTONHAUS or DIEMILTONHAUS from UniFi
- **UniFi AP** needs factory reset + adoption (currently at 192.168.1.8 after cable was moved to correct switch port)
- **Pi-hole** fixed and healthy on ThinkCentre
- **Internet via USG is healthy** — ~35ms ping, no packet loss

### Why You Only See DIEMILTONHAUS
DIEMILTONHAUS is the T-Mobile gateway's WiFi name. The UniFi AP has not been adopted yet so it broadcasts nothing. Once the AP is adopted and configured, it will broadcast MILTONHAUS (kids) and DIEMILTONHAUS (adults) from the UniFi system on the 192.168.1.x network.

---

## Hardware

| Device | Role | Status |
|---|---|---|
| **USG 3P** | Router + VLAN enforcement + firewall | Active — 192.168.1.1 |
| **Lenovo ThinkCentre M900 Tiny** | Fedora Server — runs ALL services in Docker | UP — 192.168.1.107 (WiFi fallback: 192.168.12.136) |
| **UniFi AP Long Range** | WiFi access point | Connected to switch port 2 via PoE injector — IP 192.168.1.8 — needs adoption |
| **Cloud Key** | Retired — do NOT plug back in | Retired |
| **Pi 3 A+** | Repurposed for other projects | Freed up |
| **Pi 4** | Not acquired — services moved to ThinkCentre | N/A |
| **Windows PC** | Client + management | Ethernet: 192.168.1.108 (switch port 5) + WiFi: 192.168.12.220 (T-Mobile) |
| **Mac** | Client | 192.168.1.x (via switch port 3) |

**Lenovo ThinkCentre M900 Tiny specs:** Intel i5-6500T, 8GB RAM, 256GB SSD, Fedora Workstation
**SSH:** `ssh milton@192.168.1.107` / password: `645866`

---

## Docker Services on ThinkCentre (all running)

| Container | Image | Status | Notes |
|---|---|---|---|
| `pihole` | `pihole/pihole:latest` | Healthy | DNS: 192.168.12.136:53 (listening on all interfaces) |
| `wireguard` | `lscr.io/linuxserver/wireguard:latest` | ✅ Running | Lambert tunnel active — config at /home/milton/wireguard/wg0.conf; uses Lambert.conf keypair (192.168.2.2/32); can reach 192.168.0.x |
| `homeassistant` | `ghcr.io/home-assistant/home-assistant:stable` | Running | Home Assistant — port 8123 on host (accessible at 192.168.12.136:8123) |

**Volume mounts:**
- Pi-hole: `/home/milton/pihole/etc-pihole` and `/home/milton/pihole/etc-dnsmasq.d`
- WireGuard: `/home/milton/wireguard`
- Home Assistant: `/home/milton/homeassistant`
- UniFi: `/home/milton/unifi`

---

## Fixes Applied This Session (2026-03-21)

### Pi-hole Fix — systemd-resolved conflict
Pi-hole v6 with `--network host` kept failing to bind port 53 with "Address in use" because systemd-resolved's stub listener competed during startup.

**Fix:** Disable systemd-resolved stub listener:
```bash
# On ThinkCentre (run as root)
echo "[Resolve]" >> /etc/systemd/resolved.conf
echo "DNSStubListener=no" >> /etc/systemd/resolved.conf
systemctl restart systemd-resolved
docker restart pihole
```
After this, Pi-hole binds port 53 successfully and is healthy.

### Firewall — Port 53 opened
```bash
sudo firewall-cmd --add-service=dns --permanent
sudo firewall-cmd --reload
```

### DNS — unifi hostname for AP adoption
Added dnsmasq config so "unifi" resolves to the controller:
```bash
echo "address=/unifi/192.168.1.107" > /home/milton/pihole/etc-dnsmasq.d/unifi.conf
docker restart pihole
```

### AP Physical Cable — moved to correct network
The AP's PoE injector LAN port was connected to the T-Mobile gateway (192.168.12.x) instead of the main switch. Moved to switch port 2. AP now gets 192.168.1.x from USG.

---

## UniFi AP Adoption — INCOMPLETE (next session)

**Current state:** AP is at 192.168.1.8, factory reset attempted but may not have completed.

**Problem:** SSH into AP fails — Fedora's crypto policy blocks the AP's old 1024-bit RSA key and SHA-1.

**Next steps:**
1. Hold AP reset button for 10 seconds (AP is now on correct network — 192.168.1.x)
2. Wait for LED to go white (pulsing = ready for adoption)
3. SSH in using LEGACY crypto policy:
```bash
# On ThinkCentre as root:
update-crypto-policies --set LEGACY
sshpass -p ubnt ssh -o StrictHostKeyChecking=no -o HostKeyAlgorithms=+ssh-rsa ubnt@192.168.1.8 "set-inform http://192.168.1.107:8080/inform"
update-crypto-policies --set DEFAULT
```
4. In browser, go to https://192.168.1.107:8443 and adopt the AP
5. Create SSIDs: MILTONHAUS (kids) and DIEMILTONHAUS (adults, from UniFi)
6. Delete any legacy SSIDs (MILTONHAUS2, MILTONHAUS_ADMIN)

**Note:** After adoption, the T-Mobile gateway SSID "DIEMILTONHAUS" should be RENAMED so it doesn't conflict with the UniFi SSID. Either rename the T-Mobile WiFi or use a different name for the UniFi adult network.

---

## Physical Wiring Diagram

```
Modem/ISP
    |
    | (WAN port) → USG WAN IP on T-Mobile side: 192.168.12.238
  USG 3P (192.168.1.1)
    | (LAN1 port)
    |
5-port 1GB Switch (non-PoE)
    |              |              |           |
  PoE Injector   Mac          Lenovo M900   Windows PC
  LAN port    (ethernet)     (Fedora/Docker)  (switch port 5)
    |
  PoE Injector PoE port
    |
  UniFi AP LR (192.168.1.8)

T-Mobile Gateway (192.168.12.1) → separate network, do not modify
ThinkCentre also has WiFi on T-Mobile side (192.168.12.136) — leave as fallback
```

**Switch port assignments:**
| Port | Device |
|---|---|
| 1 | USG 3P (LAN1) |
| 2 | PoE Injector → UniFi AP Long Range |
| 3 | Mac |
| 4 | Lenovo ThinkCentre M900 Tiny |
| 5 | Windows PC (connect for management) |

---

## Network Architecture

### Two VLANs (planned — not yet configured in UniFi)

| Network | SSID | Users | DNS | Internet |
|---|---|---|---|---|
| **VLAN 1** | DIEMILTONHAUS (UniFi) | Eric, Spouse | Unrestricted | Full |
| **VLAN 10** | MILTONHAUS | Kids | Pi-hole on ThinkCentre (whitelist) | Restricted |

### IP Scheme

| Device | IP | Notes |
|---|---|---|
| USG 3P | 192.168.1.1 | Router/gateway |
| ThinkCentre M900 Tiny | 192.168.1.107 | All services |
| UniFi AP | 192.168.1.8 (DHCP) | Set static reservation once adopted |
| Windows PC | 192.168.1.108 (DHCP) | When plugged into switch |
| Kids devices | 192.168.10.x | After VLAN 10 configured |
| Adult devices | 192.168.1.x | VLAN 1 |

### VPN / Lambert Network

- **Lambert VPN Server:** `edenredux.servegame.com` / `174.54.51.209:51820`
- **VPN Server Public Key:** `uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=`
- **Lambert network:** `192.168.0.x`
- **Target:** `192.168.0.100:5006` (homeschool platform)
- **WireGuard on ThinkCentre IP:** `192.168.2.4/32`
- **WireGuard Public Key:** `Od833amshNe6+MXKa9rm5VyiOoN08BnpPImH2T6W7Ww=`
- **WireGuard Private Key:** `4Ll8kfPH48svJToniFdxqorU3Hcz/lpK1AcWE0JtTEE=`

---

## Remaining Work

### Next Session
1. Factory reset AP (hold button 10s while on correct network)
2. Adopt AP in UniFi controller
3. Create MILTONHAUS and DIEMILTONHAUS SSIDs in UniFi
4. Configure VLAN 10 (Kids) in UniFi
5. Set Pi-hole DNS for VLAN 10 via USG DHCP
6. Add firewall rules in USG (Phase 4)
7. Assign kids' devices to MILTONHAUS SSID
8. Test parental controls

### Future
- Install Fedora on used MacBook Pro (eBay) for music/MIDI + Home Assistant
- Set up DHCP reservations for all static devices

---

## Credentials Reference

| Device | Access | Credentials |
|---|---|---|
| USG 3P | SSH | `mlWKaph@192.168.1.1` / `QmJ7bDN6Ed2` (NOTE: SSH login currently failing — may need reset) |
| USG 3P | Web UI | http://192.168.1.1 |
| UniFi Controller | Web | `ericmilton711@gmail.com` / `PASSword!?1711` — https://192.168.1.107:8443 |
| ThinkCentre | SSH | `milton@192.168.1.107` / `645866` |
| Cloud Key | SSH (retired — do not use) | `root@192.168.1.6` / `PASSword!?1711` |
| Lambert Router | SSH | `mac@192.168.0.1` / `645866` |
| Windows PC | WireGuard | Lambert tunnel — 192.168.2.2/32 |
| UniFi AP | SSH (after adoption) | `ubnt` / `ubnt` (default after factory reset) |

---

## Pi-hole Management (on ThinkCentre via Docker)

```bash
# SSH to ThinkCentre first
ssh milton@192.168.1.107

# Check status
docker exec pihole pihole status

# Add to whitelist
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) \
  VALUES ('newsite.com', 2, 1, 'Reason');"
docker exec pihole pihole reloaddns

# Remove from whitelist
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "DELETE FROM domainlist WHERE domain = 'newsite.com';"
docker exec pihole pihole reloaddns

# Block all (kids parental controls)
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) \
  VALUES ('.*', 3, 1, 'Block all domains');"
docker exec pihole pihole reloaddns

# Disable block-all
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "DELETE FROM domainlist WHERE domain = '.*' AND type = 3;"
docker exec pihole pihole reloaddns

# Check WireGuard VPN status
docker exec wireguard wg show
```

---

## Related Skills

- `.claude/skills/home-assistant-thinkcenter/SKILL.md`
- `.claude/skills/home-assistant-container-pi/SKILL.md`
