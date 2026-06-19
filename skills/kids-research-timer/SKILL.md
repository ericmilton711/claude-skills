# Kids Research Timer — Timed Pi-hole Unrestrict

**Last Updated:** 2026-06-17
**Status:** Live — YTI Chromebook daily 7-8:30pm cron active, updated 2026-06-18

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
| Patrick's schoolwork laptop | 192.168.12.249 | 2 | 2 | kids1 |
| Kids2 laptop | 192.168.12.239 | 3 | 3 | kids2 |
| YTI Chromebook (stale — old IP .221) | 192.168.12.221 | 5 | 4 | patricks-chromebook |
| YTI Chromebook (current — .219) | 192.168.12.219 | 11 | 7 | tower-of-gondor |
| Ev's Chromebook | 192.168.12.194 | 8 | 6 | ev-chromebook |
| Tower of Gondor | 192.168.12.160 | 9 | 7 | tower-of-gondor |
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
- **YTI Chromebook** current IP is .219, client_id 11, group 7. Client 5 (.221) is a stale DHCP drift entry — do NOT use for cron
- **YTI daily research window:** 7pm-8:30pm every day via cron on ThinkCentre (updated 2026-06-18)
- **DNS TTL cap:** `max-cache-ttl=60` set in `/etc/dnsmasq.d/99-custom.conf` inside the Pi-hole container (added 2026-06-17). Caps all forwarded DNS responses to 60-second TTL so devices must re-query Pi-hole within a minute. Without this, devices cache IPs from the open window and keep browsing after restrictions are restored. Sites will stop loading within ~60 seconds of the close cron running.

---

## Active Cron Timers

| Device | Open | Close | Added |
|--------|------|-------|-------|
| YTI Chromebook (client 11, .219) | 7:00pm daily | 8:30pm daily | 2026-06-18 |

Cron entries on ThinkCentre (`crontab -l`):
```
0 19 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM client_by_group WHERE client_id = 11;" && docker exec pihole pihole reloaddns # YTI research open
30 20 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (11,7);" && docker exec pihole pihole reloaddns # YTI research close
```

To remove:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'crontab -l | grep -v "YTI research" | crontab -'
```

---

## Testing Log

| Date | Test | Result |
|------|------|--------|
| 2026-06-17 | 2-minute one-shot unrestrict (client 5 only) | Worked. Google loaded. Restrictions restored on schedule. |
| 2026-06-17 | 7-8pm daily cron | Open worked. Close restored DB group but sites stayed accessible due to DNS caching. Fixed with `max-cache-ttl=60`. |
