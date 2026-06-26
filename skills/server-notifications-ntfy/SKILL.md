# Server Notifications via ntfy

Send push notifications from the ThinkCentre to Eric's phone using [ntfy.sh](https://ntfy.sh) — no account required.

## Setup (already done)

**Phone:** ntfy app installed, subscribed to topic `MILTONHAUS-Reminders` with instant delivery enabled and battery optimization disabled.

**Server script:** `/home/milton/notify.sh`
```bash
#!/bin/bash
# Usage: ./notify.sh "Your message here"
curl -s -d "$1" ntfy.sh/MILTONHAUS-Reminders
```

## Sending a Notification

From the ThinkCentre via SSH:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no -o BatchMode=yes milton@192.168.12.136 \
  "/home/milton/notify.sh 'Your message here'"
```

From a cron job on the ThinkCentre:
```
30 20 27 6 * /home/milton/notify.sh 'Your reminder message'
```

## Adding a New Reminder

SSH to the ThinkCentre and edit crontab:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no -o BatchMode=yes milton@192.168.12.136 "crontab -e"
```

Or append directly:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no -o BatchMode=yes milton@192.168.12.136 \
  "(crontab -l 2>/dev/null; echo \"MM HH DD MON * /home/milton/notify.sh 'Message'\") | crontab -"
```

## Notes

- ntfy topics are **case-sensitive** — always use `MILTONHAUS-Reminders` exactly
- Notifications arrive even with the app closed (battery optimization must be disabled for ntfy)
- The ThinkCentre needs internet access for ntfy.sh to work
- To send from this laptop instead: `curl -s -d "message" ntfy.sh/MILTONHAUS-Reminders`
