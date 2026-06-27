# Firefox Browser Lock — Password-Gated Browser Launch

Locks Firefox (or any browser) behind a password prompt. Password is required every time Firefox is launched. The session file is deleted when Firefox closes, so closing and reopening always requires the password again. 30-minute session window so links opened mid-browsing don't re-prompt.

**Password hash:** `1e4ea3ac36db7cc72af8f0942409942b4fe9c3c79937c6905527c2032ed67504`

---

## Deployed On

### Eric's Windows Laptop (192.168.12.220)
- **Script:** `C:\Users\ericm\.claude\hooks\browser-unlock.ps1`
- **Shortcuts rewired:** Taskbar pin, Start Menu (Firefox + Firefox Private Browsing)
- **Deployed:** 2026-06-27

### MacBook Pro — Fedora (192.168.12.190)
- **Script:** `~/.local/bin/browser-unlock`
- **Desktop override:** `~/.local/share/applications/org.mozilla.firefox.desktop`
- **Deployed:** 2026-06-26

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
$StoredHash = "1e4ea3ac36db7cc72af8f0942409942b4fe9c3c79937c6905527c2032ed67504"
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
STORED_HASH="1e4ea3ac36db7cc72af8f0942409942b4fe9c3c79937c6905527c2032ed67504"
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

## Generating a New Password Hash

```bash
# Linux
echo -n "YOUR_PASSWORD" | sha256sum | cut -d' ' -f1

# PowerShell
[System.BitConverter]::ToString([System.Security.Cryptography.SHA256]::Create().ComputeHash([System.Text.Encoding]::UTF8.GetBytes("YOUR_PASSWORD"))).Replace("-","").ToLower()
```

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

## Notes

- Only locks **GUI launchers** (shortcuts/desktop entries). Running `firefox` from a terminal bypasses it.
- Session token is cleared on reboot (lives in temp directory)
- Windows version uses Windows Forms for the dialog. Linux version uses zenity (pre-installed on Fedora/GNOME).
- To lock terminal launches too: add a shell alias pointing to the wrapper script
- Adaptable to Chromium/Chrome by changing the browser path and targeting the correct shortcut/desktop file
