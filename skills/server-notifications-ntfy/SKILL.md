# Server Notifications via ntfy

Two-way push notifications using [ntfy.sh](https://ntfy.sh) — no account required.

## Topics

| Direction | Topic | Purpose |
|-----------|-------|---------|
| Server/Laptop → Phone | `MILTONHAUS-Reminders` | Reminders, alerts from ThinkCentre or laptop |
| Phone → Laptop | `MILTONHAUS-Laptop` | Messages from Eric's phone to this laptop |

## Phone → Laptop (Listener)

**Listener script:** `C:\Users\ericm\ntfy-listener.ps1`
- Streams messages from `MILTONHAUS-Laptop` topic
- Shows a dark always-on-top popup in the bottom-right corner that **stays on screen until clicked**
- Auto-starts on login via startup shortcut
- Reconnects automatically if connection drops
- If laptop is asleep, queued messages appear when it wakes (ntfy retains messages for 12 hours)

**Phone setup:** In the ntfy app, subscribe to `MILTONHAUS-Laptop`. Tap into the topic and use the publish button to send a message.

**Auto-start location:** `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\ntfy-listener.lnk`

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
