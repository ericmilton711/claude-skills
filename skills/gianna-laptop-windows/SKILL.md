# Gianna's Laptop — Acer Aspire A515-46

**Last Updated:** 2026-07-30
**Status:** BIOS still locked — needs CH341A reflash. Running Fedora. DNS and Pi-hole fully configured. CD ripping set up.

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

**Corrected 2026-07-14** — this section previously said client 12 was in `kids2` (group 3). That was wrong: she is actually in her own dedicated group, `gianna-laptop` (group 8), described as "allow all, block Google and YouTube."

- **Pi-hole client ID:** 12
- **Pi-hole group:** `gianna-laptop` (Group ID: 8) — **default-allow**, not default-deny like kids1/kids2
- **Model:** Everything is allowed EXCEPT domains explicitly denied for group 8 (currently Google search + YouTube + associated CDNs). Gmail/Docs/Drive/Chat/Accounts are explicitly allow-listed so they survive the Google-search block.
- **Denied for group 8:** `google.com`, `youtube.com`, `youtu.be`, `ytimg.com`, `googlevideo.com`, `yt3.ggpht.com`
- **Allowed for group 8 (carve-outs):** `mail.google.com`, `gmail.com`, `accounts.google.com`, `googleapis.com`, `gstatic.com`, `googleusercontent.com`, `docs.google.com`, `drive.google.com`, `chat.google.com`
- **TEMPORARY denied for group 8:** `duckduckgo.com` (rule id 338, added 2026-07-14, comment "unblock when told") — Eric asked for this to stay blocked until he says otherwise. Remove via `POST /api/domains/deny/regex` delete or `DELETE /api/domains/deny/regex/(^|[.])duckduckgo[.]com$` (URL-encoded), then gravity + restartdns, when he gives the go-ahead.

### 2026-07-14 incident — Google/YouTube access bypass, fixed
Two independent bugs let her reach Google and YouTube:
1. **DNS bypass:** her laptop's `/etc/resolv.conf` had drifted to public resolvers (1.1.1.1, 8.8.8.8, plus IPv6 DNS), bypassing Pi-hole entirely. Fixed by re-pointing `ipv4.dns` to `192.168.12.136`, disabling IPv6 on the connection, and re-locking `/etc/resolv.conf` immutable (`chattr +i`) — see the DNS section above for the exact commands, same pattern as the original setup.
2. **Missing group wiring:** the "Block YouTube"/"Block Google" domain rules existed in Pi-hole but were only assigned to group 0 (Default) — never to group 8. Her group's own description ("block Google and YouTube") was never actually implemented. Fixed via the Pi-hole REST API (`PUT /api/domains/deny/regex/<domain>`) by adding group 8 to each rule's `groups` array, then `POST /api/action/gravity` + `POST /api/action/restartdns` to apply.

Fixed via the API (no SSH needed) — see [[reference_pihole_api]]. Note: SSH to the ThinkCentre (192.168.12.136) was hitting the known exec-hang bug during this incident (see `project_thinkcentre_ssh_exec_hang` memory) — the REST API worked fine as a fallback.

### To temporarily unrestrict (allow literally everything):
```bash
# Via API — remove group 8 membership
SID=$(curl -s -X POST http://192.168.12.136/api/auth -H "Content-Type: application/json" -d '{"password":"645866"}' | python3 -c "import sys,json;print(json.load(sys.stdin)['session']['sid'])")
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.226 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups": []}'
```

### To restore restrictions (back to gianna-laptop, group 8):
```bash
SID=$(curl -s -X POST http://192.168.12.136/api/auth -H "Content-Type: application/json" -d '{"password":"645866"}' | python3 -c "import sys,json;print(json.load(sys.stdin)['session']['sid'])")
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.226 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups": [8]}'
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

## CD Ripping — Asunder (installed 2026-07-30)

Gianna has a USB CD drive plugged into the laptop. Asunder is installed for ripping CDs to MP3.

### What's installed
- **asunder** — GTK CD ripper GUI
- **lame** — MP3 encoder (used by asunder)
- **cdparanoia** — CD audio extraction backend

### Where MP3s go
`/home/gianna/MP3s/` — organized by Artist > Album automatically.

### How Gianna accesses it
- **GNOME dock:** Asunder is pinned as the **first icon** on the favorites bar (CD icon)
- **Nautilus sidebar:** "MP3s" is bookmarked in the file manager sidebar
- **No desktop icons:** Fedora GNOME does not show desktop icons. Everything is via the dock and file manager sidebar.

### Asunder config
Config is managed through the Asunder GUI (Preferences menu), not by editing files directly. Writing the config file programmatically via SSH does not work reliably: asunder overwrites it on close and the file path varies. Always set preferences through the GUI.

Current settings (set manually via GUI 2026-07-30):
- Output: `/home/gianna/MP3s`
- Format: MP3 (VBR)
- CD device: `/dev/sr0`
- Eject on done: yes
- Creates Artist/Album subdirectories

### Permissions
- Gianna is in the `cdrom` group (added 2026-07-30) for `/dev/sr0` access
- Requires logout/login once after group was added

### Usage
1. Put a CD in the USB drive
2. Click the CD icon (first icon on the dock)
3. Click "Rip"
4. CD ejects when done
5. Open file manager, click "MP3s" in sidebar to find songs
6. Plug in MP3 player and copy files over

### If the CD drive isn't detected
- Check `lsblk | grep sr` — should show `sr0`
- Check `groups gianna` — should include `cdrom`
- If cdrom group is missing: `sudo usermod -aG cdrom gianna` then logout/login

---

## Related

- Kids laptops Pi-hole: `skills/kids-laptops-pihole/SKILL.md`
- All devices: `skills/miltonhaus-devices/SKILL.md`
