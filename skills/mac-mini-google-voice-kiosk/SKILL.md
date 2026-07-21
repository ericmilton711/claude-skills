# Mac Mini — Google Voice-Only Kiosk (Plan)

**Status:** Planned 2026-07-21. Not yet built — Chromium vs Firefox decided, implementation not started.

**Device:** Mac Mini (Fedora), 192.168.12.163. Currently Pi-hole group 1 (`mac-mini`, block-all, no whitelist).
**Google Voice account:** ericmilton711@gmail.com, number (856) 354-5644 — same account already active on the Fire HD 10 tablet (see `fire-tablet-kiosk` skill).

## Goal

Dedicate the Mac Mini as the kids' "call Mom/Dad" computer while Eric and Rosemary are out, without buying another phone. Its browser should reach **only** Google Voice (plus the Google infrastructure needed to sign in) — nothing else. Eric can toggle it back to normal/unrestricted browsing with one SSH command from any of his own machines — no Claude Code session required.

## Key finding: Firefox can't do Google Voice calling

Confirmed while setting up the Fire tablet (`fire-tablet-kiosk` skill): Firefox handles Google Voice **messaging** fine but its "Connect" button for calls does literally nothing — Firefox does not support Google Voice's WebRTC calling flow. Google Voice calling is Chrome-first. Since the Mac Mini is a full Fedora desktop (unlike the Fire tablet's limited Fire OS), a real Chromium install should support calling properly. **Decision: use Chromium, not Firefox, for the Mac Mini kiosk.**

## Part 1 — Browser lockdown (on the Mac Mini)

- `dnf install chromium`
- Enterprise policy at `/etc/chromium/policies/managed/policy.json`:
  - `URLBlocklist: ["*"]`
  - `URLAllowlist`: `voice.google.com`, `accounts.google.com`, `ogs.google.com`, `apis.google.com`, `*.googleapis.com`, `*.gstatic.com`, `*.googleusercontent.com`, `play.google.com`, `pki.goog`, `clients6.google.com` (+ whatever else a live call attempt surfaces as blocked)
  - Homepage locked to `https://voice.google.com`
- Autostart entry launches `chromium --kiosk https://voice.google.com` on login — no desktop, no address bar, no way to escape kiosk mode.
- Two local scripts: `govoice-lock` / `govoice-unlock` — install/remove the policy + kiosk autostart, restart Chromium.

## Part 2 — Pi-hole (on the ThinkCentre server, defense in depth)

- New Pi-hole group (`mac-mini-govoice`), default-deny, same domain allowlist as the Chromium policy.
- A second, fully-unrestricted group (same pattern as Eva's laptop / Ev's Chromebook, see `miltonhaus-pihole-rules` group 6) for the unlocked state.
- Necessary because group 1 currently blocks everything with no whitelist — unlocking Chromium alone would not restore normal browsing without also flipping the Pi-hole group.

## Part 3 — The one-command toggle

- Script on the ThinkCentre: `/home/milton/govoice-toggle.sh lock|unlock`
- Flips the Mac Mini's Pi-hole group **and** SSHes into the Mac Mini to swap the Chromium policy + restart the browser, in one shot.
- Trigger from any of Eric's own machines:
  ```bash
  ssh milton@192.168.12.136 '/home/milton/govoice-toggle.sh unlock'
  ssh milton@192.168.12.136 '/home/milton/govoice-toggle.sh lock'
  ```

## Validation (after building, before calling it done)

Test messaging **and** an actual call on the Mac Mini with the locked Pi-hole group in place. Watch the Pi-hole query log during a live call attempt — calling may need 1-2 more whitelisted domains not obvious up front (same iterative process used for Eva's laptop's Gmail/Chat domain list, see `miltonhaus-pihole-rules`).

## Related

- `fire-tablet-kiosk` — same Google Voice account, same WebRTC-calling finding, messaging-only workaround still in place there as of 2026-07-20
- `miltonhaus-pihole-rules` — group numbering, domain allow/deny playbooks, direct-DB helper commands
- `firefox-browser-lock` — different pattern (password-gated general browsing), not used here since the goal is the opposite: easy kid access to one site, not gated access to everything
