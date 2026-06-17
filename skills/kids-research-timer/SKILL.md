# Kids Research Timer — Timed Pi-hole Unrestrict

**Last Updated:** 2026-06-16
**Status:** Ready to use

---

## Overview

Lifts Pi-hole restrictions on all kids' devices during a research window, then restores them automatically.
All timers run on the ThinkCentre via `at` or cron — survives Claude session closure.

**ThinkCentre:** `milton@192.168.12.136`
**Pi-hole DB:** `docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db`

---

## Kids Device Map (Pi-hole Client IDs + Groups)

| Device | IP | Client ID | Group ID | Group Name |
|--------|----|-----------|----------|------------|
| Mac Mini | 192.168.12.163 | 1 | 1 | mac-mini |
| Kids1 laptop | 192.168.12.249 | 2 | 2 | kids1 |
| Kids2 laptop | 192.168.12.239 | 3 | 3 | kids2 |
| Patrick's Chromebook | 192.168.12.221 | 5 | 4 | patricks-chromebook |
| Ev's Chromebook | 192.168.12.194 | 8 | 6 | ev-chromebook |
| Tower of Gondor | 192.168.12.160 | 9 | 7 | tower-of-gondor |
| YTI Chromebook | 192.168.12.221 | 5 | 4 | patricks-chromebook |
| YTI Chromebook (drift .220) | 192.168.12.220 | 4 | 4 | patricks-chromebook |
| YTI Chromebook (drift .219) | 192.168.12.219 | 11 | 7 | tower-of-gondor |
| Gianna's laptop | 192.168.12.226 | 12 | 8 | gianna-laptop |
| Eva's laptop | 192.168.12.202 | 13 | 9 | eva-laptop |

---

## One-Shot: Open Now, Close at a Specific Time

### Step 1 — Open all kids' devices now
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM client_by_group WHERE client_id IN (1,2,3,5,8,9,11,12,13);" && docker exec pihole pihole reloaddns'
```

### Step 2 — Schedule restore at a specific time (e.g. 3pm)
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 "echo \"docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \\\"INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (1,1),(2,2),(3,3),(5,4),(8,6),(9,7),(11,7),(12,8),(13,9);\\\" && docker exec pihole pihole reloaddns\" | at 3pm"
```

---

## Recurring Daily Research Window (Cron)

Add to ThinkCentre crontab to open at 1pm and close at 3pm every day:

```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 '
(crontab -l 2>/dev/null; echo "0 13 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"DELETE FROM client_by_group WHERE client_id IN (1,2,3,5,8,9,11,12,13);\" && docker exec pihole pihole reloaddns") | crontab -
(crontab -l 2>/dev/null; echo "0 15 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (1,1),(2,2),(3,3),(5,4),(8,6),(9,7),(11,7),(12,8),(13,9);\" && docker exec pihole pihole reloaddns") | crontab -
'
```

View the crontab to confirm:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 'crontab -l'
```

Remove research window cron entries:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'crontab -l | grep -v "client_by_group.*client_id IN" | crontab -'
```

---

## Manual Restore (Close Immediately)

If you need to lock everything down right now:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (1,1),(2,2),(3,3),(5,4),(8,6),(9,7),(11,7),(12,8),(13,9);" && docker exec pihole pihole reloaddns'
```

---

## Single Device Override

To open/close one device only (e.g. just Gianna, client_id 12, group 8):

Open:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM client_by_group WHERE client_id = 12;" && docker exec pihole pihole reloaddns'
```

Close (timed via `at`):
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'echo "docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (12, 8);\" && docker exec pihole pihole reloaddns" | at 11pm'
```

---

## How the Commands Work

Each command has two parts:

### Part 1 — SSH connection (connects to ThinkCentre)
```
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136
```
- `ssh` — remote login command
- `-i ~/.ssh/id_ed25519` — uses your SSH key (no password needed)
- `-o StrictHostKeyChecking=no` — skips the "are you sure?" prompt
- `milton@192.168.12.136` — logs in as user `milton` on the ThinkCentre

### Part 2 — Pi-hole command (runs on ThinkCentre after connecting)

This is the quoted portion after the SSH line. It talks to the Pi-hole database inside Docker to add or remove group restrictions on the kids' devices, then reloads DNS so the change takes effect.

- **Opening** (DELETE) — removes devices from their restricted groups, giving full internet access
- **Closing** (INSERT OR IGNORE) — puts devices back into their restricted groups, restoring blocks
- **Timed close** (`| at 3pm`) — schedules the restore command to run later so you don't have to remember

The SSH part is the delivery truck, the quoted part is the package.

---

## Notes

- Removing a client from all groups = full unrestricted access (no group = no rules applied)
- Restoring uses `INSERT OR IGNORE` — safe to run even if already in the group
- `atq` on ThinkCentre shows pending jobs; `atrm <id>` cancels them
- After any Pi-hole DNS change, flush DNS on devices that are actively browsing: `ipconfig /flushdns` on Windows
- **YTI Chromebook has 3 client entries** (IPs .219, .220, .221) from DHCP drift. Must unrestrict/restore ALL THREE (client_id IN (4,5,11)) or it stays blocked
