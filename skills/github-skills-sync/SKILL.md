# GitHub Skills Sync

Sync Claude Code skills and commands across multiple devices using GitHub.

## Repository

**GitHub:** https://github.com/ericmilton711/claude-skills

## Skills Locations on This Computer

| Location | Type | Points To |
|----------|------|-----------|
| `C:\Users\ericm\claude-skills\` | Git repo | GitHub (source of truth) |
| `C:\Users\ericm\.claude\skills\` | Junction | → `claude-skills\skills\` |
| `C:\Users\ericm\.claude\commands\` | Junction | → `claude-skills\commands\` |

**Note:** Junctions were set up so saving to `~/.claude/skills/` automatically saves to the git repo. No manual copying needed.

## Current Skills Inventory

### Skills
| Name | Description |
|------|-------------|
| midi-keyboard | MIDI Keyboard 3D printing project with Cherry MX switches |
| 3d-printing/midi-piano-project | MIDI Piano design documentation (25-key, 2 octave) |
| esp32-door-led-strip | ESP32 door LED strip project with PlatformIO |
| esp32-cam-pan-tilt | ESP32 camera with pan/tilt servo control |
| galaxy-watch-dock | Galaxy watch charging dock Arduino project |
| printing-from-claude | Printing utilities |
| github-skills-sync | This file - documents how to sync skills |

### Commands
| Name | Description |
|------|-------------|
| esp32-closet-lights | ESP32 LED closet lighting system with PIR motion sensor |

## Setup (New Device)

### 1. Install Git and GitHub CLI
```bash
# Windows (winget)
winget install Git.Git
winget install GitHub.cli

# macOS (homebrew)
brew install git gh
```

### 2. Authenticate with GitHub
```bash
gh auth login --web --git-protocol https
```

### 3. Clone the Repository
```bash
git clone https://github.com/ericmilton711/claude-skills.git
```

### 4. Set Up Junctions (Recommended - Windows)

Junctions link Claude's config to the git repo so everything stays in sync automatically.

```cmd
mklink /J C:\Users\ericm\.claude\skills C:\Users\ericm\claude-skills\skills
mklink /J C:\Users\ericm\.claude\commands C:\Users\ericm\claude-skills\commands
```

**macOS/Linux (symlinks):**
```bash
ln -s ~/claude-skills/skills ~/.claude/skills
ln -s ~/claude-skills/commands ~/.claude/commands
```

### Alternative: Copy Files (if junctions not desired)
```bash
mkdir -p ~/.claude/skills ~/.claude/commands
cp -r claude-skills/skills/* ~/.claude/skills/
cp -r claude-skills/commands/* ~/.claude/commands/
```

## Syncing Skills

### Push Skills to GitHub
When user says **"save to skills"** or **"save and document this to skills"**, Claude will:
1. Save the skill file to `~/.claude/skills/`
2. **Immediately** commit and push to GitHub (no extra command needed)

Manual push if needed:
```bash
cd ~/claude-skills
git add -A
git commit -m "Add new skills"
git push
```

### Pull Skills from GitHub
```bash
cd ~/claude-skills
git pull
```
Skills are immediately available (junctions point to the repo).

## GitHub Account

- **Username:** ericmilton711
- **Email:** ericmilton711@gmail.com

## Git Configuration

```bash
git config --global user.email "ericmilton711@gmail.com"
git config --global user.name "ericm"
```

## Troubleshooting

### "Repository not found" error
Your GitHub username is `ericmilton711`, not `ericm`. Use:
```bash
git remote set-url origin https://github.com/ericmilton711/claude-skills.git
```

### Authentication issues
GitHub no longer accepts passwords. Use:
```bash
gh auth login --web
```
This opens a browser for secure authentication.

### Check authentication status
```bash
gh auth status
```

## Conversation Log (2026-02-14)

This skill was created during a conversation where:

1. User requested to save all Skills to GitHub for cross-device access
2. Found skills in multiple locations:
   - `C:\Users\ericm\.claude\skills\` (2 skills)
   - `C:\Users\ericm\.claude\commands\` (1 command)
   - `C:\Users\ericm\skills\` (4 projects)
3. Installed GitHub CLI via winget
4. Authenticated with GitHub (opened browser for secure login)
5. Created repository: https://github.com/ericmilton711/claude-skills
6. Pushed all skills (19 files total)
7. Deleted duplicate local folders (kept only git repo)
8. Created Windows junctions so `~/.claude/skills` → git repo
9. "Save to skills" now auto-saves AND auto-pushes to GitHub immediately

### Files Uploaded
```
skills/
├── 3d-printing/
│   └── midi-piano-project.md
├── midi-keyboard/
│   └── SKILL.md
├── esp32-door-led-strip/
│   ├── README.md
│   ├── parts-list.txt
│   ├── platformio.ini
│   └── src/
│       ├── config.h
│       └── main.cpp
├── esp32-cam-pan-tilt/
│   ├── README.md
│   ├── SETUP_GUIDE.md
│   ├── wiring_guide.txt
│   ├── arduino_joystick/
│   │   └── arduino_joystick.ino
│   └── esp32_cam_pan_tilt/
│       └── esp32_cam_pan_tilt.ino
├── galaxy-watch-dock/
│   ├── PROJECT_LOG.md
│   ├── WIRING.txt
│   └── galaxy_watch_dock/
│       └── galaxy_watch_dock.ino
├── printing-from-claude.md
└── github-skills-sync/
    └── SKILL.md (this file)

commands/
└── esp32-closet-lights.md
```
