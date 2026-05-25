# Gianna's Laptop — Acer Aspire A515-46

**Last Updated:** 2026-05-24
**Status:** ⬜ BIOS still locked — needs CH341A reflash. Running Fedora. Pi-hole DNS configured 2026-05-24.

---

## Hardware

- **Brand/Model:** Acer Aspire A515-46 series (note: originally thought to be Asus — it's Acer)
- **Serial Number:** NXABRAA00715113E9D7600
- **Current OS:** Fedora Linux
- **Target OS:** Windows (original OS)
- **Network IP:** 192.168.12.226
- **WiFi Interface:** wlp2s0
- **WiFi Connection:** DIEMILTONHAUS
- **Hostname:** fedora
- **Boot Media:** Windows Media Install on mini SD card

---

## Login

- **Username:** gianna
- **Password:** wisdom22!!
- **SSH:** Port open but key auth NOT set up. Password auth was rejected over SSH (works locally). Needs investigation.

---

## Fedora Network/DNS Config (2026-05-24)

DNS was completely broken out of the box. Required four fixes:

### 1. Firewall — DNS service was missing
Fedora's firewalld was blocking outbound DNS (UDP port 53). Fixed permanently:
```bash
sudo firewall-cmd --add-service=dns --permanent
sudo firewall-cmd --reload
```

### 2. nmcli — Point DNS at Pi-hole
```bash
sudo nmcli con modify DIEMILTONHAUS ipv4.dns "192.168.12.136"
sudo nmcli con modify DIEMILTONHAUS ipv4.ignore-auto-dns yes
sudo nmcli con down DIEMILTONHAUS && sudo nmcli con up DIEMILTONHAUS
```

### 3. resolv.conf — Bypass broken systemd-resolved stub
systemd-resolved's stub listener (127.0.0.53) was not forwarding queries even though it showed the correct upstream DNS. Overwrote resolv.conf directly:
```bash
echo "nameserver 192.168.12.136" | sudo tee /etc/resolv.conf
```
**Warning:** systemd-resolved may overwrite this on reboot/reconnect. If DNS breaks again, check this file first.

### 4. nsswitch.conf — Remove broken resolve module
glibc was using the `resolve` NSS module (systemd-resolved D-Bus) before falling through to `dns` (resolv.conf). The resolve module was broken/timing out. Removed it:
```bash
sudo sed -i '/^hosts:/c\hosts: files myhostname mdns4_minimal [NOTFOUND=return] dns' /etc/nsswitch.conf
```
Original line was: `hosts: files myhostname mdns4_minimal [NOTFOUND=return] resolve [!UNAVAIL=return] dns`

### Firefox DoH Policy — NOT YET APPLIED
Attempted to create `/etc/firefox/policies/policies.json` but the directory creation failed (mkdir ran without leading `/`). Needs to be redone:
```bash
sudo mkdir -p /etc/firefox/policies
echo '{"policies":{"DNSOverHTTPS":{"Enabled":false,"Locked":true}}}' | sudo tee /etc/firefox/policies/policies.json
```

---

## Pi-hole Status

- **Pi-hole client ID:** 12
- **Pi-hole group:** `kids2` (Group ID: 3) — same restrictions as Benedict's laptop
- **Allowed:** Gmail, Google Chat, Britannica, Zoom, Homeschool Connections, Teaching Textbooks, Duolingo, Kiddle, Rhymezone, and supporting CDN domains
- **Blocked:** Everything else (Google Search, YouTube, social media, etc.)

### To temporarily unrestrict:
```bash
ssh -i ~/.ssh/id_ed25519 milton@192.168.12.136
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM client_by_group WHERE client_id = 12;"
docker exec pihole pihole reloaddns
```

### To restore restrictions (back to kids2):
```bash
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (12, 3);"
docker exec pihole pihole reloaddns
```

---

## The Problem — BIOS Password

The BIOS asks for a supervisor password before allowing boot order changes or external boot.

### What Was Tried
- Blank password (Enter) — failed
- `admin` — failed
- `1234` — failed
- `0000` — failed
- CMOS battery removal — **did not work** (modern UEFI stores password in EEPROM, not CMOS)
- F12 boot menu — Fedora boots instead (BIOS password blocks it too)

### Why Nothing Worked
The A515-46 uses **Insyde H2O BIOS**. On this model:
- BIOS supervisor password is stored in EEPROM/flash, not CMOS — battery removal won't clear it
- No hash code is displayed after failed attempts (intentional on post-2019 Acer models)
- Serial-number-based master password generators don't work without a hash
- Gianna did not set this password — origin unknown

### BIOS Entry / Boot Keys
- **F2** — BIOS setup (prompts for password)
- **F12** — one-time boot menu (also blocked by password)

---

## Solution — DIY BIOS Reflash (CH341A Programmer)

### What to Order
- **CH341A USB programmer + SOIC8 clip kit** (~$10-15 on Amazon)
- Search: "CH341A programmer SOIC8 clip kit"
- Make sure the SOIC8 clip is included — some listings sell the programmer alone

### How It Works
The clip attaches directly to the BIOS chip on the motherboard (no soldering).
The CH341A connects to a PC via USB and reflashes the chip, wiping the password.

### Steps (once programmer arrives)
- TBD — will walk through when hardware is in hand

---

## After BIOS is Cleared — Windows Install Steps

1. Insert mini SD card with Windows Media Install
2. Power on, tap **F12** immediately → select SD card from boot menu
3. Follow Windows setup wizard
4. After Windows is installed, set up Pi-hole and WireGuard (see `skills/kids-laptops-pihole/SKILL.md`)

---

## Pi-hole Setup (after Windows install)

- Pi-hole group pending for Gianna's laptop
- IP: 192.168.12.226
- Follow the same pattern as Kids1/Kids2 in `skills/kids-laptops-pihole/SKILL.md`
- Determine whitelist needs (may differ from Kids1/Kids2)

---

## Related

- Kids laptops Pi-hole: `skills/kids-laptops-pihole/SKILL.md`
- All devices: `skills/miltonhaus-devices/SKILL.md`
