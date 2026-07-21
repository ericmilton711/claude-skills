# Mac Mini — Google Voice-Only Kiosk

**Status as of 2026-07-21: WORKING.** Locked state fully validated — messaging and calling both confirmed end-to-end with Pi-hole restriction and Firefox policy both active. Currently left **locked**.

**Device:** Mac Mini (Fedora 43, Macmini5,1 hardware), static IP **192.168.12.163** (pinned 2026-07-21 — was drifting via DHCP, previously seen at .237). User: `mac`.
**Google Voice account:** ericmilton711@gmail.com, number (856) 354-5644 — same account already active on the Fire HD 10 tablet (see `fire-tablet-kiosk` skill).

## Goal

Dedicate the Mac Mini as the kids' "call Mom/Dad" computer while Eric and Rosemary are out, without buying another phone. When locked, its browser can reach only Google Voice (plus required Google sign-in/calling infrastructure) — nothing else. Eric toggles between locked/unlocked with one SSH command from any of his own machines.

## Browser: Firefox (not Chromium)

Originally planned around Chromium because Firefox appeared unable to do Google Voice calling — that finding was from the Fire HD 10 tablet, which runs a crippled Fire OS build of Firefox with no Google Play Services. On real desktop Firefox (this Mac Mini), calling works fine, confirmed with multiple real test calls both unlocked and locked. Chromium was never installed; Firefox is used throughout.

## Usage

```bash
ssh milton@192.168.12.136 '/home/milton/govoice-toggle.sh lock'    # Google Voice only
ssh milton@192.168.12.136 '/home/milton/govoice-toggle.sh unlock'  # normal browsing
```

One command flips both halves together: the Pi-hole client group on the ThinkCentre, and the Firefox policy + kiosk launch on the Mac Mini itself (via SSH from the server). Never ends up half-locked.

## What's built

### On the Mac Mini (192.168.12.163)
- `/etc/firefox/policies/policies.json` — swapped between two versions by the scripts below:
  - **Locked**: `WebsiteFilter` blocking `<all_urls>` with exceptions for voice.google.com, Google sign-in/API/static domains, `clients6.google.com` and its subdomains, and `telephony.goog` (the actual call-signaling domain — see "Bugs found" below) — including explicit `wss://`/`ws://` entries, since the WebSocket scheme isn't covered by the `*://` wildcard. Also locks the homepage and disables session restore.
  - **Unlocked**: just the pre-existing `DNSOverHTTPS: false` + `NetworkPrediction: false` (unchanged from before this project).
- `/usr/local/bin/govoice-lock` / `/usr/local/bin/govoice-unlock` — install/remove the policy + autostart entry, launch/stop Firefox in kiosk mode on the **default** Firefox profile (see "Bugs found" — a separate dedicated profile was tried and abandoned, it was the actual cause of a crash). Requires root; scoped passwordless sudo via `/etc/sudoers.d/govoice` (only these two commands).
- `/home/mac/.config/autostart/govoice-kiosk.desktop` — GNOME autostart entry (`firefox -kiosk https://voice.google.com`), present only while locked, so it also survives a reboot.
- Firefox is launched via `systemd-run --user --unit=govoice-kiosk`, bound to the Mac Mini's persistent user session rather than the transient SSH session — necessary because plain `nohup ... & disown` still gets killed when the SSH session's systemd scope tears down. Lingering enabled for `mac` (`loginctl enable-linger mac`) as a second layer of the same fix.
- Static IP pinned, IPv6 disabled, DNS pointed at Pi-hole via `nmcli con mod DIEMILTONHAUS ipv4.method manual ipv4.addresses 192.168.12.163/24 ipv4.dns 192.168.12.136 ipv6.method disabled` (see "Bugs found").
- Leftover, unused: `/home/mac/.mozilla/firefox-govoice` — a dedicated profile created while debugging, no longer referenced by any script. Harmless to leave or delete.

