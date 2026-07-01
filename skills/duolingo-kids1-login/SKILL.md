# Duolingo Login — Kids1 (Patrick's Laptop)

**Last Updated:** 2026-07-01
**Status:** Blocked — waiting on password reset. **No pings/notifications about this — revisit only when Eric brings it up.**

---

## Goal

Get the Duolingo account `sr71pattywagon` logged in on kids1 (Patrick's laptop, 192.168.12.249) and remembered by the browser (Edge), so Patrick doesn't have to log in each time.

## Current blocker

- Given credentials: username `sr71pattywagon`, password `sr71pattywagon`.
- Account **exists** and **was set up with a password** (not Google/Apple/Facebook sign-in).
- Login attempt returned **"wrong password"** (not "user not found") — so the username is right but `sr71pattywagon` is not the actual password.
- Fix requires a password reset via Duolingo's "Forgot password?" flow, which sends a reset link to the account's registered email.
- Whoever has access to that email needs to click the reset link and set the new password. As of 2026-07-01 that person is asleep — nothing to do until they're available.

## Key technical finding: SSH to Windows can't do GUI automation

Tried automating the login remotely via SSH (see [[feedback_pexpect_ssh]] pattern: `themi@192.168.12.249` pw `1229`). This **does not work for GUI tasks**:

- `Start-Process msedge.exe <url>` over SSH launches Edge in a session with no window station attached to the physical display.
- Confirmed via `[System.Drawing.Graphics]::CopyFromScreen(...)` → `"The handle is invalid"` — the SSH session literally cannot see or interact with the desktop GUI, even though the process technically launches.
- SendKeys-based form filling is not viable here — there's no way to verify what's on screen or that the right window has focus.

**Conclusion:** Any task on kids1 (or other Windows machines) that requires clicking, typing into a GUI form, or reading what's on screen must be done at the physical keyboard, or via a real remote-desktop protocol (RDP/VNC) with visual feedback — not blind PowerShell-over-SSH.

## Next steps (when revisited)

1. Reset the Duolingo password via "Forgot password?" (needs email access).
2. Set new password to `sr71pattywagon` to match the username, as originally intended.
3. Physically log in on kids1 in Edge at duolingo.com, click "Save" when Edge offers to remember the password.
4. Session persistence after that is automatic — Duolingo doesn't have a separate "remember me" toggle, it just keeps you logged in via cookies unless logged out.

## Related

- [[reference_kids1_laptop]] — SSH creds and device info for kids1
- [[feedback_pexpect_ssh]] — SSH automation pattern for Windows machines (works for headless commands, not GUI)
