# YTI Chromebook

School-issued (or school-enrolled) Chromebook for YTI. Enterprise-enrolled to YTI's Google Admin console — Eric is ~90% sure, since another student's Chromebook needed the school's IT department to resolve an enrollment issue.

## Identity

- **MAC:** `b0:47:e9:e3:78:d0`
- **IP:** historically assumed static at `192.168.12.219`, but that claim was wrong — confirmed 2026-08-13 the device actually runs on `.220` (the address other memory called "stale DHCP drift"). Treat any "static IP, will not drift" note for this device with suspicion; verify via MAC in Pi-hole's network/devices table if it acts up.

## Pi-hole status (as of 2026-08-13)

**Permanently unrestricted**, per Eric's explicit request:
- Group `yti-chromebook-unrestricted` (id 14) — zero domain rules, fully open.
- Client rule keyed by **MAC** `b0:47:e9:e3:78:d0` (client_id 17) → group 14. This is what actually matters — it matches regardless of DHCP-assigned IP.
- An older IP-based client rule (client_id 11, originally `.219`) also points to group 14 — harmless leftover, doesn't need cleanup.
- The old daily 7-8:30pm "research window" cron on the ThinkCentre (client_id 11 in/out of group 7) was deleted — it would have re-restricted the device nightly, undoing the permanent unrestrict.
- Group 7 (tower-of-gondor) still has its spot blocks (google.com, gmail.com, spotify.com, apple.com) but those now only affect Tower of Gondor (client_id 9, `.160`), not this Chromebook.

## "Internet connected, but no internet" incident (2026-08-13)

Right after the unrestrict, the Chromebook couldn't load anything, including Homeschool Connections. Root cause was **not** Pi-hole — Pi-hole showed **zero DNS queries ever** from this device's MAC/IP. The real cause: the device's on-device DNS override had silently reverted from Custom `192.168.12.136` back to Automatic, which on this network means the T-Mobile gateway (`192.168.12.1`) — and the gateway's own DNS didn't resolve anything useful. Disabling Secure DNS (DoH) did **not** fix it; this was plain DNS pointing at the wrong server.

**Fix:** On the Chromebook — Settings → Network → Wi-Fi → (connected network) → Network tab → DNS servers → switch from Automatic to **Custom** → enter `192.168.12.136`. Confirmed working after this change.

**Why this happens:** the T-Mobile gateway (TMO-G5AR, `192.168.12.1`) has **no configurable web UI at all** — screenshotted 2026-08-13, it's just a status page and a support/FAQ page, no login wall, no network/DHCP settings. All gateway config (if any exists) is app-only via T-Mobile's mobile app. That means Pi-hole DNS routing for every kid device on this network is enforced entirely by a **manual per-device DNS override**, not router DHCP options — and this override can apparently reset itself over time (cause unclear — possibly a ChromeOS network profile reset). If this device (or any other kid device) reports "no internet" or shows unfiltered browsing again, check its on-device DNS setting first, before touching Pi-hole or the router.

## Reimage / enrollment considerations

Eric wants to eventually Powerwash this Chromebook (Ctrl+Alt+Shift+R at sign-in → Restart → Powerwash) for a fresh start — not to escape restrictions, just routine cleanup before returning the device to YTI.

Important caveats:
- Since it's enterprise-enrolled, a Powerwash triggers **forced re-enrollment** — on first boot it automatically re-pulls YTI's management profile before allowing sign-in. It comes back exactly as managed as before (possibly with updated policies).
- The Custom DNS override does **not** survive a Powerwash — must be re-applied afterward.
- It's possible (not yet confirmed) that a future policy push from YTI could lock the Network/DNS settings field, which would break the ability to route this device through Pi-hole at all. If that happens there's no known workaround from this end.
- Legit unenrollment can only be done by YTI's IT department, from their Google Admin console — there is no self-service or device-side way to remove enterprise enrollment. Do not attempt developer-mode/firmware bypass tricks to dodge forced re-enrollment; that circumvents a security control YTI has legitimate authority over, not something to route around unilaterally.
- Eric has equipment to return to YTI anyway — good opportunity to hand back the Chromebook and ask them to deprovision it from their console at the same time.

## Related

- Pi-hole reference: `skills/miltonhaus-pihole-rules/SKILL.md`, `skills/kids-laptops-pihole/SKILL.md`
- Device inventory: `skills/miltonhaus-devices/SKILL.md`
- Memory: `project_yti_chromebook_lockdown.md`, `reference_tmobile_gateway.md`
