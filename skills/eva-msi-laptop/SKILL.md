# Eva's Laptop — MSI Modern 14 A10M-1029US

**Last Updated:** 2026-06-12
**Status:** Fully set up. RAM upgrade ordered (16GB, not yet installed).

---

## Hardware

| Component | Spec |
|-----------|------|
| Model | MSI Modern 14 A10M-1029US |
| CPU | Intel i5-10210U (10th Gen) |
| RAM (stock) | 8GB DDR4 |
| RAM (upgrade) | 16GB DDR4 2666MHz SO-DIMM (260-pin) — pending install |
| RAM slots | 2x SO-DIMM (max 32GB total) |
| RAM type note | NOT soldered — fully upgradeable |
| OS | Windows 11 24H2 (build 10.0.26100) |

### RAM Upgrade Instructions

1. Power off, unplug, flip laptop over
2. Remove all bottom panel screws (Phillips head)
3. Pry bottom panel off from rear edge with spudger or old credit card
4. SO-DIMM slot visible on motherboard
5. Spread metal clips on sides → stick pops up at 45°
6. Slide out old stick, insert new one at 45°, press down until clips snap
7. Reassemble

**Reference video (exact model):** https://www.youtube.com/watch?v=HVPVNzX6JFw

---

## Network / Identity

- **Hostname:** Eva
- **IP:** 192.168.12.202 (WiFi, MILTONHAUS)
- **User:** `eva milton` *(note the space — quote it in SSH)*
- **Password:** 645866
- **Role:** Administrator

---

## SSH Access

Key auth works passwordless:

```bash
ssh "eva milton"@192.168.12.202
```

Key installed at `C:\ProgramData\ssh\administrators_authorized_keys` (admin accounts ignore per-user authorized_keys on Windows).

**Gotcha:** After major Windows updates, sshd may stop. May need to start it from the laptop: `Start-Service sshd` or Services app.

---

## Pi-hole

- **Group:** 9 (`eva-laptop`) — default-deny
- **Allowed:** Homeschool Connections, Teaching Textbooks, Duolingo, Kiddle, Vimeo, Zoom, Firefox/Mozilla, Windows essentials, Gmail, Google Chat, Google Docs/Drive/Sheets, Britannica
- **Blocked:** Google Search (google.com/www.google.com), YouTube, DuckDuckGo, Bing, and every other search engine
- **pki.goog** must stay whitelisted or Gmail TLS breaks (cert revocation)
- **STANDING POLICY (set 2026-07-25, permanent until Eric says otherwise):** Chat + Gmail + Docs/Drive/Sheets allowed, all search engines + YouTube blocked. Do not revert.

### Google Chat + Gmail — domains required (added 2026-07-25)

Chat was originally banned (deny rules 271-274), then explicitly re-allowed for group 9 only. Allow-regex rules now scoped to group 9 (in addition to pre-existing gmail infra: `googleapis.com`, `gstatic.com`, `googleusercontent.com`, `accounts.google.com`, `apis.google.com`, `ogs.google.com`, `pki.goog`, `workspace.google.com`):

| Domain | Rule ID | Notes |
|--------|---------|-------|
| `gmail.com` | 251 | added group 9 |
| `mail.google.com` | 250 | added group 9 |
| `chat.google.com` | 258 | added group 9 — overrides old deny rule 271 (allow beats deny at same regex tier) |
| `chat.usercontent.google.com` | 360 | new, group 9 only |
| `hangouts.google.com` | 361 | new, group 9 only, legacy fallback |
| `clients6.google.com` | 349 | added group 9 — covers signaler-pa/chat-pa/people-pa subdomains |
| `clients4.google.com` | 362 | new, group 9 only, chat sync |
| `mtalk.google.com` | 363 | new, group 9 only, push channel |

Domain list modeled on Benedict's laptop (kids2/group 3), which already had full working Chat access. Old deny rules 271-274 are still in the DB but are overridden, not deleted.

**Docs/Drive/Sheets** were already allowed pre-existing (rules 256, 257, 291) — unaffected by the Chat/Gmail change, still working.

**Verify after any Pi-hole change:** SSH to the laptop and `nslookup chat.google.com. 192.168.12.136` (should resolve) vs `nslookup www.google.com. 192.168.12.136` (should return `0.0.0.0`). Always run `pihole reloaddns` on the ThinkCentre + `ipconfig /flushdns` on the laptop after any rule change.

---

## Setup Notes

- Firefox installed and set as default; bookmarks toolbar pinned (Milton Home Page, Homeschool Connections, Teaching Textbooks, Gmail)
- WireGuard Lambert tunnel active (`C:\lambert.conf`) — DNS line intentionally omitted to preserve Pi-hole filtering
- Edge neutered via NTFS deny-execute ACL on msedge.exe (uninstall not feasible over SSH on Win11)
- IPv6 disabled system-wide; Firefox DoH locked off via policies.json
- Sleep/hibernate disabled (Never, AC+DC, lid=nothing)

---

## Related

- Pi-hole config: `~/.claude/skills/miltonhaus-pihole-rules/`
- WireGuard: `~/.claude/skills/miltonhaus-wireguard/`
- Network overview: `~/.claude/skills/miltonhaus-devices/`
