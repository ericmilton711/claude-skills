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

## MSI Laptop Setup (Eva, .202) — Updated 2026-10-28

### SMB Client Fix (CRITICAL — Windows 24H2)
Windows 11 24H2 defaults `RequireSecuritySignature = True`, which breaks connections to Samba. Must be set to False:
```powershell
Set-SmbClientConfiguration -RequireSecuritySignature $false -EnableInsecureGuestLogons $false -Confirm:$false
```
This matches Eric's Windows laptop settings. Applied 2026-10-28.

### Windows 11 Guest Access Fix
Windows 11 blocks unauthenticated guest access by default. Run this via SSH or on the laptop:
```
reg add "HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters" /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f
```
Requires a reboot to take effect.

### Credential Storage
Credentials stored in Windows Credential Manager so UNC paths don't prompt for passwords:
```
cmdkey /add:192.168.12.136 /user:milton /pass:645866
```
**IMPORTANT:** `cmdkey` cannot run from an SSH session ("Credentials cannot be saved from this logon session"). Must be run in Eva's interactive desktop session. Use a scheduled task:
```powershell
$action = New-ScheduledTaskAction -Execute "cmd.exe" -Argument "/c cmdkey /add:192.168.12.136 /user:milton /pass:645866"
$principal = New-ScheduledTaskPrincipal -UserId "eva milton" -LogonType Interactive
Register-ScheduledTask -TaskName "StoreCreds" -Action $action -Principal $principal -Force
Start-ScheduledTask -TaskName "StoreCreds"
Start-Sleep 5
Unregister-ScheduledTask -TaskName "StoreCreds" -Confirm:$false
```

### Auto-Map Z: Drive on Login
Startup script maps Z: and stores credentials every login:
`C:\Users\eva milton\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\map-server.bat`
```
@echo off
cmdkey /add:192.168.12.136 /user:milton /pass:645866
net use Z: \\192.168.12.136\shared /user:milton 645866 /persistent:no
```
Z: drive also mapped with `/persistent:yes` as backup.

### Desktop Path (GOTCHA)
Eva's real desktop is `C:\Users\Eva Milton\OneDrive\Desktop`, NOT `C:\Users\eva milton\Desktop`.
Always use `[Environment]::GetFolderPath('Desktop')` to find the real path.

### SSH vs Desktop Session (GOTCHA)
SSH commands run in a separate Windows logon session. `net use` and `cmdkey` from SSH are invisible to Explorer. Use scheduled tasks with `LogonType Interactive` for anything that needs to affect the desktop.

### UNC Path Escaping Through SSH (GOTCHA)
Backslashes get eaten through SSH to cmd.exe. `\\\\server\\share` arrives as `\server\share`. Fix: write a .bat or .ps1 file via SCP or `[IO.File]::WriteAllText()`, then execute the file remotely.

### Accessing via Firefox
Type `\\192.168.12.136\shared` directly in the Firefox address bar — works on Windows.
Firefox bookmark policy at `C:\Program Files\Mozilla Firefox\distribution\policies.json`:
```json
{"policies":{"Bookmarks":[{"Title":"Server - Shared Files","URL":"file://192.168.12.136/shared","Placement":"toolbar"}]}}
```
Note: clicking the bookmark may not work due to Firefox security restrictions — use the address bar instead.

### Chromebook File Transfer Options
- **If ChromeOS supports SMB:** Files app > Add Network File Share > `\\192.168.12.136\shared`
- **If not:** Use Linux (Crostini) terminal: `scp files/* milton@192.168.12.136:/srv/shared/`
- **Fallback:** Copy files to USB drive, plug into ThinkCentre or MSI, copy to share

## Samba Server Config (ThinkCentre)
`server signing = mandatory` added to `[global]` in `/etc/samba/smb.conf` on 2026-10-28.

## Troubleshooting
- **Error 67 "network name cannot be found":** Check `RequireSecuritySignature` on the Windows client. Set to `$false` if it's `$true`.
- **Credential prompt on UNC paths:** Run `cmdkey /add:192.168.12.136 /user:milton /pass:645866` in the user's INTERACTIVE session (not SSH). Use scheduled task method above.
- **Z: drive shows "Unavailable":** `net use Z: /delete /y` then remap. If remapping from SSH, it won't show on the desktop — use scheduled task or startup script.
- **Access denied from Windows:** `net use \\192.168.12.136\shared /delete` then reconnect
- **Share not visible:** Check `systemctl status smb nmb` on the server
- **SELinux blocking:** `sudo setsebool -P samba_export_all_rw on` and `sudo chcon -t samba_share_t /srv/shared -R`
- **Firewall:** `sudo firewall-cmd --list-services` should show `samba`
