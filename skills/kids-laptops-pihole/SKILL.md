# Kids Laptops — Pi-hole Parental Controls

**Last Updated:** 2026-04-17
**Status:** Kids1 ✅ Kids2 ✅ Patrick's Chromebook ✅ M900 (Kids Research) ✅ complete. Gianna's Fedora laptop and Ev's Chromebook pending.

---

## Overview

All kids laptops are pointed at Pi-hole (192.168.12.136) for DNS.
Each device has its own Pi-hole group with a custom whitelist.
The block-all regex (`.*`) is assigned to each group — only whitelisted domains resolve.

**Key rule:** Pi-hole blocks DNS only. Direct IP access (like WireGuard IPs) still works.

---

## Critical: Firefox DoH Bypass Fix

Firefox uses DNS-over-HTTPS (DoH) by default, which completely bypasses Pi-hole. The GUI setting ("DNS over HTTPS → Off") is unreliable — Firefox can re-enable it silently. The **only reliable fix** is an enterprise policy file:

**On every Windows machine with Firefox**, create this file via SSH:
```powershell
New-Item -Path "C:\Program Files\Mozilla Firefox\distribution" -ItemType Directory -Force
# Use [System.IO.File]::WriteAllText or base64-encoded PowerShell to preserve JSON quotes:
$json = '{"policies":{"DNSOverHTTPS":{"Enabled":false,"Locked":true}}}'
Set-Content -Path "C:\Program Files\Mozilla Firefox\distribution\policies.json" -Value $json -Force
```

This locks the DoH setting off (greyed out in Firefox settings). Survives Firefox updates and restarts.

**Also critical: IPv6 must be disabled** on all network adapters. Windows gets IPv6 DNS from the router via SLAAC/DHCPv6, which bypasses Pi-hole entirely. Even with IPv4 DNS set to Pi-hole, the browser can resolve via IPv6 DNS.

```powershell
Get-NetAdapterBinding -ComponentId ms_tcpip6 | Where-Object { $_.Enabled } | ForEach-Object { Disable-NetAdapterBinding -Name $_.Name -ComponentId ms_tcpip6 -Confirm:$false }
```

---

## Pi-hole Groups Summary

| Group ID | Name | Device | IP | Whitelist |
|----------|------|--------|----|-----------|
| 0 | Default | All other devices | — | none |
| 1 | mac-mini | Mac Mini | 192.168.12.163 | same as kids1 |
| 2 | kids1 | Kids1 Windows laptop | 192.168.12.249 | standard kids |
| 3 | kids2 | Kids2 Windows laptop | 192.168.12.239 | standard kids + Gmail + Britannica |
| 4 | patricks-chromebook | Patrick's Chromebook + M900 | 192.168.12.221, .160 | standard kids + Britannica |

---

## Mac Mini — Fedora (192.168.12.163)

**Status: ✅ Complete**

- IP: 192.168.12.163
- Pi-hole group: `mac-mini` (Group ID: 1)
- Whitelist: same as Kids1 (homeschoolconnections.com, teachingtextbooks.com, teachingtextbooksapp.com, duolingo.com, kiddle.co, www.kiddle.co, CDN domains)
- No Gmail, no Google Search, no YouTube

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
- kiddle.co + www.kiddle.co (safe search engine)
- **Zoom** (for Homeschool Connections videos): zoom.us, homeschoolconnections.zoom.us, st1.zoom.us, us02st1.zoom.us, explore.zoom.us, us02web.zoom.us, ssrweb.zoom.us, ssrweb-cf.zoom.us, us.telemetry.zoom.us
- ssl.gstatic.com (required for Zoom web player)
- cdn.cookielaw.org
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
2. Disable IPv6 on ALL adapters (critical — YouTube bypasses Pi-hole via IPv6 otherwise):
   ```powershell
   Get-NetAdapterBinding -ComponentId ms_tcpip6 | Where-Object { $_.Enabled } | ForEach-Object { Disable-NetAdapterBinding -Name $_.Name -ComponentId ms_tcpip6 -Confirm:$false }
   ```
3. Lock Firefox DoH off (critical — Firefox bypasses Pi-hole via DNS-over-HTTPS otherwise):
   ```powershell
   New-Item -Path "C:\Program Files\Mozilla Firefox\distribution" -ItemType Directory -Force
   $json = '{"policies":{"DNSOverHTTPS":{"Enabled":false,"Locked":true}}}'
   Set-Content -Path "C:\Program Files\Mozilla Firefox\distribution\policies.json" -Value $json -Force
   ```
