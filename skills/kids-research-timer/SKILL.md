# Kids Research Timer — Timed Pi-hole Unrestrict

**Last Updated:** 2026-06-23
**Status:** Live — YTI Chromebook daily 7-8:30pm cron active. Eva (.202) + Benedict (.239) one-time 1pm-4pm window set for 2026-06-24.

---

## Overview

Lifts Pi-hole restrictions on all kids' devices during a research window, then restores them automatically.
All timers run on the ThinkCentre via `at` or cron — survives Claude session closure.

**ThinkCentre:** `milton@192.168.12.136`
**Pi-hole DB:** `docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db`

> **CRITICAL (2026-06-23):** ThinkCentre SSH now requires the `-tt` flag — it hangs silently without it. All SSH commands to ThinkCentre must use `ssh -tt -i ~/.ssh/id_ed25519 ...`

---

## Kids Device Map (Pi-hole Client IDs + Groups)

| Device | IP | Client ID | Group ID | Group Name |
|--------|----|-----------|----------|------------|
| Mac Mini | 192.168.12.163 | 1 | 1 | mac-mini |
| Patrick's schoolwork laptop | 192.168.12.249 | 2 | 2 | kids1 |
| Benedict's laptop | 192.168.12.239 | 3 | 3 | kids2 |
| YTI Chromebook | 192.168.12.219 | 11 | 7 | tower-of-gondor |
| Ev's Chromebook | 192.168.12.194 | 8 | 6 | ev-chromebook |
| Tower of Gondor | 192.168.12.160 | 9 | 7 | tower-of-gondor |
| Gianna's laptop | 192.168.12.226 | 12 | 8 | gianna-laptop |
| Eva's laptop | 192.168.12.202 | 13 | 9 | eva-laptop |

---

## One-Shot: Open Now, Close at a Specific Time

### Step 1 — Open all kids' devices now
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 \
  'docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM client_by_group WHERE client_id IN (1,2,3,8,9,11,12,13);" && docker exec pihole pihole reloaddns'
```

### Step 1b — Flush DNS on all Windows devices (REQUIRED after opening)

Without this, devices still have cached blocked entries — google.com loads but links don't work.

```python
import subprocess, pexpect

# Eva's laptop — key auth (username has a space, must be quoted)
subprocess.run([
    'ssh', '-i', '/home/ericmilton/.ssh/id_ed25519',
    '-o', 'StrictHostKeyChecking=no',
    'eva milton@192.168.12.202', 'ipconfig /flushdns'
])

# Patrick's laptop (.249) and Benedict's laptop (.239) — password auth
for ip in ['192.168.12.249', '192.168.12.239']:
    child = pexpect.spawn(f'ssh -o StrictHostKeyChecking=no themi@{ip}')
    child.expect('password:')
    child.sendline('1229')
    child.expect(r'\$|>')
    child.sendline('ipconfig /flushdns')
    child.expect(r'\$|>')
    print(child.before.decode())
    child.close()
```

Or as a one-liner for Eva's laptop only:
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no "eva milton@192.168.12.202" 'ipconfig /flushdns'
```

### Step 2 — Schedule restore at a specific time (e.g. 3pm)
```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 "echo \"docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \\\"INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (1,1),(2,2),(3,3),(8,6),(9,7),(11,7),(12,8),(13,9);\\\" && docker exec pihole pihole reloaddns\" | at 3pm"
```

---

## One-Time Future Window (Specific Date + Time)

Use this when you want a window to open and close on a specific future date — e.g. "1pm to 4pm tomorrow." Uses cron with a full `minute hour day month *` spec.

### Step 1 — SSH to ThinkCentre and add the two cron entries
```bash
timeout 20 ssh -tt -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 "
(crontab -l 2>/dev/null; echo '';
 echo '# Eva + Benedict unrestricted 1-4pm 2026-06-24';
 echo '0 13 24 6 * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"DELETE FROM client_by_group WHERE client_id IN (13,3); INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (13,6),(3,6);\" && docker exec pihole pihole reloaddns # open';
 echo '0 16 24 6 * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"DELETE FROM client_by_group WHERE client_id IN (13,3); INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (13,9),(3,3);\" && docker exec pihole pihole reloaddns # close') | crontab -
echo DONE
" 2>&1
```

**Cron date format:** `minute hour day month *` — e.g. `0 13 24 6 *` = 1:00pm on June 24.

**Important:** The "open" command moves devices to group 6 (ev-temp-unrestricted) rather than using `groups:[]`. This is intentional — group 6 has no adlists and only 4 cosmetic deny rules (Google Fonts, rhymezone), making it effectively unrestricted while keeping devices in a named group. The "close" command restores each device to its original group.

### Verify cron was added
```bash
timeout 20 ssh -tt -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 "crontab -l | tail -5" 2>&1
```

### If you need to open them immediately AND restore via cron
Open both devices now via Pi-hole API (see API section below), then add only the close cron entry.

