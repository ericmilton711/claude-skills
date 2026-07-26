# Eva's Chat + Email Timer — 9:30-10:30pm Daily Window

**Set up:** 2026-07-25
**Status:** Live. Two independent layers enforce the same window: Pi-hole (network) + a Windows scheduled task (tab-closer).

---

## What this does

Eva (MSI laptop, `eva milton@192.168.12.202`, Pi-hole client_id 13) can only use **Google Chat** and **Gmail** between **9:30pm and 10:30pm** every day. Outside that window both are DNS-blocked. Everything else on her laptop (Docs/Drive/Sheets, Duolingo, homeschool sites, search-engine blocks) is unaffected — this only touches Chat + Gmail.

This supersedes the "Chat + Gmail always allowed" standing policy that was set earlier the same day (2026-07-25) — see `eva-msi-laptop` skill.

Two layers, because DNS blocking alone can't cut off an already-open tab immediately:

1. **Pi-hole (network layer)** — blocks/allows the actual DNS resolution for Chat/Gmail domains. This is the real enforcement; Eva has no access to it.
2. **Windows Scheduled Task (client layer)** — force-closes any open Gmail/Chat browser tab at 10:30pm sharp, so a session that was open right at the cutoff doesn't linger for a few minutes waiting on DNS caches/connection cycling.

---

## Layer 1 — Pi-hole group swap

