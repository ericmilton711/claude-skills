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

## How It Works for the Kids
- Files live on the server, not on the laptop
- Open, edit, and save files directly from the mapped drive or UNC path
- Close the laptop and walk away. Nothing stays on the laptop except temp files created by apps (e.g., Word creates a local temp copy while editing, then saves back to the server)
- Any device on the network can connect to the same share

## TODO: MSI Laptop Setup
When setting up the MSI laptop for the daughter:
1. SSH into the MSI laptop
2. Map `\\192.168.12.136\shared` as a persistent drive letter
3. Save credentials so she doesn't get prompted every time
4. Optionally create a subfolder per kid (e.g., `/srv/shared/gianna/`) for organization
5. Transfer files from the Chromebook to `/srv/shared/` (via SCP, SFTP, or USB drive)

### Chromebook File Transfer Options
- **If ChromeOS supports SMB:** Files app > Add Network File Share > `\\192.168.12.136\shared`
- **If not:** Use Linux (Crostini) terminal: `scp files/* milton@192.168.12.136:/srv/shared/`
- **Fallback:** Copy files to USB drive, plug into ThinkCentre or MSI, copy to share

## Troubleshooting
- **Access denied from Windows:** Credentials may be cached. Run `net use \\192.168.12.136\shared /delete` then reconnect
- **Share not visible:** Check `systemctl status smb nmb` on the server
- **SELinux blocking:** `sudo setsebool -P samba_export_all_rw on` and `sudo chcon -t samba_share_t /srv/shared -R`
- **Firewall:** `sudo firewall-cmd --list-services` should show `samba`