### On the ThinkCentre server (192.168.12.136)
- Pi-hole group **11** (`mac-mini-govoice`) — default-deny (`.*` regex) + allow-regex for: `voice.google.com`, `accounts.google.com`, `ogs.google.com`, `apis.google.com`, `googleapis.com`, `gstatic.com`, `googleusercontent.com`, `play.google.com`, `pki.goog`, `clients6.google.com`, `telephony.goog` (added after the first real-call test — see below), Firefox essentials (`firefox.com`, `mozilla.com/net/org`, `ipv4only.arpa`, `detectportal.firefox.com`), plus connectivity-check domains (`fedoraproject.org`, `connectivitycheck.gstatic.com` — needed or GNOME shows a false "sign in to network" popup).
- Pi-hole group **12** (`mac-mini-unrestricted`) — no custom rules, fully open. Used when unlocked.
- `/home/milton/govoice-toggle.sh lock|unlock` — flips the Mac Mini's `client_by_group` row (client_id 1) between groups 11/12, SSHes into the Mac Mini (key auth both directions) to run `govoice-lock`/`govoice-unlock`, then `pihole reloaddns`.

## Bugs found and fixed along the way

1. **Mac Mini's DNS was never actually pointed at Pi-hole** — it resolved via the router (192.168.12.1) directly, meaning Pi-hole's original group 1 rules had been a complete no-op the entire time this device existed. Fixed via `nmcli con mod ... ipv4.dns 192.168.12.136`.
2. **IP was drifting via DHCP** (seen at .163, then .237) despite being documented everywhere as .163 — silently breaks Pi-hole's IP-based client-group assignment. Pinned static via the same `nmcli` command.
3. **False "sign in to network" GNOME popup** — NetworkManager's connectivity-check domain was blocked while group 11's rules were still incomplete, so GNOME assumed a captive portal. Fixed by allowlisting `fedoraproject.org` and `connectivitycheck.gstatic.com`.
4. **Kiosk-mode Firefox crashed within ~10-25 seconds, every time**, via `-kiosk`, `systemd-run`, or plain backgrounding — journal showed `Exiting due to channel error` / compositor `AbnormalShutdown`. Root cause: launching with a freshly created, separate Firefox profile (`-no-remote -profile /home/mac/.mozilla/firefox-govoice`), created specifically to avoid restored old tabs/homepage from the device's normal-use profile. The **fresh profile itself** was unstable for reasons never fully diagnosed (possibly first-run shader/GL compilation on old Mac Mini graphics hardware). Fix: drop the separate profile entirely and use the **default** profile — `-kiosk https://voice.google.com` correctly overrides any old homepage/session state on its own, so the separate profile wasn't even necessary in the first place.
5. **Calling failed under the real lock, even though messaging and page-load worked fine**: the actual real-time call-signaling domain is `web.voice.telephony.goog` — completely different from `voice.google.com`, and undiscoverable without a real call attempt (it never appears just from loading the page or texting). Missing from both the Pi-hole allowlist and the Firefox `WebsiteFilter` policy initially. Found by watching the Pi-hole query log live during a real call attempt.
6. **Even after adding `telephony.goog` to both layers, calling still silently failed** the first time — the domain never appeared in the DNS log at all, meaning Firefox's own policy was blocking it before attempting resolution. Cause: Firefox `WebsiteFilter`'s `*://` scheme wildcard only covers `http`/`https`, not `ws`/`wss` — and Voice's real-time signaling uses a WebSocket. Fixed by adding explicit `wss://*.telephony.goog/*` (and `ws://`) exceptions, plus the same for `clients6.google.com` and `voice.google.com` as a precaution.

## Related

- `fire-tablet-kiosk` — same Google Voice account; the Firefox-can't-call finding there is Fire-OS-specific, does not apply to desktop Firefox
- `miltonhaus-pihole-rules` — group numbering, domain allow/deny playbooks, direct-DB helper commands
- `miltonhaus-devices` — device inventory; update this if the Mac Mini's IP is ever intentionally changed from .163
