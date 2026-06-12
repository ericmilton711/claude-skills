# Samba MSI Sharing — ThinkCentre File Server

## Overview
Samba file share on the ThinkCentre M700 Tiny (192.168.12.136) for sharing files across laptops on the MILTONHAUS network. Set up 2026-06-11 so the kids can transfer files from old devices and work on them from new ones without storing files locally.

## Server Details
- **Server:** ThinkCentre M700 Tiny (Fedora 43)
- **IP:** 192.168.12.136
- **SSH user:** milton
- **Samba user:** milton / 645866
- **Share path on server:** `/srv/shared/`
- **UNC path from Windows:** `\\192.168.12.136\shared`

## What Was Installed/Configured

### Packages
```bash
sudo dnf install -y samba samba-client
```

### Samba Config (`/etc/samba/smb.conf`)
Added to `[global]` section:
```ini
map to guest = Bad User
```

Added share block:
```ini
[shared]
   path = /srv/shared
   browseable = yes
   read only = no
   guest ok = yes
   create mask = 0666
   directory mask = 0777
   force user = milton
```

### Samba User
```bash
printf '645866\n645866\n' | sudo smbpasswd -s -a milton
```

### Services Enabled
```bash
sudo systemctl enable --now smb nmb
```

### Firewall Opened
```bash
sudo firewall-cmd --permanent --add-service=samba
sudo firewall-cmd --reload
```

### SELinux
```bash
sudo setsebool -P samba_export_all_rw on
sudo chcon -t samba_share_t /srv/shared -R
```

## Connecting from Windows

### Manual (File Explorer)
1. Open File Explorer
2. Type `\\192.168.12.136\shared` in the address bar
3. Enter credentials: user `milton`, password `645866`
4. Files open and save directly on the server

### Map as Drive Letter (temporary)
```powershell
net use Z: \\192.168.12.136\shared /user:milton 645866 /persistent:no
```

### Map as Drive Letter (persistent across reboots)
```powershell
net use Z: \\192.168.12.136\shared /user:milton 645866 /persistent:yes
```

### Disconnect
```powershell
net use Z: /delete
```

## Connecting from Eric's Fedora Laptop

**Nautilus sidebar (permanent):** Added to `~/.config/gtk-3.0/bookmarks`:
```
smb://192.168.12.136/shared Server - Shared Files
```
Shows in the Files app left panel every time — one click to open.

**Open via terminal:**
```bash
setsid nautilus --new-window smb://192.168.12.136/shared &
```

Note: Firefox does not handle `smb://` on Linux. GNOME Fedora has no desktop icons. Use the Files app sidebar bookmark.

## How It Works for the Kids
- Files live on the server, not on the laptop
- Open, edit, and save files directly from the mapped drive or UNC path
- Close the laptop and walk away. Nothing stays on the laptop except temp files created by apps (e.g., Word creates a local temp copy while editing, then saves back to the server)
- Any device on the network can connect to the same share

## MSI Laptop Setup (Eva, .202) — Completed 2026-06-11

### Windows 11 Guest Access Fix
Windows 11 blocks unauthenticated guest access by default. Run this via SSH or on the laptop:
```
reg add "HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters" /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f
```
Requires a reboot to take effect.

### Desktop Shortcut
Created `C:\Users\eva milton\Desktop\Server - Shared Files.bat`:
```
explorer.exe \\192.168.12.136\shared
```
Double-click to open the share in File Explorer.

### Auto-Map Z: Drive on Login
Created startup script so Z: maps automatically when Eva logs in:
`C:\Users\eva milton\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\map-server.bat`
```
net use Z: \\192.168.12.136\shared /user:milton 645866 /persistent:no
```

### Accessing via Firefox
Type `\\192.168.12.136\shared` directly in the Firefox address bar — works on Windows.
Firefox bookmark policy at `C:\Program Files\Mozilla Firefox\distribution\policies.json`:
```json
{"policies":{"Bookmarks":[{"Title":"Server - Shared Files","URL":"file://192.168.12.136/shared","Placement":"toolbar"}]}}
```
Note: clicking the bookmark may not work due to Firefox security restrictions — use the address bar or desktop shortcut instead.

### Chromebook File Transfer Options
- **If ChromeOS supports SMB:** Files app > Add Network File Share > `\\192.168.12.136\shared`
- **If not:** Use Linux (Crostini) terminal: `scp files/* milton@192.168.12.136:/srv/shared/`
- **Fallback:** Copy files to USB drive, plug into ThinkCentre or MSI, copy to share

## Troubleshooting
- **Access denied from Windows:** Credentials may be cached. Run `net use \\192.168.12.136\shared /delete` then reconnect
- **Share not visible:** Check `systemctl status smb nmb` on the server
- **SELinux blocking:** `sudo setsebool -P samba_export_all_rw on` and `sudo chcon -t samba_share_t /srv/shared -R`
- **Firewall:** `sudo firewall-cmd --list-services` should show `samba`