4. Flush DNS and restart:
   ```powershell
   ipconfig /flushdns
   ```
5. Enable SSH:
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

## Kids2 — Benedict's Windows Laptop (Lenovo V15 G2 IJL)

**Status: ✅ Complete**

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
# (use pexpect from Eric's laptop — key auth not set up, sshpass banned)
```

### Allowed Sites
- Same as Kids1 (homeschoolconnections.com, teachingtextbooks.com, teachingtextbooksapp.com, duolingo.com, kiddle.co, supporting CDN domains)
- **Gmail access** (Google Search and YouTube remain blocked):
  - `mail.google.com`
  - `accounts.google.com`
  - `gmail.com`
  - `googleapis.com`
  - `googleusercontent.com`
  - `gstatic.com`
- **Britannica** (added 2026-04-17):
  - `britannica.com`, `www.britannica.com`, `cdn.britannica.com`
  - Supporting: `static.cloudflareinsights.com`, `fonts.googleapis.com`, `www.googleapis.com`, `www.googletagmanager.com`, `launchpad-wrapper.privacymanager.io`, `www.googletagservices.com`, `dev.visualwebsiteoptimizer.com`

### WireGuard Setup
- Config: `C:\lambert.conf` (uses direct IP `174.54.51.209:51820` — NOT hostname)
- Service: `WireGuardTunnel$lambert` — auto-starts on boot
- Installed via MSI (scp'd from Linux) + tunnel service via SSH:
  ```powershell
  Start-Process "C:\Program Files\WireGuard\wireguard.exe" -ArgumentList "/installtunnelservice C:\lambert.conf" -Verb RunAs -Wait
  ```

### Firefox
- Homepage: `http://192.168.0.100:5006` (Milton Home Page — requires WireGuard)
- Bookmark: "Milton Home Page" added to Bookmarks Bar (set via places.sqlite edit over SSH)

### Setup Progress
- [x] Converted Microsoft account to local account (themi / 1229)
- [x] SSH enabled
- [x] IPv6 disabled
- [x] DNS set to Pi-hole (192.168.12.136)
- [x] Pi-hole group created and whitelist applied
- [x] WireGuard installed and running as service
- [x] Firefox homepage + bookmark set
- [x] Firefox DoH policy locked off (policies.json) — added 2026-04-17
- [x] IPv6 disabled on all adapters (lambert, Ethernet, Bluetooth, LAN had it re-enabled) — fixed 2026-04-17

### Important Notes
- Username on machine is `themi` — same as Kids1, fine since they are separate devices
- Gmail whitelisted via subdomains only — `google.com` and `youtube.com` remain blocked
- WireGuard MSI had to be downloaded on Linux and scp'd over — direct download on Windows failed due to TLS revocation check (Pi-hole blocks CRL servers)
- Multi-line PowerShell commands don't execute over SSH — use single-line with commas for arrays (e.g. `Set-Content -Value 'line1','line2'`)
- WireGuard silent install required a scheduled task workaround (msiexec fails in non-interactive SSH session)
- **Firefox DoH policy is critical** — without it, Firefox resolves DNS over HTTPS and bypasses Pi-hole entirely
- **IPv6 can re-enable itself** after Windows updates or WireGuard changes — verify periodically

---

## M900 — Kids Research Computer (Lenovo ThinkCentre M900)

**Status: ✅ Complete (2026-04-17)**

- IP: 192.168.12.160
- Username: user
- Password: 645866
- Pi-hole group: `patricks-chromebook` (Group ID: 4) — same restrictions as Patrick's Chromebook
- SSH: enabled (OpenSSH Server installed manually)

### SSH Access
```bash
# Key auth NOT set up — use pexpect:
python3 -c "
import pexpect, base64
ps_cmd = '<your powershell command>'
encoded = base64.b64encode(ps_cmd.encode('utf-16-le')).decode()
child = pexpect.spawn(f'ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10 user@192.168.12.160 powershell -EncodedCommand {encoded}', timeout=30)
child.expect('[Pp]assword')
child.sendline('645866')
child.expect(pexpect.EOF)
print(child.before.decode())
"
```

