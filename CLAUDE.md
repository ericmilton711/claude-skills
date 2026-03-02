# Claude Instructions for Eric

## Skills Management

**GitHub Repository:** https://github.com/ericmilton711/claude-skills

**Local Setup:**
- Git repo: `C:\Users\ericm\claude-skills\`
- Junctions link `~/.claude/skills/` and `~/.claude/commands/` to the git repo

**When user says "save to skills" or "save and document this to skills":**
1. Save the content to `~/.claude/skills/<skill-name>/SKILL.md`
2. **Immediately** commit and push to GitHub:
   ```bash
   cd ~/claude-skills && git add -A && git commit -m "Add <skill-name>" && git push
   ```
3. Do NOT wait for a separate "push" command - auto-push is always on

## GitHub Account

- Username: ericmilton711
- Email: ericmilton711@gmail.com

## Git Config

```bash
git config --global user.email "ericmilton711@gmail.com"
git config --global user.name "ericm"
```

## System Optimizations Applied (2026-02-26)

**Machine:** Intel i5-2415M (2011), 16GB RAM, Fedora 43 with GNOME

**Optimizations applied:**
1. GNOME animations disabled (`gsettings set org.gnome.desktop.interface enable-animations false`)
2. GNOME Software autostart disabled (created `~/.config/autostart/gnome-software-service.desktop` with `Hidden=true`)
3. Unnecessary services disabled at boot:
   - `abrt-journal-core`, `abrt-oops`, `abrt-xorg`, `abrtd` (crash reporters)
   - `ModemManager` (mobile broadband)
   - `iscsi-onboot`, `iscsi-starter` (network storage)
   - `livesys`, `livesys-late` (live USB)
   - `qemu-guest-agent` (VM guest)
   - `mcelog` (hardware error logging)

**To re-enable animations if needed:**
```bash
gsettings set org.gnome.desktop.interface enable-animations true
```

**Additional tips suggested:**
- Close unused Firefox tabs (each uses ~100-150MB)
- Consider SSD upgrade if on HDD
