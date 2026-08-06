# Eric's Windows PC ("Eric" / Eric.lan)

**Last Updated:** 2026-08-06

---

## Identity

- **Hostname:** Eric
- **Username:** ericm
- **Password:** 645866
- **Home network IP:** 192.168.12.220 (was .219, drifts — see `skills/miltonhaus-devices/SKILL.md`)
- **OS:** Windows 11
- **Python:** 3.12.10, installed at `C:\Users\ericm\AppData\Local\Programs\Python\Python312\`. Both `python` and `python3` work from a terminal.

---

## SSH Access

- **No SSH key auth configured** — password only. Use the pexpect pattern (see `skills/windows-ssh-powershell-quirks/SKILL.md`), not sshpass.
- Default shell over SSH is `cmd.exe`; PowerShell is available at the usual path for `-EncodedCommand` work.
- For file transfer, plain `scp` works fine over password auth (via pexpect) — no need to fall back to base64/PowerShell tricks for whole files, only for inline text edits.

### Off the home LAN
When Eric is away from the MILTONHAUS network with this laptop, it's still discoverable via mDNS as **`Eric.local`** (confirmed working 2026-08-06 on an AT&T-gateway network at 192.168.1.x). Resolve with `ping Eric.local` or `avahi-resolve -n Eric.local`, then SSH to whatever IP that returns with the same ericm/645866 credentials. Verify it's actually this machine (confirm username is `ericm`) before connecting on an unfamiliar network — other devices can broadcast similar names.

---

## Files Eric Keeps There

- **Desktop:** resume (`Eric C Milton 2026 Resume.docx`), cover letter, DD-214, externship timesheets, `Job Search Links.docx` (older, superseded by the tracker below).
- **`C:\Users\ericm\`** (home dir root): a bunch of personal Python scripts (`homestead.py`, `blink.py`, `genqr.py`, etc.) — this is where scripts should go if Eric wants to just `cd` into a fresh terminal and run `python3 <script>.py` with no path, since a new terminal opens here by default.

### Added 2026-08-06
- **`Desktop\Job Hunt Tracker.md`** — snapshot of the `electronics-technician-job-search` skill content, sent over for offline viewing on this laptop.
- **`Desktop\Claude Skills (GitHub).url`** — shortcut to https://github.com/ericmilton711/claude-skills. Lets Eric view any skill or upload new ones straight from the GitHub web UI (Add file → Upload files) with no git install needed — the easiest way to browse/add skills from a device that isn't set up with the git repo.
- **`tracker.py` + `applications.csv`** — moved to `C:\Users\ericm\` (home dir root, not a subfolder) specifically so `python3 tracker.py` works immediately from a fresh terminal, matching how it behaves on the Fedora machine where the same files live directly in `/home/ericmilton/`. A subfolder would have required `cd` first.

---

## Related

- All devices: `skills/miltonhaus-devices/SKILL.md`
- SSH/PowerShell quirks: `skills/windows-ssh-powershell-quirks/SKILL.md`
- Job search tracker (source of truth): `skills/electronics-technician-job-search/SKILL.md`