### Allowed Sites
- Same as Patrick's Chromebook (homeschoolconnections.com, teachingtextbooks.com, teachingtextbooksapp.com, duolingo.com, kiddle.co, supporting CDN domains)
- **Britannica**: britannica.com, www.britannica.com, cdn.britannica.com + supporting domains

### Pi-hole SafeSearch Enforcement
CNAME records added to force SafeSearch on Google and Bing for ALL Pi-hole users:
- `www.google.com` → `forcesafesearch.google.com`
- `google.com` → `forcesafesearch.google.com`
- `www.bing.com` → `strict.bing.com`
- `bing.com` → `strict.bing.com`

Allow rules for `forcesafesearch.google.com` and `strict.bing.com` added to all groups.

### Setup Checklist
- [x] OpenSSH Server installed and started (Automatic startup)
- [x] DNS set to Pi-hole (192.168.12.136) — manual IPv4, no alternate
- [x] Windows DNS-over-HTTPS: Off
- [x] IPv6 disabled on all adapters (Wi-Fi, Ethernet, Bluetooth)
- [x] Firefox DoH policy locked off (policies.json)
- [x] Pi-hole group 4 assigned
- [x] DNS cache flushed
- [x] Blocking verified (YouTube, Facebook, Reddit, Google all blocked)

### Important Notes
- Windows username is `user` (not themi) — default account on this machine
- Key auth failed during setup — `administrators_authorized_keys` never got written correctly. Use pexpect with password instead.
- **sshpass is banned** — use Python pexpect for password-based SSH
- IPv6 was the hardest bypass to find — browser queries weren't appearing in Pi-hole at all because Windows was using IPv6 DNS from the router

---

## Patrick's Chromebook

**Status: ✅ Complete**

- IP: 192.168.12.221 (note: was expected .220 but DHCP assigned .221)
- Pi-hole group: `patricks-chromebook` (Group ID: 4)

### Allowed Sites (same as Kids1)
- homeschoolconnections.com + caravel.software, cloudfront.net, amazonaws.com, vimeo.com, vimeocdn.com
- teachingtextbooks.com + teachingtextbooksapp.com
- duolingo.com
- kiddle.co (safe search engine)

### Pi-hole Setup
Pi-hole group 4 created and whitelist applied via:
```bash
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "
INSERT OR IGNORE INTO [group] (id, enabled, name, description) VALUES (4, 1, 'patricks-chromebook', 'Patrick Chromebook - same as kids1');
INSERT OR IGNORE INTO client (ip, comment) VALUES ('192.168.12.220', 'Patrick Chromebook');
INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES ((SELECT id FROM client WHERE ip='192.168.12.220'), 4);
DELETE FROM client_by_group WHERE client_id=(SELECT id FROM client WHERE ip='192.168.12.220') AND group_id=0;
...whitelist inserts...
"
docker exec pihole pihole reloaddns
```

### Chromebook DNS Setup
1. Click clock (bottom-right) → Settings
2. Network → Wi-Fi → click network name
3. Scroll to **Name servers** → select **Custom**
4. Enter `192.168.12.136`
5. Set IPv6 name server to `192.168.12.136` as well

### Important Notes
- IPv6 cannot be disabled system-wide on ChromeOS — set Pi-hole as IPv6 DNS too
- Pi-hole web admin (http://192.168.12.136/admin) requires port 80 open on ThinkCentre firewall:
  ```bash
  echo 645866 | sudo -S firewall-cmd --permanent --add-service=http
  echo 645866 | sudo -S firewall-cmd --reload
  ```

### Setup Checklist
- [x] Pi-hole group created (Group ID 4)
- [x] Whitelist applied (same as Kids1)
- [x] Port 80 opened on ThinkCentre firewall
- [x] DNS set on Chromebook (Settings → Network → Wi-Fi → network name → Name servers → Custom → 192.168.12.136)
- [x] Verified blocking works (google.com blocked, whitelisted sites work)

### Important Notes
- Chromebook got IP .221 not .220 — Pi-hole client registered as .221
- Pi-hole v6 API used for all group/client management (http://192.168.12.136/api)

---

## Gianna's Laptop — Fedora

**Status: ⬜ Pending**

- IP: 192.168.12.226 (per miltonhaus-devices skill)

---

## Chromebooks (x2)

**Status: ⬜ Pending**

- Patrick's Chromebook: 192.168.12.221 ✅
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
