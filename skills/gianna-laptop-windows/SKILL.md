# Gianna's Laptop — Acer Aspire A515-46

**Last Updated:** 2026-05-25
**Status:** BIOS still locked — needs CH341A reflash. Running Fedora. DNS and Pi-hole fully configured.

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
- **SSH:** Key auth working (ed25519 key from Eric's Windows laptop). Password auth also enabled.

---

## Fedora Network/DNS Config

DNS was completely broken out of the box (2026-05-24). Permanently fixed 2026-05-25.

### 1. Firewall — DNS service added permanently
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

### 3. systemd-resolved — Disabled and masked
systemd-resolved's stub listener (127.0.0.53) was broken and kept overwriting resolv.conf on reboot. Permanently killed:
```bash
sudo systemctl disable --now systemd-resolved
sudo systemctl mask systemd-resolved systemd-resolved-varlink.socket systemd-resolved-monitor.socket
```

### 4. NetworkManager DNS mode — Set to direct
Without this, NetworkManager tries to push DNS through systemd-resolved even when it's disabled:
```bash
# /etc/NetworkManager/conf.d/dns.conf
[main]
dns=default
```

### 5. resolv.conf — Locked
NetworkManager writes the correct DNS from nmcli settings. File is locked immutable so nothing overwrites it:
```bash
# Contents: nameserver 192.168.12.136
sudo chattr +i /etc/resolv.conf
```
To modify: `sudo chattr -i /etc/resolv.conf` first, make changes, then re-lock.

### 6. nsswitch.conf — Broken resolve module removed
```bash
# Current working line:
hosts: files myhostname mdns4_minimal [NOTFOUND=return] dns
```

### 7. Firefox DoH Policy — Applied
```bash
# /etc/firefox/policies/policies.json
{"policies":{"DNSOverHTTPS":{"Enabled":false,"Locked":true}}}
```

### If DNS breaks after reboot
1. Check `cat /etc/resolv.conf` — should say `nameserver 192.168.12.136`
2. If wrong: `sudo chattr -i /etc/resolv.conf && sudo nmcli con down DIEMILTONHAUS && sudo nmcli con up DIEMILTONHAUS && sudo chattr +i /etc/resolv.conf`
3. Verify systemd-resolved is still masked: `systemctl is-enabled systemd-resolved` should say `masked`

---

## Pi-hole Status

- **Pi-hole client ID:** 12
- **Pi-hole group:** `kids2` (Group ID: 3)
- **Allowed sites:**
  - Gmail, Google Chat, Google Accounts
  - Duolingo
  - Typing.com
  - Bulk Apothecary (bulkapothecary.com)
  - LEO Dictionary (leo.org)
  - Sally's Baking Addiction (sallysbakingaddiction.com)
  - AccuWeather (accuweather.com)
  - USCCB Catechism (usccb.cld.bz, pages.cld.bz)
  - Suno (suno.com)
  - KWeather app (api.met.no, geonames.org)
  - Britannica, Kiddle, Rhymezone
  - Zoom, Homeschool Connections, Teaching Textbooks
  - Supporting CDNs (cloudfront, amazonaws, gstatic, googleapis)
  - Firefox captive portal (detectportal.firefox.com)
  - Fedora system (fedoraproject.org, fedora.pool.ntp.org)
- **Blocked:** Everything else (Google Search, YouTube, social media, Amazon, etc.)

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

## Related

- Kids laptops Pi-hole: `skills/kids-laptops-pihole/SKILL.md`
- All devices: `skills/miltonhaus-devices/SKILL.md`