**New group created:** `eva-chat-email-window` (group_id **13**, coincidentally same number as Eva's client_id — different tables, no relation).

All domain rules that are *exclusively* for Gmail/Chat were moved off group 9 (Eva's permanent group) and onto group 13 (the window-only group). Rules shared with Docs/Drive/general Google infra (googleapis.com, gstatic.com, googleusercontent.com, ssl.gstatic.com, pki.goog, docs/drive/sheets.google.com, accounts.google.com) were left on group 9 permanently — untouched.

Domains moved from group 9 → group 13 (via `PUT /api/domains/allow/regex/<url-encoded-domain>` with new `groups` array):

| Domain (regex) | Rule ID | Was on groups | Now on groups |
|---|---|---|---|
| `(^\|[.])mail[.]google[.]com$` | 250 | 0,8,9 | 0,8,**13** |
| `(^\|[.])gmail[.]com$` | 251 | 0,8,9 | 0,8,**13** |
| `(^\|[.])chat[.]google[.]com$` | 258 | 0,8,9 | 0,8,**13** |
| `(^\|[.])apis[.]google[.]com$` | 275 | 0,9,11 | 0,11,**13** |
| `(^\|[.])ogs[.]google[.]com$` | 276 | 0,9,11 | 0,11,**13** |
| `(^\|[.])workspace[.]google[.]com$` | 280 | 0,9 | 0,**13** |
| `(^\|[.])clients6[.]google[.]com$` | 349 | 0,9,11 | 0,11,**13** |
| `dynamite.*[.]clients6[.]google[.]com$` | 359 | 9 | **13** |
| `(^\|[.])chat[.]usercontent[.]google[.]com$` | 360 | 9 | **13** |
| `(^\|[.])hangouts[.]google[.]com$` | 361 | 9 | **13** |
| `(^\|[.])clients4[.]google[.]com$` | 362 | 9 | **13** |
| `(^\|[.])mtalk[.]google[.]com$` | 363 | 9 | **13** |

Client 13 (Eva) is a **permanent** member of group 9, and a member of group 13 **only during the 9:30-10:30pm window**. Pi-hole ORs allow-rules across all groups a client belongs to, so being added to group 13 makes the Chat/Gmail domains resolve; removing her from group 13 makes them block again (falls back to group 9's default-deny catch-all).

### Cron on ThinkCentre (controls the window)

```
30 21 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES (13,13);" && docker exec pihole pihole reloaddns # eva-chat-email open
30 22 * * * docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM client_by_group WHERE client_id=13 AND group_id=13;" && docker exec pihole pihole reloaddns # eva-chat-email close
```

Check current membership:
```bash
ssh -o BatchMode=yes -i ~/.ssh/id_ed25519 milton@192.168.12.136 \
  'docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "SELECT * FROM client_by_group WHERE client_id=13;"'
```
Two rows (9 and 13) = currently inside the window. One row (9 only) = outside the window (blocked).

**Manual override** (e.g. cron already passed for the day but you want to open it now): just run the INSERT/DELETE line directly over SSH, same as the cron body.

**DNS TTL cap** (`max-cache-ttl=60`, already set on the Pi-hole container from earlier work) means any *new* DNS lookup reflects the block/allow change within ~60 seconds. It does not kill already-open connections — see the caveats section below.

---

## Layer 2 — Windows scheduled task (hard tab-closer)

DNS blocking can't forcibly close a tab someone is already using. To get closer to a hard 10:30pm cutoff, a scheduled task runs locally on Eva's laptop and closes just the Gmail/Chat tabs (not the whole browser, not other tabs).

- **Script:** `C:\ProgramData\claude-scripts\close-chat-gmail-tabs.ps1`
- **Log:** `C:\ProgramData\claude-scripts\tab-closer-log.txt`
- **Scheduled Task name:** `FirefoxSessionCleanup` (deliberately generic name — Eva doesn't know how to use admin tools on her machine, so this is just tidy, not a serious tamper-proofing measure)
- **Trigger:** Daily at 22:30, **Logon Mode: Interactive only**, **Run As User: Eva Milton**

### How it works

Uses .NET UI Automation (`System.Windows.Automation`) to walk Firefox's actual tab strip, reading each tab's **title** (not URL — Firefox doesn't expose tab URLs to non-extension automation). Matches titles containing `Gmail`, `Google Chat`, or ` - Chat`, and closes only those tabs via the tab's close-button `InvokePattern` (falls back to selecting the tab + sending Ctrl+W if no close button is found). Every other open tab is left alone.

### Why it must be an Interactive scheduled task, not SSH

Critical gotcha discovered during setup: **SSH sessions to a Windows box run in a separate window station from the user's real logged-in desktop.** Querying `Get-Process firefox | select MainWindowHandle` over SSH returns `0` for every process, even though Firefox has real, visible windows in Eva's actual session — SSH simply can't see them. This matches [[feedback_no_gui_automation_over_ssh]].

Windows Scheduled Tasks configured as **"Run only when user is logged on" (Interactive)** are a different, session-aware launch mechanism — the Task Scheduler service actually injects the process into the user's real interactive session. This is *why* the task works even though an equivalent command run directly over SSH would not. Confirmed live: the SSH diagnostic query showed `hwnd=0` for all Firefox processes, but triggering the same script via `schtasks /Run` (which asks the Task Scheduler service to do the launch, not SSH itself) produced a real non-zero window handle and successfully closed a "Gmail" tab.

**Consequence:** you cannot verify this script's behavior by querying UI state over SSH — it will always look broken from that angle. To verify, either trigger the task (`schtasks /Run /TN "FirefoxSessionCleanup"`) and read the **log file** afterward (plain file I/O over SSH works fine), or watch it happen live in person.

### Deploying / updating the script

```bash
cat close-chat-gmail-tabs.ps1 | ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" \
  "powershell -Command \"\$input | Set-Content -Path 'C:\ProgramData\claude-scripts\close-chat-gmail-tabs.ps1' -Encoding UTF8\""
```

### Creating/checking the task

```bash
# Create (run once)
ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" \
  'schtasks /Create /TN "FirefoxSessionCleanup" /TR "powershell.exe -WindowStyle Hidden -ExecutionPolicy Bypass -File C:\ProgramData\claude-scripts\close-chat-gmail-tabs.ps1" /SC DAILY /ST 22:30 /F'

# Check config
ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" 'schtasks /Query /TN "FirefoxSessionCleanup" /V /FO LIST'

# Manually trigger (for testing)
ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" 'schtasks /Run /TN "FirefoxSessionCleanup"'

# Read the log afterward
ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" 'powershell -Command "Get-Content C:\ProgramData\claude-scripts\tab-closer-log.txt -Tail 40"'
```

### Full script

```powershell
# Closes only Firefox tabs whose title matches Gmail or Google Chat.
# Scheduled to run daily at 10:30pm on Eva's laptop.
# Leaves every other open tab (Docs, homeschool sites, etc.) untouched.

Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
Add-Type -AssemblyName System.Windows.Forms

$titlePatterns = @('Gmail', 'Google Chat', ' - Chat')
$logPath = 'C:\ProgramData\claude-scripts\tab-closer-log.txt'
function Log($msg) {
    "$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  $msg" | Out-File -FilePath $logPath -Append -Encoding UTF8
}

Log "--- run start ---"

$firefoxProcs = Get-Process firefox -ErrorAction SilentlyContinue
if (-not $firefoxProcs) {
    Log "no firefox processes found"
    exit 0
}
Log "found $($firefoxProcs.Count) firefox process(es)"

foreach ($proc in $firefoxProcs) {
    foreach ($hwnd in ($proc | Select-Object -ExpandProperty MainWindowHandle)) {
        Log "proc $($proc.Id) hwnd=$hwnd"
        if ($hwnd -eq 0) { continue }

        try {
            $root = [System.Windows.Automation.AutomationElement]::FromHandle($hwnd)
        } catch {
            Log "FromHandle threw: $_"
            continue
        }
        if (-not $root) { continue }

        $tabCondition = New-Object System.Windows.Automation.PropertyCondition(
            [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
            [System.Windows.Automation.ControlType]::TabItem
        )
        $tabs = $root.FindAll([System.Windows.Automation.TreeScope]::Descendants, $tabCondition)
        Log "found $($tabs.Count) tab(s) in this window"

        foreach ($tab in $tabs) {
            $name = $tab.Current.Name
            if (-not $name) { continue }
            Log "tab title: '$name'"

            $matched = $false
            foreach ($pattern in $titlePatterns) {
                if ($name -like "*$pattern*") { $matched = $true; break }
            }
            if (-not $matched) { continue }
            Log "MATCH -> closing '$name'"

            $buttonCondition = New-Object System.Windows.Automation.PropertyCondition(
                [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
                [System.Windows.Automation.ControlType]::Button
            )
            $closeBtn = $tab.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $buttonCondition)

            if ($closeBtn) {
                try {
                    $invokePattern = $closeBtn.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern)
                    $invokePattern.Invoke()
                    Start-Sleep -Milliseconds 300
                    Log "closed via InvokePattern"
                } catch {
                    Log "InvokePattern failed: $_ -- falling back to Ctrl+W"
                    try {
                        $selPattern = $tab.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern)
                        $selPattern.Select()
                        Start-Sleep -Milliseconds 200
                        [System.Windows.Forms.SendKeys]::SendWait("^w")
                        Start-Sleep -Milliseconds 300
                        Log "closed via Ctrl+W fallback"
                    } catch {
                        Log "Ctrl+W fallback also failed: $_"
                    }
                }
            } else {
                Log "no close button found for '$name'"
            }
        }
    }
}
Log "--- run end ---"
```

---

## Why not a browser extension instead?

Considered first, since it could match by actual URL instead of guessing from tab title. Ruled out: release-channel Firefox enforces Mozilla's AMO signature requirement even for enterprise-policy force-installed extensions — there's no local/unsigned bypass (`xpinstall.signatures.required` cannot be overridden via `policies.json` on Release/Beta, only on ESR/Nightly/Dev builds). Getting a real signed extension would require an AMO developer account and the self-distribution signing flow, which is significant overhead for this. The UI Automation approach avoids all of that at the cost of matching by title text rather than URL.

Sources checked: [Mozilla ExtensionSettings docs](https://firefox-admin-docs.mozilla.org/reference/policies/extensionsettings/), [Mozilla support thread on signature bypass](https://support.mozilla.org/en-US/questions/1134589).

---

## Known caveats (told to Eric directly, worth remembering)

1. **Not an instant kill-switch.** DNS blocking stops *new* connections; an already-open tab with a live connection can keep working for up to a few minutes past 10:30pm until Google's own channel/reconnect cycle forces a fresh DNS lookup. The scheduled task closes the tab at 10:30pm regardless, which is the main thing that makes the cutoff feel hard.
2. **Title-based matching, not URL-based.** If Google ever changes the tab title format for Gmail/Chat, the pattern list (`Gmail`, `Google Chat`, ` - Chat`) may need updating.
3. **Not tamper-proof against a technically savvy admin.** Eva has local admin rights on this laptop, so in principle a knowledgeable user could find and remove the scheduled task. Acceptable here because Eva doesn't know how to use admin tools. The Pi-hole layer is the real, non-bypassable enforcement since she has no access to the ThinkCentre.
4. **Verifying the Windows side must go through the log file, not a live SSH query** — see the SSH window-station gotcha above.

---

## Related

- [[project_eva_chat_email_timer]] — memory entry with the why/when
- `eva-msi-laptop` skill — hardware, SSH access, base Pi-hole group 9 config
- `kids-research-timer` skill — the client_by_group cron pattern this reuses (there it's used to *open* access temporarily; here it's inverted to *restrict* access to a window)
- `miltonhaus-pihole-rules` skill — overall Pi-hole default-deny philosophy
