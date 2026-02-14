# GitHub Skills Sync

Sync Claude Code skills and commands across multiple devices using GitHub.

## Repository

**GitHub:** https://github.com/ericmilton711/claude-skills

## Skills Locations on This Computer

| Location | Contents |
|----------|----------|
| `C:\Users\ericm\.claude\skills\` | Claude Code skills (midi-keyboard, 3d-printing) |
| `C:\Users\ericm\.claude\commands\` | Claude Code commands (esp32-closet-lights) |
| `C:\Users\ericm\skills\` | Project skills (esp32-door-led-strip, esp32-cam-pan-tilt, galaxy-watch-dock) |
| `C:\Users\ericm\claude-skills\` | Local git repository synced to GitHub |

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

### 4. Copy Skills to Claude Code Directory

**Windows:**
```bash
mkdir -p ~/.claude/skills ~/.claude/commands
cp -r claude-skills/skills/* ~/.claude/skills/
cp -r claude-skills/commands/* ~/.claude/commands/
```

**macOS/Linux:**
```bash
mkdir -p ~/.claude/skills ~/.claude/commands
cp -r claude-skills/skills/* ~/.claude/skills/
cp -r claude-skills/commands/* ~/.claude/commands/
```

## Syncing Skills

### Push New Skills to GitHub
```bash
cd ~/claude-skills

# Copy new skills from Claude directory
cp -r ~/.claude/skills/* skills/
cp -r ~/.claude/commands/* commands/

# Commit and push
git add -A
git commit -m "Add new skills"
git push
```

### Pull Skills from GitHub
```bash
cd ~/claude-skills
git pull

# Copy to Claude directory
cp -r skills/* ~/.claude/skills/
cp -r commands/* ~/.claude/commands/
```

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
