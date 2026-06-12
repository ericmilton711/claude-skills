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
- **Allowed:** Homeschool Connections, Teaching Textbooks, Duolingo, Kiddle, Vimeo, Zoom, Firefox/Mozilla, Windows essentials, Gmail, Britannica
- **Blocked:** Google Search, YouTube, Google Chat/Hangouts
- **pki.goog** must stay whitelisted or Gmail TLS breaks (cert revocation)

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
