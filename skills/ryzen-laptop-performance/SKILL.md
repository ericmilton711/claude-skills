# Ryzen Laptop Performance Tuning

Boost AMD Ryzen laptop CPU performance by adjusting power limits using RyzenAdj, cleaning up startup bloat, and removing adware.

## Machine

Lenovo 83CR (IdeaPad V15 G4 ABP), AMD Ryzen 5 7530U (6c/12t), 38 GB RAM, 1.86 TB SSD

## What Was Done (2026-08-03)

### 1. RyzenAdj — CPU Power Boost (ACTIVE)

Raises sustained power limits so the CPU holds boost clocks longer.

- **Installed:** `C:\Users\ericm\RyzenAdj\ryzenadj.exe` (v0.16.0)
- **Settings:** `--stapm-limit=28000 --fast-limit=38000 --slow-limit=30000 --tctl-temp=95`
- **Batch file:** `C:\Users\ericm\RyzenAdj\apply-settings.bat`
- **Scheduled task:** "RyzenAdj Performance" re-applies on login and wake from sleep
- **Power plan:** Balanced (Eric chose not to switch to Ultimate Performance)

### 2. Startup Programs Disabled (7 items)

Removed from HKCU\Run or startup folders:

1. Docker Desktop — heavy, ~1 GB RAM
2. OneDrive — Eric doesn't use OneDrive
3. MS Teams — auto-start not needed
4. Adobe Acrobat Synchronizer — unnecessary
5. MuseHub — music software, not needed at boot
6. WisprFlow (Wispr Flow) — AI transcription, not needed at boot
7. Ableton USB Audio Control Panel — shortcut removed from ProgramData startup folder

### 3. What Still Starts on Boot (kept)

- SecurityHealthSystray (Windows Defender tray icon)
- RtkAudUService (Realtek audio driver)
- map_shared_drive.bat (user's network drive mapping)
- ExpressVPN services

### 4. Malware/Adware Cleanup

- Windows Defender detected `VirTool:Win32/DefenderTamperingRestore` (something disabled behavior monitoring)
- Defender caught and restored itself automatically
- **Adaware Privacy** (v2.3.0.539) — formerly legit antivirus, now adware. Auto-starting on boot. Processes killed, removed from startup. Folder may still exist under `C:\Program Files (x86)\Adaware` — uninstall from Settings > Apps if present.
- **SoftLanding scheduled tasks** — three adware tasks removed (CreativeManagement, Deferral, Trigger)

## Commands

### Check Current RyzenAdj Values
```powershell
C:\Users\ericm\RyzenAdj\ryzenadj.exe --info
```

### Manually Re-Apply Settings (Run as Admin)
```powershell
C:\Users\ericm\RyzenAdj\ryzenadj.exe --stapm-limit=28000 --fast-limit=38000 --slow-limit=30000 --tctl-temp=95
```

### Check Startup Entries
```powershell
Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run"
Get-ChildItem "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup"
```

### Check System Health
```powershell
# CPU and RAM
$cpu = (Get-CimInstance Win32_Processor).LoadPercentage
$mem = Get-CimInstance Win32_OperatingSystem
$usedGB = [math]::Round(($mem.TotalVisibleMemorySize - $mem.FreePhysicalMemory) / 1MB, 1)
$totalGB = [math]::Round($mem.TotalVisibleMemorySize / 1MB, 1)
"CPU: $cpu% | RAM: $usedGB / $totalGB GB"

# Top processes
Get-Process | Sort-Object WorkingSet64 -Descending | Select-Object -First 10 Name, @{N='MB';E={[math]::Round($_.WorkingSet64/1MB,0)}} | Format-Table
```

### Check Defender Threat History
```powershell
Get-MpThreatDetection | Select-Object -First 5 ThreatID, InitialDetectionTime, Resources
```

## RyzenAdj Parameters Reference

| Parameter | Description | Default (U-series) | Eric's Setting |
|-----------|-------------|-------------------|----------------|
| `--stapm-limit` | Sustained power (mW) | ~15000 | 28000 |
| `--fast-limit` | Burst power (mW) | ~25000 | 38000 |
| `--slow-limit` | Post-burst power (mW) | ~18000 | 30000 |
| `--tctl-temp` | Max temp before throttle (C) | 90 | 95 |

## Quick Benchmark

```powershell
$start = Get-Date
for ($i = 1; $i -le 100000; $i++) { $r = [math]::Sqrt($i) * [math]::PI }
$time1 = ((Get-Date) - $start).TotalMilliseconds
$start = Get-Date
$primes = 0
for ($i = 2; $i -lt 20000; $i++) {
    $isPrime = $true
    for ($j = 2; $j -le [math]::Sqrt($i); $j++) {
        if ($i % $j -eq 0) { $isPrime = $false; break }
    }
    if ($isPrime) { $primes++ }
}
$time2 = ((Get-Date) - $start).TotalMilliseconds
"Math ops: $time1 ms | Primes: $time2 ms | Total: $($time1 + $time2) ms"
```

## Auto-Apply on Startup/Wake

Batch file at `C:\Users\ericm\RyzenAdj\apply-settings.bat`:
```batch
@echo off
cd /d C:\Users\ericm\RyzenAdj
ryzenadj.exe --stapm-limit=28000 --fast-limit=38000 --slow-limit=30000 --tctl-temp=95
```

Scheduled task triggers on logon and on wake from sleep (Event ID 1 from Microsoft-Windows-Power-Troubleshooter). Runs as highest available privileges.

## Files Location

```
C:\Users\ericm\RyzenAdj\
├── ryzenadj.exe
├── apply-settings.bat
├── RyzenAdjTask.xml
└── (supporting DLLs)
```

## Warnings

- Higher power limits = more heat and less battery life
- Monitor temps with `ryzenadj.exe --info` (THM VALUE CORE)
- If temps exceed 90C sustained, reduce limits
- Settings reset on reboot/sleep without the scheduled task
- After BIOS or major Windows updates, verify the scheduled task still works

## Compatibility

- **Supported CPUs:** AMD Ryzen 2000-7000 series mobile (Raven Ridge through Phoenix)
- **Tested on:** Ryzen 5 7530U (Cezanne/Zen 3)
- **OS:** Windows 10/11
