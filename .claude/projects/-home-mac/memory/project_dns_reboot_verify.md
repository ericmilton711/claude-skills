---
name: dns-reboot-verification
description: After reboot on 2026-03-16, verify DNS fix persisted and internet speed is still good
type: project
---

On 2026-03-16, fixed slow internet by replacing dead DNS server (192.168.1.104) with Cloudflare (1.1.1.1) and Google (8.8.8.8) on network profile "Profile 1". Speed went from ~2 Mbps to ~126 Mbps. User is rebooting to confirm the fix persists.

**Why:** User wants confirmation the DNS change survived a reboot.
**How to apply:** On next conversation, check DNS settings (`cat /etc/resolv.conf`) and run a speed test to verify. Remove this memory once confirmed.
