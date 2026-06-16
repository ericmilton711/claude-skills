# Server-Side LED Shutoff Timer

**Last Updated:** 2026-06-16
**Status:** SSH key setup required before use (see Setup section)

---

## Overview

Schedule the barn LEDs to shut off at a specific time using `at` on the ThinkCentre.
ThinkCentre SSHes to the Pi and runs `homestead.py leds-off`.
Survives Claude session closure — runs on the server regardless.

**LED command on Pi:** `python3 /home/eric/homestead.py leds-off`
**Pi SSH:** `eric@192.168.12.114` (requires `-tt` flag)

---

## One-Shot Timer (Tonight)

```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'echo "ssh -i /home/milton/.ssh/id_ed25519 -o StrictHostKeyChecking=no -tt eric@192.168.12.114 python3 /home/eric/homestead.py leds-off" | at 11pm'
```

Check it was scheduled:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'atq'
```

Cancel a scheduled job (replace `<JOB_ID>` with number from atq):
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'atrm <JOB_ID>'
```

---

## Recurring Cron (Every Night)

Add a cron entry on the ThinkCentre to shut off LEDs every night at 11pm:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  '(crontab -l 2>/dev/null; echo "0 23 * * * ssh -i /home/milton/.ssh/id_ed25519 -o StrictHostKeyChecking=no -tt eric@192.168.12.114 python3 /home/eric/homestead.py leds-off") | crontab -'
```

View the ThinkCentre crontab:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'crontab -l'
```

Remove the LED cron entry:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'crontab -l | grep -v "homestead.py leds-off" | crontab -'
```

---

## Setup — ThinkCentre SSH Key to Pi

This must be done once before the timer works.

### Step 1 — Generate a key on the ThinkCentre (if none exists)
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'ls ~/.ssh/id_ed25519 2>/dev/null || ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519'
```

### Step 2 — Get the ThinkCentre's public key
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'cat ~/.ssh/id_ed25519.pub'
```

### Step 3 — Add it to the Pi's authorized_keys
```bash
PUBKEY=$(ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'cat ~/.ssh/id_ed25519.pub')
ssh -i ~/.ssh/id_ed25519 -tt -o StrictHostKeyChecking=no eric@192.168.12.114 \
  "mkdir -p ~/.ssh && echo '$PUBKEY' >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys"
```

### Step 4 — Test it
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'ssh -i /home/milton/.ssh/id_ed25519 -o StrictHostKeyChecking=no -tt eric@192.168.12.114 echo "SSH from ThinkCentre to Pi works"'
```

---

## Notes

- Pi requires `-tt` flag (forced TTY) or SSH commands hang silently
- The Pi already has a Pi-side cron (`0 23 * * *`) for leds-off — the ThinkCentre timer is an *additional* or *replacement* option. Don't double-schedule without removing one.
- `atq` lists pending jobs; `atrm <id>` cancels them
- All `at` jobs run as the `milton` user on the ThinkCentre
