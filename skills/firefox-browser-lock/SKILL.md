# Browser Lock — Password-Gated Browser Launch

Locks browsers (Firefox, Safari) behind a password prompt. Password is required every time the browser is launched. The session file is deleted when the browser closes, so closing and reopening always requires the password again. 30-minute session window so links opened mid-browsing don't re-prompt.

**Password hash:** `bbac07878d5ebfc9079776ce25119974e7f7bb55f9bbad42195d6ee57f3fcc87` (changed 2026-08-09)

**Sync status (2026-08-09):** MacBook Pro — Fedora (.190), Windows laptop (.219, drifted from .220), and Benedict's laptop (.239) all use the same hash. Rosemary's MacBook (.109) is unaffected — it uses its own separate password/hash.

**Bypass found & NOT YET FIXED (2026-08-09):** Password change alone doesn't stop Benedict — on both his own laptop and Eric's Windows laptop, he's getting a fully unwrapped Firefox with no session token ever being written, meaning he isn't going through the password dialog at all. Two confirmed bypass vectors on Windows:
1. **`HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\firefox.exe`** registry key still points at the real `firefox.exe`. Typing "firefox" into Windows Search or Run (Win+R) launches it directly, no wrapper involved.
2. **Firefox "Web App" shortcuts** (e.g. `...\Start Menu\Programs\Firefox Web Apps\Google Mail.lnk`, created automatically by Firefox's "Install this site as an app" feature) also launch `firefox.exe` directly with a full, unrestricted window.

These exist independently of the taskbar/Start Menu/Public Desktop shortcuts that were already wrapped — a shortcut-swap approach can never be complete since Windows offers many paths to the real binary. Needs fixing: remove/redirect the App Paths key, delete or rewrap Web App shortcuts, and audit for more (any `.lnk` anywhere targeting `firefox.exe` directly, e.g. via `Get-ChildItem -Recurse -Filter *.lnk`).

---

## Deployed On

### Eric's Windows Laptop (192.168.12.220, drifts — was .219 on 2026-08-09)
- **Script:** `C:\Users\ericm\.claude\hooks\browser-unlock.ps1`
- **Shortcuts rewired:** Taskbar pin, Start Menu (Firefox + Firefox Private Browsing)
- **Deployed:** 2026-06-27
- **Password changed 2026-08-09** — Benedict was using this laptop's Firefox directly (confirmed live, 25 firefox.exe processes running, no session token file present at all). Same App Paths / Web-App-shortcut bypass suspected here as on his own laptop — not yet audited on this machine.

### MacBook Pro — Fedora (192.168.12.190)
- **Script:** `~/.local/bin/browser-unlock`
- **Desktop override:** `~/.local/share/applications/org.mozilla.firefox.desktop`
- **Deployed:** 2026-06-26

### Benedict's Windows Laptop (kids2, 192.168.12.239 / kids2.local)
- **Script:** `C:\Users\themi\browser-unlock.ps1`
- **Shortcuts rewired:** Taskbar pin, Start Menu (Firefox + Firefox Private Browsing), **Public Desktop** (`C:\Users\Public\Desktop\Firefox.lnk`)
- **Password:** Same as Eric's machines
- **Deployed:** 2026-07-30
- **Gap found & fixed 2026-08-06:** the 2026-07-30 deployment only rewired the taskbar pin and Start Menu — it missed the `Firefox.lnk` on `C:\Users\Public\Desktop`, which still pointed straight at `firefox.exe`. Benedict used that icon to get on YouTube without the password. Always check **all three** locations (taskbar, Start Menu, Public Desktop) when deploying or auditing this on Windows.
- **Microsoft Edge fully disabled 2026-08-06:** rather than wrap Edge, it was blocked outright since a kid only needs one legitimate browser. See "Fully Disabling a Browser (Windows)" below.

### Rosemary's MacBook Pro — macOS (192.168.12.109)
- **Script:** `~/.local/bin/browser-unlock`
- **Wrapper apps:** `~/Applications/Safari.app`, `~/Applications/Firefox.app`
- **Browsers locked:** Safari + Firefox
- **Dock:** Safari and Firefox icons replaced with locked wrappers
- **Password:** Different from other machines (hash: `a36bfa3cafb40c3d89ad4b8a9d6e2bdb14a8a97fbcd92780ab5fe87163bd1dbe`)
- **Deployed:** 2026-07-13

---

## Windows Version

### How It Works

1. PowerShell script at `C:\Users\ericm\.claude\hooks\browser-unlock.ps1`
2. Checks for a session token in `%TEMP%`
3. If session is active (within 30 min), launches Firefox directly
4. Otherwise shows a Windows Forms password dialog
5. On correct password, writes session token, launches Firefox, deletes token when Firefox closes
6. Taskbar pin + Start Menu shortcuts point to the wrapper via `powershell.exe -WindowStyle Hidden`

### Script (`browser-unlock.ps1`)

```powershell
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$SessionFile = "$env:TEMP\browser_unlocked_$env:USERNAME"
$SessionDuration = 1800
$StoredHash = "bbac07878d5ebfc9079776ce25119974e7f7bb55f9bbad42195d6ee57f3fcc87"
$Browser = "C:\Program Files\Mozilla Firefox\firefox.exe"

if (Test-Path $SessionFile) {
    $unlockTime = [int](Get-Content $SessionFile)
    $now = [int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds()
    if (($now - $unlockTime) -lt $SessionDuration) {
        if ($args.Count -gt 0) {
            Start-Process $Browser -ArgumentList $args
        } else {
            Start-Process $Browser
        }
        exit 0
    }
}

$form = New-Object System.Windows.Forms.Form
$form.Text = "Browser Access"
$form.Size = New-Object System.Drawing.Size(320, 170)
$form.StartPosition = "CenterScreen"
$form.FormBorderStyle = "FixedDialog"
$form.MaximizeBox = $false
$form.MinimizeBox = $false
$form.TopMost = $true

$label = New-Object System.Windows.Forms.Label
$label.Text = "Enter password to open browser:"
$label.Location = New-Object System.Drawing.Point(20, 15)
$label.Size = New-Object System.Drawing.Size(260, 20)
$form.Controls.Add($label)

$textbox = New-Object System.Windows.Forms.TextBox
$textbox.Location = New-Object System.Drawing.Point(20, 40)
$textbox.Size = New-Object System.Drawing.Size(260, 25)
$textbox.UseSystemPasswordChar = $true
$form.Controls.Add($textbox)

$okButton = New-Object System.Windows.Forms.Button
$okButton.Text = "OK"
$okButton.Location = New-Object System.Drawing.Point(120, 80)
$okButton.Size = New-Object System.Drawing.Size(80, 30)
$okButton.DialogResult = [System.Windows.Forms.DialogResult]::OK
$form.AcceptButton = $okButton
$form.Controls.Add($okButton)

$cancelButton = New-Object System.Windows.Forms.Button
$cancelButton.Text = "Cancel"
$cancelButton.Location = New-Object System.Drawing.Point(205, 80)
$cancelButton.Size = New-Object System.Drawing.Size(80, 30)
$cancelButton.DialogResult = [System.Windows.Forms.DialogResult]::Cancel
$form.CancelButton = $cancelButton
$form.Controls.Add($cancelButton)

$form.Add_Shown({ $textbox.Focus() })
$result = $form.ShowDialog()

if ($result -ne [System.Windows.Forms.DialogResult]::OK) {
    exit 1
}

$password = $textbox.Text
$sha256 = [System.Security.Cryptography.SHA256]::Create()
$hash = [System.BitConverter]::ToString($sha256.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($password))).Replace("-","").ToLower()

if ($hash -eq $StoredHash) {
    [int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds() | Out-File $SessionFile -Encoding utf8
    if ($args.Count -gt 0) {
        $proc = Start-Process $Browser -ArgumentList $args -PassThru
    } else {
        $proc = Start-Process $Browser -PassThru
    }
    $proc.WaitForExit()
    Remove-Item $SessionFile -Force -ErrorAction SilentlyContinue
} else {
    [System.Windows.Forms.MessageBox]::Show("Incorrect password.", "Access Denied", "OK", "Error")
    exit 1
}
```

### Shortcut Setup (requires admin for Start Menu)

```powershell
# Taskbar pin (no admin needed)
$shell = New-Object -ComObject WScript.Shell
$lnk = $shell.CreateShortcut("$env:APPDATA\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\Firefox.lnk")
$lnk.TargetPath = "powershell.exe"
$lnk.Arguments = '-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\Users\ericm\.claude\hooks\browser-unlock.ps1"'
$lnk.IconLocation = "C:\Program Files\Mozilla Firefox\firefox.exe,0"
$lnk.Save()

# Start Menu shortcuts (run elevated)
# Firefox
$lnk = $shell.CreateShortcut("C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Firefox.lnk")
$lnk.TargetPath = "powershell.exe"
$lnk.Arguments = '-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\Users\ericm\.claude\hooks\browser-unlock.ps1"'
$lnk.IconLocation = "C:\Program Files\Mozilla Firefox\firefox.exe,0"
$lnk.Save()

# Firefox Private Browsing
$lnk = $shell.CreateShortcut("C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Firefox Private Browsing.lnk")
$lnk.TargetPath = "powershell.exe"
$lnk.Arguments = '-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "C:\Users\ericm\.claude\hooks\browser-unlock.ps1" -private-window'
$lnk.IconLocation = "C:\Program Files\Mozilla Firefox\private_browsing.exe,0"
$lnk.Save()
```

---

## Linux/Fedora Version

### How It Works

1. Bash script at `~/.local/bin/browser-unlock`
2. Checks for a session token in `/tmp/`
3. If session is active, launches Firefox directly
4. Otherwise shows a `zenity` password dialog
5. On correct password, writes session token, launches Firefox, deletes token on close
6. A `.desktop` override in `~/.local/share/applications/` replaces the system launcher

### Script (`browser-unlock`)

```bash
#!/bin/bash

SESSION_FILE="/tmp/browser_unlocked_$(whoami)"
SESSION_DURATION=1800
STORED_HASH="bbac07878d5ebfc9079776ce25119974e7f7bb55f9bbad42195d6ee57f3fcc87"
BROWSER="/usr/bin/firefox"

if [ -f "$SESSION_FILE" ]; then
    UNLOCK_TIME=$(cat "$SESSION_FILE")
    NOW=$(date +%s)
    ELAPSED=$((NOW - UNLOCK_TIME))
    if [ "$ELAPSED" -lt "$SESSION_DURATION" ]; then
        exec "$BROWSER" "$@"
    fi
fi

PASSWORD=$(zenity --password --title="Browser Access" --text="Enter password to open browser:" 2>/dev/null)

if [ $? -ne 0 ]; then
    exit 1
fi

HASH=$(echo -n "$PASSWORD" | sha256sum | cut -d' ' -f1)

if [ "$HASH" = "$STORED_HASH" ]; then
    date +%s > "$SESSION_FILE"
    "$BROWSER" "$@"
    rm -f "$SESSION_FILE"
else
    zenity --error --title="Access Denied" --text="Incorrect password." --width=200 2>/dev/null
    exit 1
fi
```

### Desktop Launcher Override

```bash
sed 's|Exec=firefox|Exec=/home/USERNAME/.local/bin/browser-unlock|g' \
    /usr/share/applications/org.mozilla.firefox.desktop \
    > ~/.local/share/applications/org.mozilla.firefox.desktop

update-desktop-database ~/.local/share/applications/
```

---

## Changing the Password

The script never stores the real password — only a SHA256 hash of it. To change it, generate a new hash and drop it into the `STORED_HASH` (Linux) / `$StoredHash` (Windows) line.

### Linux/Fedora — step by step

1. Hash the new password:
   ```bash
   echo -n 'YourNewPassword' | sha256sum
   ```
   Copy the long hex string (ignore the trailing `-`).
2. Open the script: `nano ~/.local/bin/browser-unlock`
3. Find the line `STORED_HASH="..."` and replace the string between the quotes with the new hash.
4. Save and exit: `Ctrl+O`, `Enter`, `Ctrl+X`.

One-liner that does steps 2-4 automatically:
```bash
NEW_HASH=$(echo -n 'YourNewPassword' | sha256sum | cut -d' ' -f1)
sed -i "s/STORED_HASH=\".*\"/STORED_HASH=\"$NEW_HASH\"/" ~/.local/bin/browser-unlock
```

### Windows — step by step

1. Hash the new password:
   ```powershell
   $hash = [System.BitConverter]::ToString([System.Security.Cryptography.SHA256]::Create().ComputeHash([System.Text.Encoding]::UTF8.GetBytes("YourNewPassword"))).Replace("-","").ToLower()
   $hash
   ```
2. Open the script: `notepad "C:\Users\ericm\.claude\hooks\browser-unlock.ps1"`
3. Find the line `$StoredHash = "..."` and replace the string between the quotes with the new hash.
4. Save (`Ctrl+S`) and close Notepad.

One-liner that does steps 2-4 automatically:
```powershell
(Get-Content "C:\Users\ericm\.claude\hooks\browser-unlock.ps1") -replace '\$StoredHash = ".*"', '$StoredHash = "NEW_HASH_HERE"' | Set-Content "C:\Users\ericm\.claude\hooks\browser-unlock.ps1"
```

### macOS (Rosemary's MacBook)

Same idea, but hash with `shasum -a 256` and edit `~/.local/bin/browser-unlock`, line `STORED_HASH="..."`.

**Remember:** Eric's Windows laptop and Fedora machine share the same password — update both when changing it. Rosemary's MacBook uses its own separate password.

## Removing It

### Windows
```powershell
# Delete wrapper script
Remove-Item "C:\Users\ericm\.claude\hooks\browser-unlock.ps1"

# Re-point shortcuts back to firefox.exe (taskbar, Start Menu)
```

### Linux
```bash
rm ~/.local/bin/browser-unlock
rm ~/.local/share/applications/org.mozilla.firefox.desktop
update-desktop-database ~/.local/share/applications/
```

## macOS Version (Rosemary's MacBook)

### How It Works

1. Bash script at `~/.local/bin/browser-unlock` takes browser name as first arg (`safari` or `firefox`)
2. Checks for a session token in `/tmp/`
3. If session is active, launches the browser directly via `open -a`
4. Otherwise shows a native macOS password dialog (`osascript`)
5. On correct password, writes session token, launches browser, deletes token when browser closes
6. Wrapper `.app` bundles in `~/Applications/` replace the Dock icons

### Script (`browser-unlock`)

```bash
#!/bin/bash

SESSION_FILE="/tmp/browser_unlocked_$(whoami)"
SESSION_DURATION=1800
STORED_HASH="a36bfa3cafb40c3d89ad4b8a9d6e2bdb14a8a97fbcd92780ab5fe87163bd1dbe"

BROWSER_NAME="$1"
shift

if [ "$BROWSER_NAME" = "safari" ]; then
    BROWSER_PATH="/Applications/Safari.app"
    BROWSER_PROC="Safari"
else
    BROWSER_PATH="/Applications/Firefox.app"
    BROWSER_PROC="firefox"
fi

if [ -f "$SESSION_FILE" ]; then
    UNLOCK_TIME=$(cat "$SESSION_FILE")
    NOW=$(date +%s)
    ELAPSED=$((NOW - UNLOCK_TIME))
    if [ "$ELAPSED" -lt "$SESSION_DURATION" ]; then
        open "$BROWSER_PATH" --args "$@"
        exit 0
    fi
fi

PASSWORD=$(osascript -e 'display dialog "Enter password to open browser:" default answer "" with hidden answer buttons {"Cancel", "OK"} default button "OK" with title "Browser Access"' -e 'text returned of result' 2>/dev/null)

if [ $? -ne 0 ]; then
    exit 1
fi

HASH=$(echo -n "$PASSWORD" | shasum -a 256 | cut -d' ' -f1)

if [ "$HASH" = "$STORED_HASH" ]; then
    date +%s > "$SESSION_FILE"
    open "$BROWSER_PATH" --args "$@"
    sleep 2
    while pgrep -x "$BROWSER_PROC" > /dev/null; do sleep 5; done
    rm -f "$SESSION_FILE"
else
    osascript -e 'display dialog "Incorrect password." buttons {"OK"} default button "OK" with title "Access Denied" with icon stop' 2>/dev/null
    exit 1
fi
```

### Wrapper App Setup

Each browser gets a minimal `.app` bundle in `~/Applications/` (named `Safari.app` and `Firefox.app` so the Dock shows clean labels):

```bash
# Safari wrapper
mkdir -p ~/Applications/Safari.app/Contents/{MacOS,Resources}
cat > ~/Applications/Safari.app/Contents/MacOS/Safari << 'EOF'
#!/bin/bash
exec ~/.local/bin/browser-unlock safari
EOF
chmod +x ~/Applications/Safari.app/Contents/MacOS/Safari
cp /Applications/Safari.app/Contents/Resources/AppIcon.icns ~/Applications/Safari.app/Contents/Resources/AppIcon.icns

# Firefox wrapper
mkdir -p ~/Applications/Firefox.app/Contents/{MacOS,Resources}
cat > ~/Applications/Firefox.app/Contents/MacOS/Firefox << 'EOF'
#!/bin/bash
exec ~/.local/bin/browser-unlock firefox
EOF
chmod +x ~/Applications/Firefox.app/Contents/MacOS/Firefox
cp /Applications/Firefox.app/Contents/Resources/firefox.icns ~/Applications/Firefox.app/Contents/Resources/AppIcon.icns
```

### Dock Replacement

Use Python to swap the Dock entries, then restart the Dock:

```python
import plistlib, os
dock_plist = os.path.expanduser('~/Library/Preferences/com.apple.dock.plist')
with open(dock_plist, 'rb') as f:
    dock = plistlib.load(f)
# ... modify persistent-apps entries ...
with open(dock_plist, 'wb') as f:
    plistlib.dump(dock, f)
```
Then: `killall Dock`

### Removing It (macOS)

```bash
rm -rf ~/Applications/Safari.app ~/Applications/Firefox.app
rm ~/.local/bin/browser-unlock
# Re-add real browsers to Dock manually or via defaults write
killall Dock
```

---

## Fully Disabling a Browser (Windows)

For a secondary browser a kid shouldn't use at all (e.g. Edge on a laptop where Firefox is the wrapped, approved browser), it's simpler and more durable to disable it outright rather than wrap it — one less password dialog to maintain, and no shortcut anywhere can accidentally bypass it.

Requires an admin account (check with `whoami /groups` — SSH sessions to an admin-group Windows account over OpenSSH get a real elevated token, no UAC prompt needed).

```powershell
# 1. Disable Edge's self-update so Windows doesn't silently restore it
Get-ScheduledTask | Where-Object { $_.TaskName -match "MicrosoftEdgeUpdate" } |
    ForEach-Object { Disable-ScheduledTask -TaskName $_.TaskName -TaskPath $_.TaskPath }
Stop-Service -Name "edgeupdate","edgeupdatem" -Force -ErrorAction SilentlyContinue
Set-Service -Name "edgeupdate" -StartupType Disabled -ErrorAction SilentlyContinue
Set-Service -Name "edgeupdatem" -StartupType Disabled -ErrorAction SilentlyContinue

# 2. Take ownership and rename the real binary (Edge keeps versioned subfolders like
#    150.0.xxxx.xx\ — but the top-level msedge.exe is the actual launcher, not a stub,
#    so renaming just this one file is enough to kill every launch path)
$exe = "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
takeown /F $exe /A
icacls $exe /grant "Administrators:F"
Rename-Item -Path $exe -NewName "msedge.exe.blocked_by_parent" -Force

# 3. Belt-and-suspenders: block it at the shell level too, in case the binary
#    ever comes back via a Windows Update component repair
$key = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer"
New-Item -Path $key -Force | Out-Null
New-ItemProperty -Path $key -Name "DisallowRun" -Value 1 -PropertyType DWord -Force
New-Item -Path "$key\DisallowRun" -Force | Out-Null
New-ItemProperty -Path "$key\DisallowRun" -Name "1" -Value "msedge.exe" -PropertyType String -Force
```

**Gotcha:** Edge sometimes already has a stale `msedge.exe.disabled` file sitting in the Application folder from its own in-place update process (a leftover previous-version binary, different file size, owned by `NT AUTHORITY\SYSTEM`) — this is normal and unrelated to this lockdown. Pick a unique name for your renamed file (`msedge.exe.blocked_by_parent`) so `Rename-Item` doesn't collide with it.

**To reverse:** re-enable the two scheduled tasks and the `edgeupdate`/`edgeupdatem` services, rename `msedge.exe.blocked_by_parent` back to `msedge.exe`, and remove the `DisallowRun` registry value.

**Verify it's actually dead**, don't just trust the rename:
```powershell
Start-Process "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" -ErrorAction Stop
# should throw "The system cannot find the file specified."
```

---

## Claude Code — Bypass the Lock

When Claude Code needs to open a URL, call Firefox directly to skip the password prompt:

```powershell
# Windows — use this, NOT Start-Process "http://..."
Start-Process "C:\Program Files\Mozilla Firefox\firefox.exe" -ArgumentList "http://example.com"
```

```bash
# Linux — terminal launches already bypass the .desktop override
firefox "http://example.com"
```

## Notes

- Only locks **GUI launchers** (shortcuts/desktop entries). Running `firefox` from a terminal bypasses it.
- Session token is cleared on reboot (lives in temp directory)
- Windows version uses Windows Forms for the dialog. Linux version uses zenity (pre-installed on Fedora/GNOME). macOS version uses `osascript` (native AppleScript dialog).
- To lock terminal launches too: add a shell alias pointing to the wrapper script
- Adaptable to Chromium/Chrome by changing the browser path and targeting the correct shortcut/desktop file
- On Windows, audit **every** shortcut location, not just taskbar and Start Menu — `C:\Users\Public\Desktop\*.lnk` and `C:\Users\<user>\Desktop\*.lnk` are easy to miss and were the actual bypass on Benedict's laptop (2026-08-06)
- A stray, unwrapped browser (Edge, IE) is itself a bypass even if Firefox is locked down — either wrap it too or disable it outright (see "Fully Disabling a Browser" above)
