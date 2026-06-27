# Server Notifications via ntfy

Two-way push notifications using [ntfy.sh](https://ntfy.sh) — no account required.

## Topics

| Topic | Direction | Device |
|-------|-----------|--------|
| `MILTONHAUS-Reminders` | Server/Laptop → Phone | Eric's phone |
| `MILTONHAUS-Laptop` | Phone → Device | Eric's laptop (.220) |
| `MILTONHAUS-Tower` | Phone → Device | Tower of Gondor (.160) |
| `MILTONHAUS-Eva` | Phone → Device | Eva's MSI (.202) |
| `MILTONHAUS-Gianna` | Phone → Device | Gianna's Acer (.226) |
| `MILTONHAUS-Patrick` | Phone → Device | Patrick's laptop (.249) |
| `MILTONHAUS-Benedict` | Phone → Device | Benedict's laptop (.239) |

**Phone setup:** Subscribe to each topic in the ntfy app. Publish to whichever device you want to reach.

## Devices — Listener Deployment

| Device | IP | OS | SSH Auth | Topic | Status |
|--------|----|----|----------|-------|--------|
| Eric's laptop | .220 | Windows | — | `MILTONHAUS-Laptop` | **DEPLOYED** |
| Tower of Gondor | .160 | Windows | Password (`user` / `645866`) | `MILTONHAUS-Tower` | PENDING |
| Eva's MSI | .202 | Windows | SSH key | `MILTONHAUS-Eva` | PENDING |
| Gianna's Acer | .226 | Fedora | SSH key | `MILTONHAUS-Gianna` | PENDING |
| Patrick's laptop | .249 | Windows | Password (`themi` / `1229`) | `MILTONHAUS-Patrick` | PENDING |
| Benedict's laptop | .239 | Windows | Password (`themi` / `1229`) | `MILTONHAUS-Benedict` | PENDING |

## Phone → Devices (Listener)

Each device runs a listener on its own topic. The scripts accept the topic as a parameter.

### Windows Listener — `ntfy-listener.ps1`

Shows a dark always-on-top popup in the bottom-right corner that **stays on screen until clicked to dismiss**.

**Script location (Eric's laptop):** `C:\Users\ericm\ntfy-listener.ps1`
**Usage:** `powershell -File ntfy-listener.ps1 -Topic MILTONHAUS-Tower`
**Default topic (no param):** `MILTONHAUS-Laptop`

**Auto-start:** Startup shortcut at `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\ntfy-listener.lnk`

```powershell
# Startup shortcut creation (run once per machine, set -Topic to match the device):
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut("$env:APPDATA\Microsoft\Windows\Start Menu\Programs\Startup\ntfy-listener.lnk")
$shortcut.TargetPath = "powershell.exe"
$shortcut.Arguments = "-WindowStyle Hidden -ExecutionPolicy Bypass -File `"$env:USERPROFILE\ntfy-listener.ps1`" -Topic MILTONHAUS-DEVICENAME"
$shortcut.WorkingDirectory = $env:USERPROFILE
$shortcut.Save()
```

### Linux Listener — `ntfy-listener.sh`

Uses `notify-send -u critical -t 0` so notifications persist until dismissed (on GNOME/XFCE).

**Script location (Eric's laptop, for deployment):** `C:\Users\ericm\ntfy-listener.sh`
**Usage:** `./ntfy-listener.sh MILTONHAUS-Gianna`
**Default topic (no arg):** `MILTONHAUS-Laptop`

**Deploy to a Linux machine:**
```bash
scp ~/ntfy-listener.sh USER@IP:~/ntfy-listener.sh
ssh USER@IP "chmod +x ~/ntfy-listener.sh"
ssh USER@IP "mkdir -p ~/.config/autostart && cat > ~/.config/autostart/ntfy-listener.desktop << 'DEOF'
[Desktop Entry]
Type=Application
Name=ntfy Listener
Exec=/home/USER/ntfy-listener.sh MILTONHAUS-DEVICENAME
Hidden=false
X-GNOME-Autostart-enabled=true
DEOF"
```

### Behavior

- Reconnects automatically if connection drops
- If a device is asleep/off, messages queue on ntfy's server for 12 hours and appear when the listener reconnects
- Multiple popups can stack if several messages arrive before being dismissed

## Server/Laptop → Phone

**Server script:** `/home/milton/notify.sh`
```bash
#!/bin/bash
# Usage: ./notify.sh "Your message here"
curl -s -d "$1" ntfy.sh/MILTONHAUS-Reminders
```

From the ThinkCentre via SSH:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no -o BatchMode=yes milton@192.168.12.136 \
  "/home/milton/notify.sh 'Your message here'"
```

From this laptop directly:
```bash
curl -s -d "message" ntfy.sh/MILTONHAUS-Reminders
```

From a cron job on the ThinkCentre:
```
30 20 27 6 * /home/milton/notify.sh 'Your reminder message'
```

## Adding a New Reminder

Append to ThinkCentre crontab:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no -o BatchMode=yes milton@192.168.12.136 \
  "(crontab -l 2>/dev/null; echo \"MM HH DD MON * /home/milton/notify.sh 'Message'\") | crontab -"
```

## Notes

- ntfy topics are **case-sensitive**
- Phone: battery optimization must be disabled for ntfy app
- ThinkCentre needs internet access for ntfy.sh to work
