# Duolingo Login — Kids1 (Patrick's Laptop)

**Last Updated:** 2026-07-04
**Status:** Unresolved — extensive troubleshooting exhausted every network/browser cause we could find. Recommendation: contact Duolingo support directly, this looks like an account/device flag on their end.

---

## Goal

Get the Duolingo account `sr71pattywagon` logged in on kids1 (Patrick's laptop, 192.168.12.249) so he can use it without login trouble.

## Correction: kids1 uses Firefox, not Edge

Earlier notes assumed Edge. **It's actually Firefox**, accessed via a bookmark (`https://www.duolingo.com/learn`). Edge is technically installed (confirmed in the registry, v146) but has no visible shortcut and isn't what Patrick uses.

## Firefox on kids1 is the Microsoft Store (packaged/MSIX) version

This matters a lot for any future remote troubleshooting:

- Executable alias: `C:\Users\themi\AppData\Local\Microsoft\WindowsApps\firefox.exe`
- Real profile data lives in a sandboxed path, NOT `%APPDATA%\Mozilla\Firefox\`:
  ```
  C:\Users\themi\AppData\Local\Packages\Mozilla.Firefox_n80bbvh6b1yt2\LocalCache\Roaming\Mozilla\Firefox\Profiles\
  ```
- Multiple profile folders exist there, but check `profiles.ini` in that same Roaming\Mozilla\Firefox\ folder to find the one actually in use — look for the `[InstallXXXXXXXX]` section's `Default=` path (this is what a locked/store install actually launches), not the older `[Profile1]` section's `Default=1` flag. On kids1 as of 2026-07-04 that's `Profiles/1I593DKn.Profile 1`.
- **The standard Firefox DoH-lock policy (`C:\Program Files\Mozilla Firefox\distribution\policies.json`) does NOT apply to this install** — it's a completely different sandboxed app, doesn't read that path. If DoH ever needs locking down on this specific machine, it has to be done via profile prefs (`network.trr.mode` in `prefs.js`) instead. (As of this session, DoH was already off — `doh-rollout.mode: 0` — so this wasn't the cause of anything, just a gotcha to remember.)

## profiles.ini "Locked" gotcha

The `[InstallXXXXXXXX]` section has `Locked=1`. This means if you delete or rename the profile folder Firefox is locked to, **Firefox will NOT auto-create a replacement** — it throws "Your Firefox profile cannot be loaded. It may be missing or inaccessible." Instead, you must recreate an **empty folder at the exact same path**, and Firefox will initialize fresh profile files there on next launch. Renaming the path in `profiles.ini` itself is a cleaner alternative if you want to point it somewhere new instead.

## Technique: editing Firefox SQLite databases on Windows via SSH (no sqlite3 on Windows)

Windows/PowerShell has no built-in SQLite tooling. Working pattern used successfully this session:

1. `scp` the `.sqlite` file (main file only if Firefox was cleanly closed; include `-wal`/`-shm` siblings too if unsure) from the Windows box to Linux:
   ```
   scp -o StrictHostKeyChecking=no "themi@192.168.12.249:C:/.../cookies.sqlite" /tmp/local_copy/
   ```
   (pexpect-driven, password `1229` — see [[feedback_pexpect_ssh]]). Scp works fine even though PowerShell exec over SSH is otherwise used for everything else — Windows OpenSSH Server ships sftp-server by default.
2. Edit locally with `sqlite3` (e.g. `PRAGMA wal_checkpoint(TRUNCATE);` first if a `-wal` file exists, then `DELETE FROM ... WHERE ...;`).
3. `scp` the cleaned single file back, overwriting the original.
4. **Delete the remote `-wal` and `-shm` files** after overwriting the main db — leaving stale WAL/SHM files next to a replaced main file will cause Firefox to load an inconsistent/corrupt-looking database.
5. Make sure Firefox (and all its background processes — it spawns many, e.g. 15+ `firefox.exe` PIDs) is fully stopped first (`Stop-Process -Name "*firefox*" -Force`), or file locks make edits/deletes silently fail even with `-ErrorAction SilentlyContinue` masking the real error.

This same technique works for `places.sqlite` (bookmarks/history) if bookmarks ever need extracting/editing outside the GUI.

## Everything ruled out this session (2026-07-04)

Password was reset multiple times to different values; credentials confirmed **correct** — same password logged in successfully from other devices/networks, including Eric's own separate Windows laptop on the same home network. Despite that, kids1 consistently returned "wrong password" for every attempt, including Eric typing it himself at the physical keyboard. None of the following fixed it:

- Pi-hole domain-level blocking (kids1 moved to a fully unrestricted group, confirmed zero deny rules attached — Google/gstatic/recaptcha domains all resolved fine)
- Pi-hole as the DNS resolver at all (tested pointing kids1's DNS straight at 1.1.1.1, bypassing Pi-hole completely — no change)
- Stale saved Firefox password for duolingo.com (found and removed one with a suspiciously recent `timeLastUsed`)
- Cached Firefox site storage for `www.duolingo.com` and a partitioned `recaptcha.net` origin, plus the registered Service Worker (`gcm-service-worker.js`) — all cleared
- Stale Duolingo cookies including `csrf_token` and `jwt_token` (removed via the sqlite scp-edit-scp technique above; 31 duolingo/recaptcha cookies deleted, all other sites' cookies left untouched)
- System clock drift (checked — in sync with the ThinkCentre to within 1 second)
- WireGuard tunnel routing all traffic through a flagged egress (checked — `lambert` tunnel is split-tunnel, only routes 192.168.0.0/24 and 192.168.2.0/24, general internet traffic is unaffected)
- Low-level keyboard remapping (no scancode map, no AutoHotkey/PowerToys processes, standard en-US layout, no StickyKeys/FilterKeys)
- Wrong Windows account (confirmed `explorer.exe` and all Firefox processes run as `kids1\themi`, matching the SSH account used for all the fixes above — not a case of editing the wrong profile)
- A completely fresh Firefox profile (renamed the old one aside, let Firefox build a new one) — **also failed**, which somewhat undercuts the "flagged browser fingerprint" theory too

**Network context worth knowing:** MILTONHAUS's home internet is T-Mobile Home Internet (fixed wireless/cellular, confirmed via `ip-api.com` lookup — `mobile: true`, AS21928 T-Mobile USA), which uses Carrier-Grade NAT (shared public IP across many customers). A router power-cycle rotates the public IP. This was a strong theory for a while, but Eric successfully logging in from his own separate laptop on the exact same (rotated) IP argues against a pure network/IP-level block.

## Current recommendation

Nothing left to try on the technical/network side from here. **Contact Duolingo support directly** — describe that the correct password is being rejected consistently only for this one device, from a household that otherwise has multiple devices successfully logged into the same account. This smells like a device/session-level fraud flag on Duolingo's end (possibly tied to reCAPTCHA Enterprise risk scoring, which the login page loads) that only they can see or clear.

## Backed-up bookmarks (kids1 Firefox toolbar, as of 2026-07-04)

In case the profile ever needs rebuilding again:

1. Homeschool Connections — https://caravel.homeschoolconnections.com/splash
2. Milton Home Page — http://192.168.0.100:5006/
3. MILTONHAUS Weather — http://192.168.12.240/
4. (Fall 2022) Middle School Catechism, Part One with Dan Egan - Zoom — (long signed Zoom recording URL, see chat history if needed)
5. Duolingo — https://www.duolingo.com/learn
6. Translator - JMJ Local — http://192.168.0.102:5003/

(Firefox's stock default bookmarks — Get Help, Customize Firefox, Get Involved, About Us — aren't included; any new profile gets those automatically.)

## Key technical finding: SSH to Windows can't do GUI automation

Tried automating the login remotely via SSH (see [[feedback_pexpect_ssh]] pattern: `themi@192.168.12.249` pw `1229`). This **does not work for GUI tasks**:

- `Start-Process <browser>.exe <url>` over SSH launches the browser in a session with no window station attached to the physical display.
- Confirmed via `[System.Drawing.Graphics]::CopyFromScreen(...)` → `"The handle is invalid"` — the SSH session literally cannot see or interact with the desktop GUI, even though the process technically launches.
- SendKeys-based form filling is not viable here — there's no way to verify what's on screen or that the right window has focus.

**Conclusion:** Any task on kids1 (or other Windows machines) that requires clicking, typing into a GUI form, or reading what's on screen must be done at the physical keyboard, or via a real remote-desktop protocol (RDP/VNC) with visual feedback — not blind PowerShell-over-SSH. Headless file/registry/process work (like everything in this doc) works fine over SSH.

## Related

- [[reference_kids1_laptop]] — SSH creds and device info for kids1
- [[feedback_pexpect_ssh]] — SSH automation pattern for Windows machines (works for headless commands, not GUI)
- [[project_thinkcentre_ssh_exec_hang]] — separate, unrelated ThinkCentre SSH issue that recurred mid-session (SSH auths but exec hangs) — use the Pi-hole HTTP API directly instead of SSH `docker exec` when this happens
