# Server Notifications via ntfy

Two-way push notifications using [ntfy.sh](https://ntfy.sh) — no account required.

## Topics

| Direction | Topic | Purpose |
|-----------|-------|---------|
| Server/Laptop → Phone | `MILTONHAUS-Reminders` | Reminders, alerts from ThinkCentre or laptop |
| Phone → All Devices | `MILTONHAUS-Laptop` | Messages from Eric's phone to all computers |

## Devices — Listener Deployment

| Device | IP | OS | SSH Auth | Status |
|--------|----|----|----------|--------|
| Eric's laptop | .220 | Windows | — | **DEPLOYED** |
| Tower of Gondor | .160 | Windows | Password (`user` / `645866`) | PENDING |
| Eva's MSI | .202 | Windows | SSH key | PENDING |
| Gianna's Acer | .226 | Fedora | SSH key | PENDING |
| Patrick's laptop | .249 | Windows | Password (`themi` / `1229`) | PENDING |
| Benedict's laptop | .239 | Windows | Password (`themi` / `1229`) | PENDING |

## Phone → Devices (Listener)

All devices subscribe to the same `MILTONHAUS-Laptop` topic. One message from the phone pops up on every screen.

### Windows Listener — `ntfy-listener.ps1`

Shows a dark always-on-top popup in the bottom-right corner that **stays on screen until clicked to dismiss**.

**Script location (Eric's laptop):** `C:\Users\ericm\ntfy-listener.ps1`
**Auto-start:** Startup shortcut at `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\ntfy-listener.lnk`

```powershell
# Startup shortcut creation (run once per machine):
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut("$env:APPDATA\Microsoft\Windows\Start Menu\Programs\Startup\ntfy-listener.lnk")
$shortcut.TargetPath = "powershell.exe"
$shortcut.Arguments = "-WindowStyle Hidden -ExecutionPolicy Bypass -File `"$env:USERPROFILE\ntfy-listener.ps1`""
$shortcut.WorkingDirectory = $env:USERPROFILE
$shortcut.Save()
```

**Deploy to a Windows machine via SCP + SSH:**
```bash
scp ~/ntfy-listener.ps1 USER@IP:C:/Users/USER/ntfy-listener.ps1
ssh USER@IP "powershell -Command \"$shell = New-Object -ComObject WScript.Shell; $sc = $shell.CreateShortcut('$env:APPDATA\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\ntfy-listener.lnk'); $sc.TargetPath = 'powershell.exe'; $sc.Arguments = '-WindowStyle Hidden -ExecutionPolicy Bypass -File C:\\Users\\USER\\ntfy-listener.ps1'; $sc.WorkingDirectory = '$env:USERPROFILE'; $sc.Save()\""
```

### Linux Listener — `ntfy-listener.sh`

Uses `notify-send -u critical -t 0` so notifications persist until dismissed (on GNOME/XFCE).

**Script location (Eric's laptop, for deployment):** `C:\Users\ericm\ntfy-listener.sh`

**Deploy to a Linux machine:**
```bash
scp ~/ntfy-listener.sh USER@IP:~/ntfy-listener.sh
ssh USER@IP "chmod +x ~/ntfy-listener.sh"
# Create autostart entry:
ssh USER@IP "mkdir -p ~/.config/autostart && cat > ~/.config/autostart/ntfy-listener.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=ntfy Listener
Exec=/home/USER/ntfy-listener.sh
Hidden=false
X-GNOME-Autostart-enabled=true
EOF"
```

### Behavior

- Reconnects automatically if connection drops
- If a device is asleep/off, messages queue on ntfy's server for 12 hours and appear when the listener reconnects
- Multiple popups can stack if several messages arrive before being dismissed

## Phone Setup

In the ntfy app, subscribe to `MILTONHAUS-Laptop` (case-sensitive). Tap into the topic and use the publish button to send.

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
