# Mac Mini — Google Voice-Only Kiosk

**Status as of 2026-07-21:** Toggle infrastructure built and working. Mac Mini currently **unlocked** (normal browsing) — kiosk mode has an unresolved Firefox crash bug, see "Known issue" below. Google Voice **calling confirmed working** in a normal (non-kiosk) Firefox session — two test calls to Eric's phone connected with audio both ways.

**Device:** Mac Mini (Fedora 43, Macmini5,1 hardware), static IP **192.168.12.163** (pinned 2026-07-21 — was drifting via DHCP, previously seen at .237). User: `mac`.
**Google Voice account:** ericmilton711@gmail.com, number (856) 354-5644 — same account already active on the Fire HD 10 tablet (see `fire-tablet-kiosk` skill).

## Goal

Dedicate the Mac Mini as the kids' "call Mom/Dad" computer while Eric and Rosemary are out, without buying another phone. Its browser should reach **only** Google Voice (plus required Google sign-in infrastructure) when locked. Eric can toggle between locked/unlocked with one SSH command from any of his own machines.

## Browser: Firefox (not Chromium)

Originally planned around Chromium because Firefox appeared unable to do Google Voice calling — that finding was from the Fire HD 10 tablet, which runs a crippled Fire OS build of Firefox with no Google Play Services. **On real desktop Firefox (this Mac Mini), calling works fine** — confirmed 2026-07-21 with two real test calls. Chromium was never installed; stick with Firefox.

## What's built

### On the Mac Mini (192.168.12.163)
- `/etc/firefox/policies/policies.json` — swapped between two versions by the scripts below:
  - Locked: `WebsiteFilter` (Block `<all_urls>`, Exceptions for voice.google.com + Google sign-in/API/static domains), `Homepage` locked to voice.google.com, `Preferences` forcing `browser.sessionstore.resume_from_crash=false` and `browser.startup.page=0`.
  - Unlocked: just the pre-existing `DNSOverHTTPS: false` + `NetworkPrediction: false` (unchanged from before this project).
- `/home/mac/.mozilla/firefox-govoice` — dedicated, purpose-built Firefox profile for the kiosk, created via `firefox -CreateProfile "govoice /home/mac/.mozilla/firefox-govoice"`. Necessary because the normal/default profile has real usage history (custom homepage, pinned tabs incl. the ESP32 weather dashboard at .240) that kept getting restored on launch regardless of session-store clearing.
- `/home/mac/.config/autostart/govoice-kiosk.desktop` — GNOME autostart entry, only present while locked.
- `/usr/local/bin/govoice-lock` / `/usr/local/bin/govoice-unlock` — install/remove the policy + autostart entry, launch/stop Firefox. Requires root; scoped passwordless sudo granted via `/etc/sudoers.d/govoice` (`mac ALL=(root) NOPASSWD: /usr/local/bin/govoice-lock, /usr/local/bin/govoice-unlock` — only these two commands, nothing else).
- Static IP pinned + IPv6 disabled + DNS pointed at Pi-hole via `nmcli con mod DIEMILTONHAUS ipv4.method manual ...` (see below — this was previously broken, see "Fixed along the way").
- systemd lingering enabled for `mac` (`loginctl enable-linger mac`) so processes started over SSH survive the SSH session closing.
- Firefox launched via `systemd-run --user --unit=govoice-kiosk` (bound to the persistent user session, not the transient SSH session) — necessary because plain `nohup ... & disown` still gets killed when the SSH session's systemd scope tears down.

### On the ThinkCentre server (192.168.12.136)
- Pi-hole group **11** (`mac-mini-govoice`) — default-deny (`.*` regex, id 1) + allow-regex for: `voice.google.com`, `accounts.google.com`, `ogs.google.com`, `apis.google.com`, `googleapis.com`, `gstatic.com`, `googleusercontent.com`, `play.google.com`, `pki.goog`, `clients6.google.com`, Firefox essentials (`firefox.com`, `mozilla.com/net/org`, `ipv4only.arpa`, `detectportal.firefox.com`), plus connectivity-check domains (`fedoraproject.org`, `connectivitycheck.gstatic.com` — needed or GNOME shows a false "sign in to network" popup, see below).
- Pi-hole group **12** (`mac-mini-unrestricted`) — no custom rules, fully open. Current state.
- `/home/milton/govoice-toggle.sh lock|unlock` — flips the Mac Mini's `client_by_group` row (client_id 1) between groups 11/12, SSHes into the Mac Mini (key auth, both directions now set up) to run `govoice-lock`/`govoice-unlock`, then `pihole reloaddns`.

