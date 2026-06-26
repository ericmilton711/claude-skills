# Firefox Browser Lock — Password-Gated Browser Launch

Locks Firefox (or any browser) behind a password prompt on Fedora/Linux. Password is required every time Firefox is launched. The session file is deleted when Firefox closes, so closing and reopening always requires the password again.

## How It Works

1. A wrapper script at `~/.local/bin/browser-unlock` checks for a session token in `/tmp/`
2. If a session token exists (Firefox already running, opened a link mid-session), launches Firefox directly
3. Otherwise shows a `zenity` password dialog
4. On success, writes a session token, launches Firefox, and deletes the token when Firefox closes
5. A `.desktop` override in `~/.local/share/applications/` replaces the system Firefox launcher

## Setup Steps

### 1. Generate password hash
```bash
echo -n "YOUR_PASSWORD" | sha256sum | cut -d' ' -f1
```

### 2. Create wrapper script
Save to `~/.local/bin/browser-unlock` (make executable with `chmod +x`):

```bash
#!/bin/bash

SESSION_FILE="/tmp/browser_unlocked_$(whoami)"
SESSION_DURATION=1800  # 30 minutes
STORED_HASH="<sha256_hash_here>"
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

### 3. Create desktop launcher override
```bash
sed 's|Exec=firefox|Exec=/home/USERNAME/.local/bin/browser-unlock|g' \
    /usr/share/applications/org.mozilla.firefox.desktop \
    > ~/.local/share/applications/org.mozilla.firefox.desktop

update-desktop-database ~/.local/share/applications/
```

## Removing It

```bash
rm ~/.local/bin/browser-unlock
rm ~/.local/share/applications/org.mozilla.firefox.desktop
update-desktop-database ~/.local/share/applications/
```

## Notes

- Only locks the **desktop launcher** — terminal `firefox` command bypasses it
- Session file lives in `/tmp/` and is cleared on reboot
- Works on Fedora with GNOME (Wayland or X11) — requires `zenity` (usually pre-installed)
- To lock terminal launches too, add an alias to `~/.bashrc`: `alias firefox='/home/USERNAME/.local/bin/browser-unlock'`
- Easily adapted for Chromium/Chrome by changing the `BROWSER` variable and targeting the correct `.desktop` file
