# Claude Code Windows Performance Tuning

## Problem

Claude Code runs noticeably slower on Windows than macOS, even on faster hardware. The bottleneck is local overhead, not the API.

## Root Causes

### 1. Windows Defender Real-Time Scanning (Biggest Impact)
Claude Code reads, writes, and searches files constantly. Defender inspects every single operation. On macOS, XProtect is far less aggressive.

### 2. PowerShell Startup Time
Every tool call that runs a shell command spins up a PowerShell process (~300ms vs ~50ms for bash/zsh on macOS). Adds up across dozens of calls per session.

### 3. NTFS vs APFS
File search (glob/grep) and small-file I/O is measurably slower on NTFS, especially in directories with many files.

## Fix: Add Defender Exclusions (Permanent)

Run as admin:

```powershell
Add-MpPreference -ExclusionPath "C:\Users\ericm"
Add-MpPreference -ExclusionProcess "claude.exe"
Add-MpPreference -ExclusionProcess "node.exe"
Add-MpPreference -ExclusionProcess "powershell.exe"
```

### What This Does
- **Path exclusion:** Stops Defender from scanning all files under the home directory (projects, `.claude`, skills, etc.)
- **Process exclusions:** Stops Defender from scanning Claude Code, Node.js, and PowerShell on every launch

### Applied: 2026-06-11
All four exclusions are active on Eric's machine.

## Verify Exclusions

```powershell
# Run as admin
(Get-MpPreference).ExclusionPath
(Get-MpPreference).ExclusionProcess
```

## Undo If Needed

```powershell
# Run as admin
Remove-MpPreference -ExclusionPath "C:\Users\ericm"
Remove-MpPreference -ExclusionProcess "claude.exe"
Remove-MpPreference -ExclusionProcess "node.exe"
Remove-MpPreference -ExclusionProcess "powershell.exe"
```

These exclusions are permanent, persisting across reboots and Windows updates. They're stored in the registry and stay until manually removed.