### Toggle usage
```bash
ssh milton@192.168.12.136 '/home/milton/govoice-toggle.sh lock'    # Google Voice only
ssh milton@192.168.12.136 '/home/milton/govoice-toggle.sh unlock'  # normal browsing (current state)
```

## Known issue: kiosk-mode Firefox crashes within ~10-25 seconds (UNRESOLVED)

Every attempt to launch Firefox with `-kiosk https://voice.google.com` — via `systemd-run --user`, via plain `nohup`, even a `-no-remote` fresh-profile launch — died within roughly 10-25 seconds with `Exiting due to channel error` / `CompositorBridgeChild receives IPC close with reason=AbnormalShutdown` in the journal. One GNOME-launched (non-kiosk-flag) Firefox instance crashed the same way during testing too, which pointed toward a GPU/graphics driver issue on this older (2011, Macmini5,1) hardware under Wayland.

**Forcing software rendering did NOT reliably fix it** (`LIBGL_ALWAYS_SOFTWARE=1 MOZ_WEBRENDER=0`) — it survived one 25-second foreground SSH test with those flags, but crashed again in ~7 seconds once redeployed through the real `govoice-lock` → `systemd-run` path. Inconclusive — never got a fully clean run through the actual toggle mechanism.

**Important counter-evidence (2026-07-21):** Eric opened Firefox normally through the desktop (not kiosk mode, not SSH-triggered, not the dedicated govoice profile) and made two real Google Voice calls with no crash at all. This means the crash is **not** a general hardware/Firefox instability problem — it's something specific to kiosk mode and/or the remote launch mechanism (`-kiosk` flag itself, the `-profile`/fresh-profile combination, or something about `systemd-run --user` + Wayland socket handling that differs from a normal GUI-launched session).

**Next things to try, not yet attempted:**
- Reproduce with `-kiosk` flag alone on the normal/default profile (isolates whether `-kiosk` itself is the trigger, independent of the fresh govoice profile).
- Reproduce by launching normally through the GNOME app launcher but pointed at the govoice profile (isolates whether the profile itself is the trigger, independent of `-kiosk`/systemd-run).
- Try `--kiosk` (double dash) instead of `-kiosk` — should be equivalent but worth ruling out.
- Check `~/.mozilla/firefox-govoice/compatibility.ini` / crash reports (`~/.mozilla/firefox/Crash Reports/`) for an actual signal instead of inferring from journal timing.

## Fixed along the way (unrelated bugs found during this build)

- **Mac Mini's DNS was never actually pointed at Pi-hole** — it was resolving via the router (192.168.12.1) directly, meaning Pi-hole group 1's rules had been a complete no-op the whole time this device existed. Fixed by setting `ipv4.dns 192.168.12.136` via `nmcli con mod DIEMILTONHAUS`.
- **IP was drifting via DHCP** (seen at .163, then .237) despite being documented everywhere as .163. Pinned static via the same `nmcli con mod` command. This matters because Pi-hole client-group assignment is by IP — drift silently breaks the whole lockdown.
- **IPv6 disabled** on the connection (same `nmcli` command) — matches the pattern used on other kid devices, since IPv6 silently bypasses Pi-hole's IPv4-only filtering.
- **False "sign in to network" GNOME popup** — triggered transiently while group 11's rules were incomplete (NetworkManager's connectivity-check domain was blocked, so GNOME assumed a captive portal). Resolved once `fedoraproject.org` and `connectivitycheck.gstatic.com` were added to the allowlist.

## Validation still needed once kiosk mode is fixed

Re-test messaging **and** a real call specifically *inside* the locked-down kiosk profile/policy (the successful test call was in the normal unlocked profile) — the Pi-hole group 11 domain list was built from Eva's laptop's Gmail-era allowlist plus guesses for Voice-specific domains (`clients6.google.com` etc.); it has not yet been validated against a real call while group 11 was actually active. Watch the Pi-hole query log during that test for anything still blocked.

## Related

- `fire-tablet-kiosk` — same Google Voice account; the Firefox-can't-call finding there is Fire-OS-specific, does not apply to desktop Firefox (see above)
- `miltonhaus-pihole-rules` — group numbering, domain allow/deny playbooks, direct-DB helper commands
- `miltonhaus-devices` — device inventory; update this if the Mac Mini's IP is ever intentionally changed from .163
