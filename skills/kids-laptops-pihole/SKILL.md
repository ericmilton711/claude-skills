# Kids Laptops — Pi-hole Parental Controls

**Last Updated:** 2026-03-30
**Status:** Kids1 (Windows) locked down. Kids2 in progress. Fedora laptop and 2 Chromebooks pending.

---

## Overview

All kids laptops are pointed at Pi-hole (192.168.12.136) for DNS.
Each device has its own Pi-hole group with a custom whitelist.
The block-all regex (`.*`) is assigned to each group — only whitelisted domains resolve.

**Key rule:** Pi-hole blocks DNS only. Direct IP access (like WireGuard IPs) still works.

---

## Pi-hole Groups Summary

| Group ID | Name | Device | IP |
|----------|------|--------|----|
| 0 | Default | All other devices | — |
| 1 | mac-mini | Mac Mini | 192.168.12.163 |
| 2 | kids1 | Kids1 Windows laptop | 192.168.12.249 |

---

## Kids1 — Windows Laptop (Lenovo V15 G2 IJL)

**Status: ✅ Complete**

- IP: 192.168.12.249
- Username: themi
- Password: 1229
- Pi-hole group: `kids1`
- SSH: enabled (OpenSSH Server installed)

### SSH Access
```bash
ssh themi@192.168.12.249
# password: 1229
```

### Allowed Sites
- homeschoolconnections.com (+ caravel.software, cloudfront.net, amazonaws.com, vimeo.com, vimeocdn.com)
- teachingtextbooks.com + **teachingtextbooksapp.com** (the app uses this domain, not teachingtextbooks.com)
- duolingo.com
- Milton Home Page: http://192.168.0.100:5006 (via WireGuard — direct IP, bypasses Pi-hole)

### Blocked
- Google, YouTube, social media, and all other domains

### WireGuard Setup
- Config: `C:\lambert.conf` (uses direct IP `174.54.51.209:51820` — NOT hostname, hostname won't resolve through Pi-hole)
- Service: `WireGuardTunnel$lambert` — auto-starts on boot
- Install via SSH:
  ```powershell
  # Write config to C:\lambert.conf first, then:
  Start-Process "C:\Program Files\WireGuard\wireguard.exe" -ArgumentList "/installtunnelservice C:\lambert.conf" -Verb RunAs -Wait
  ```

### Important Notes
- **teachingtextbooksapp.com** is the actual domain the Teaching Textbooks app uses — teachingtextbooks.com alone is not enough
- **Google is blocked** — removed from whitelist intentionally
- **IPv6 must be disabled** — YouTube and others bypass Pi-hole via IPv6 if enabled
- Windows shows "No internet, secured" on WiFi — this is normal, Pi-hole blocks Microsoft's connectivity check (msftconnecttest.com). Allowed sites still work fine.
- **WireGuard config must use direct IP** (`174.54.51.209`) not hostname (`edenredux.servegame.com`) — Pi-hole blocks the hostname lookup
- To install apps through Pi-hole: temporarily whitelist download domains, install, then remove from whitelist

### Windows Setup Steps (if rebuilding)
1. Set DNS via PowerShell Admin:
   ```powershell
   netsh interface ip set dns name="Wi-Fi" static 192.168.12.136
   ```
2. Disable IPv6 (critical — YouTube bypasses Pi-hole via IPv6 otherwise):
   ```powershell
   Disable-NetAdapterBinding -Name "Wi-Fi" -ComponentID ms_tcpip6
   ```
3. Flush DNS and restart:
   ```powershell
   ipconfig /flushdns
   ```
4. Enable SSH:
   ```powershell
   Add-WindowsCapability -Online -Name OpenSSH.Server~~~~0.0.1.0
   Start-Service sshd
   Set-Service -Name sshd -StartupType Automatic
   New-NetFirewallRule -Name sshd -DisplayName 'OpenSSH Server (sshd)' -Enabled True -Direction Inbound -Protocol TCP -Action Allow -LocalPort 22
   ```
5. Enable password auth in SSH config:
   ```powershell
   (Get-Content "C:\ProgramData\ssh\sshd_config") -replace '#PasswordAuthentication yes', 'PasswordAuthentication yes' | Set-Content "C:\ProgramData\ssh\sshd_config"
   Restart-Service sshd
   ```

### Important Notes
- **IPv6 MUST be disabled** — Windows will use IPv6 DNS from the router and bypass Pi-hole entirely
- SSH service stops after Windows updates — need to run `Start-Service sshd` again
- `themi` was switched from Microsoft account to local account for SSH to work
- sshd auto-start set but may need re-enabling after major updates

---

## Kids2 — Windows Laptop (Lenovo V15 G2 IJL)

**Status: 🔧 In Progress**

- IP: 192.168.12.239
- Hardware: Identical to Kids1 (Lenovo V15 G2 IJL)
- RAM: 8GB (Kids1 has 16GB)
- Username: themi
- Password: 1229
- Pi-hole group: `kids2` (Group ID: 3)

### SSH Access
```bash
ssh themi@192.168.12.239
# password: 1229
```

### Allowed Sites
- Same as Kids1 (homeschoolconnections.com, teachingtextbooks.com, teachingtextbooksapp.com, duolingo.com, supporting CDN domains)
- **Gmail access** (Google Search and YouTube remain blocked):
  - `mail.google.com`
  - `accounts.google.com`
  - `gmail.com`
  - `googleapis.com`
  - `googleusercontent.com`
  - `gstatic.com`

### Setup Progress
- [x] Converted Microsoft account to local account (themi / 1229)
- [ ] SSH enabled
- [ ] IPv6 disabled
- [ ] DNS set to Pi-hole (192.168.12.136)
- [ ] Pi-hole group created and whitelist applied
- [ ] WireGuard installed (if needed)

### Important Notes
- Username on machine is `themi` — same as Kids1, fine since they are separate devices
- Gmail whitelisted via subdomains only — `google.com` and `youtube.com` remain blocked

---

## Gianna's Laptop — Fedora

**Status: ⬜ Pending**

- IP: 192.168.12.226 (per miltonhaus-devices skill)

---

## Chromebooks (x2)

**Status: ⬜ Pending**

- Patrick's Chromebook: 192.168.12.220
- Ev's Chromebook: Tailscale IP 100.115.92.195

---

## Pi-hole Group Setup (for each new device)

Replace `<GROUP_ID>`, `<GROUP_NAME>`, `<IP>`, and `<COMMENT>` as needed:

```bash
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "
INSERT OR IGNORE INTO 'group' (id, enabled, name, description) VALUES (<GROUP_ID>, 1, '<GROUP_NAME>', '<COMMENT>');
INSERT OR IGNORE INTO client (ip, comment) VALUES ('<IP>', '<COMMENT>');
INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES ((SELECT id FROM client WHERE ip='<IP>'), <GROUP_ID>);
DELETE FROM client_by_group WHERE client_id=(SELECT id FROM client WHERE ip='<IP>') AND group_id=0;
INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES ((SELECT id FROM domainlist WHERE domain='.*' AND type=3), <GROUP_ID>);
"
docker exec pihole pihole reloaddns
```

Then add whitelisted domains and link them to the group:
```bash
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('example.com', 2, 1, 'Description');
INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES ((SELECT id FROM domainlist WHERE domain='example.com'), <GROUP_ID>);
"
docker exec pihole pihole reloaddns
```

---

## Related

- ThinkCentre / Pi-hole server: `skills/home-assistant-thinkcenter/SKILL.md`
- Mac Mini: `skills/mac-mini-fedora/SKILL.md`
- All devices: `skills/miltonhaus-devices/SKILL.md`