### Remove one-time cron entries after they've fired
```bash
timeout 20 ssh -tt -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 "crontab -l | grep -v '# open\|# close' | crontab -" 2>&1
```

---

## Recurring Daily Research Window (Cron)

Add to ThinkCentre crontab to open at 1pm and close at 3pm every day:

```bash
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136 '
(crontab -l 2>/dev/null; echo "0 13 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"DELETE FROM client_by_group WHERE client_id IN (1,2,3,8,9,11,12,13);\" && docker exec pihole pihole reloaddns") | crontab -
(crontab -l 2>/dev/null; echo "0 15 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db \"INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (1,1),(2,2),(3,3),(8,6),(9,7),(11,7),(12,8),(13,9);\" && docker exec pihole pihole reloaddns") | crontab -
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
  'docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (1,1),(2,2),(3,3),(8,6),(9,7),(11,7),(12,8),(13,9);" && docker exec pihole pihole reloaddns'
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

## Pi-hole API Method (Manual, Per-Device)

**Preferred for one-off manual use.** No SSH to ThinkCentre needed — runs directly from Eric's terminal.

> `groups:[]` = fully open (no rules), `groups:[N]` = locked back to assigned group

### Step 1 — Get a session ID (run once per session)
```bash
SID=$(curl -s -X POST http://192.168.12.136/api/auth \
  -H "Content-Type: application/json" \
  -d '{"password":"645866"}' | python3 -c \
  "import sys,json; print(json.load(sys.stdin)['session']['sid'])")
```

### Per-Device Lift / Restore

| Device | IP | Group |
|--------|----|-------|
| Patrick's laptop | 192.168.12.249 | 2 |
| Benedict's laptop | 192.168.12.239 | 3 |
| Eva's laptop | 192.168.12.202 | 9 |
| Gianna's laptop | 192.168.12.226 | 8 |
| Tower of Gondor | 192.168.12.160 | 7 |
| YTI Chromebook | 192.168.12.219 | 7 |
| Ev's Chromebook | 192.168.12.194 | 6 |
| Mac Mini | 192.168.12.163 | 1 |

Replace `<IP>` and `<GROUP>` below:

**LIFT:**
```bash
curl -s -X PUT http://192.168.12.136/api/clients/<IP> \
  -H "sid: $SID" -H "Content-Type: application/json" \
  -d '{"groups":[]}'
```

**RESTORE:**
```bash
curl -s -X PUT http://192.168.12.136/api/clients/<IP> \
  -H "sid: $SID" -H "Content-Type: application/json" \
  -d '{"groups":[<GROUP>]}'
```

### Lift ALL Devices at Once
```bash
for IP in 192.168.12.249 192.168.12.239 192.168.12.202 \
          192.168.12.226 192.168.12.160 192.168.12.219 \
          192.168.12.194 192.168.12.163; do
  curl -s -X PUT http://192.168.12.136/api/clients/$IP \
    -H "sid: $SID" -H "Content-Type: application/json" \
    -d '{"groups":[]}'
done
```

### Restore ALL Devices at Once
```bash
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.249 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[2]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.239 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[3]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.202 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[9]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.226 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[8]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.160 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[7]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.219 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[7]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.194 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[6]}'
curl -s -X PUT http://192.168.12.136/api/clients/192.168.12.163 -H "sid: $SID" -H "Content-Type: application/json" -d '{"groups":[1]}'
```

### After Lifting — Flush DNS on Windows Laptops
```bash
ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" 'ipconfig /flushdns'
```
Patrick, Benedict, Gianna: run `ipconfig /flushdns` in their own cmd terminal.

> **WARNING:** `groups:[0]` is NOT unrestricted — group 0 is Pi-hole's default group and still blocks most sites. Use `groups:[]` (empty) to fully open a device, OR `groups:[6]` to put it in the `ev-temp-unrestricted` group (no adlists, only 4 cosmetic deny rules — effectively open and preferred for kids devices).

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
- **YTI Chromebook** is .219, client_id 11, group 7. Static IP set on device 2026-06-18 — will not drift
- **YTI daily research window:** 7pm-8:30pm every day via cron on ThinkCentre (updated 2026-06-18)
- **DNS TTL cap:** `max-cache-ttl=60` set in `/etc/dnsmasq.d/99-custom.conf` inside the Pi-hole container (added 2026-06-17). Caps all forwarded DNS responses to 60-second TTL so devices must re-query Pi-hole within a minute. Without this, devices cache IPs from the open window and keep browsing after restrictions are restored. Sites will stop loading within ~60 seconds of the close cron running.

---

## Active Cron Timers

| Device | Open | Close | Added |
|--------|------|-------|-------|
| YTI Chromebook (client 11, .219) | 7:00pm daily | 8:30pm daily | 2026-06-18 |
| Eva (.202) + Benedict (.239) | 1:00pm 2026-06-24 (one-time) | 4:00pm 2026-06-24 (one-time) | 2026-06-23 |

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
