---
name: thinkcentre-samba-share
description: Samba file share on ThinkCentre (.136) — setup, access from Windows/Linux, credentials, drive mapping, and troubleshooting. Use when accessing or managing the shared folder at \\192.168.12.136\shared.
metadata:
  type: project
---

# ThinkCentre Samba File Share

**Server:** ThinkCentre M700 at 192.168.12.136 (user: `milton`)
**Share path on server:** `/srv/shared`
**Windows UNC path:** `\\192.168.12.136\shared`
**Samba credentials:** username `milton`, password `645866`

---

## Samba Config (`/etc/samba/smb.conf`)

```ini
[shared]
    path = /srv/shared
    guest ok = Yes
    read only = No
    force user = milton
    create mask = 0666
    directory mask = 0777
```

Services: `smb.service` and `nmb.service` (both enabled and running)

---

## Accessing the Share

### From Windows (Eva's MSI Laptop, .202)

**In Firefox address bar** (easiest):
```
\\192.168.12.136\shared
```

**Via File Explorer:** Double-click the **"Server - Shared Files"** shortcut on Eva's desktop.

**Z: drive** maps automatically at login via startup script:
```
C:\Users\eva milton\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\map-server.bat
```
Contents: `explorer.exe \\192.168.12.136\shared`

**If Windows blocks guest access** (error: "security policies block unauthenticated guest access"):
```
reg add "HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters" /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f
```
Then reboot the Windows machine.

**If prompted for credentials:** username `milton`, password `645866`

### From Eric's Fedora Laptop (this machine)

Open in Nautilus:
```bash
setsid nautilus smb://192.168.12.136/shared &
```

Firefox bookmark policy at `/usr/lib64/firefox/distribution/policies.json`:
```json
{"policies":{"Bookmarks":[{"Title":"Server - Shared Files","URL":"smb://192.168.12.136/shared","Placement":"toolbar"}]}}
```

---

## Firefox Bookmark Policy (Both Machines)

Firefox policy files add a permanent "Server - Shared Files" toolbar bookmark. Restart Firefox to apply.

**Eva's laptop:** `C:\Program Files\Mozilla Firefox\distribution\policies.json`
```json
{"policies":{"Bookmarks":[{"Title":"Server - Shared Files","URL":"file://192.168.12.136/shared","Placement":"toolbar"}]}}
```
Note: Firefox bookmark clicks may not open UNC paths — use the desktop shortcut or type directly in Firefox address bar instead.

**Eric's laptop:** `/usr/lib64/firefox/distribution/policies.json`
```json
{"policies":{"Bookmarks":[{"Title":"Server - Shared Files","URL":"smb://192.168.12.136/shared","Placement":"toolbar"}]}}
```

---

## Managing the Share (SSH into ThinkCentre)

```bash
ssh -i ~/.ssh/id_ed25519 milton@192.168.12.136

# List files
ls -la /srv/shared/

# Check Samba status
systemctl status smb nmb

# Restart Samba
sudo systemctl restart smb nmb

# Reload after config changes
sudo systemctl reload smb
```

---

## Troubleshooting

**Z: drive shows "Unavailable" via SSH:** Normal — SSH runs in a separate Windows session. Check File Explorer in Eva's interactive session instead.

**Files visible via SSH `dir` but not in File Explorer:** Run the `AllowInsecureGuestAuth` registry fix above, then reboot.

**"Windows cannot access \\192.168.12.136\shared":** Port 445 is open (confirmed). Usually a credentials/guest-auth issue — apply the registry fix.

**Windows Update on reboot:** Brand new laptop had months of pending updates. One-time catch-up, normal behavior.
