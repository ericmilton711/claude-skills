# Ryzen Laptop Performance Tuning

Boost AMD Ryzen laptop CPU performance by adjusting power limits using RyzenAdj and Windows power settings.

## Overview

AMD Ryzen mobile CPUs (U-series) are power-limited by default for battery life. This skill increases sustained performance by raising power limits, allowing the CPU to maintain boost clocks longer.

## What This Does

1. **Enables Ultimate Performance power plan** (Windows)
2. **Installs RyzenAdj** - command-line tool to adjust CPU power limits
3. **Applies optimized power settings** - increases TDP limits
4. **Creates scheduled task** - reapplies settings on login and wake from sleep

## Commands

### Check System Info
```bash
wmic computersystem get model,manufacturer && wmic cpu get name,maxclockspeed && wmic memorychip get capacity
```

### Enable Ultimate Performance Power Plan
```bash
# Add the plan (hidden by default)
powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61

# Activate it
powercfg -setactive <GUID-from-above>

# Verify
powercfg -list
```

### Install RyzenAdj
```bash
mkdir -p ~/RyzenAdj
curl -L -o ~/RyzenAdj/ryzenadj.zip "https://github.com/FlyGoat/RyzenAdj/releases/download/v0.16.0/ryzenadj-win64.zip"
cd ~/RyzenAdj && powershell -Command "Expand-Archive -Path ryzenadj.zip -DestinationPath . -Force"
```

### Apply Performance Settings
```bash
# Run as Administrator
ryzenadj.exe --stapm-limit=28000 --fast-limit=38000 --slow-limit=30000 --tctl-temp=95
```

### Check Current Values
```bash
ryzenadj.exe --info
```

## RyzenAdj Parameters

| Parameter | Description | Default (U-series) | Suggested |
|-----------|-------------|-------------------|-----------|
| `--stapm-limit` | Sustained power (mW) | ~15000 | 25000-38000 |
| `--fast-limit` | Burst power (mW) | ~25000 | 35000-40000 |
| `--slow-limit` | Post-burst power (mW) | ~18000 | 28000-35000 |
| `--tctl-temp` | Max temp before throttle (°C) | 90 | 90-95 |
| `--vrm-current` | TDC limit (A) | ~24 | 35-45 |
| `--vrmmax-current` | EDC limit (A) | ~36 | 50-65 |

## Auto-Apply on Startup/Wake

### Create Settings Script
Save to `C:\Users\<user>\RyzenAdj\apply-settings.bat`:
```batch
@echo off
cd /d C:\Users\<user>\RyzenAdj
ryzenadj.exe --stapm-limit=28000 --fast-limit=38000 --slow-limit=30000 --tctl-temp=95
```

### Create Scheduled Task XML
Save to `C:\Users\<user>\RyzenAdj\RyzenAdjTask.xml`:
```xml
<?xml version="1.0" encoding="UTF-16"?>
<Task version="1.4" xmlns="http://schemas.microsoft.com/windows/2004/02/mit/task">
  <Triggers>
    <LogonTrigger>
      <Enabled>true</Enabled>
    </LogonTrigger>
    <EventTrigger>
      <Enabled>true</Enabled>
      <Subscription>&lt;QueryList&gt;&lt;Query Id="0" Path="System"&gt;&lt;Select Path="System"&gt;*[System[Provider[@Name='Microsoft-Windows-Power-Troubleshooter'] and EventID=1]]&lt;/Select&gt;&lt;/Query&gt;&lt;/QueryList&gt;</Subscription>
    </EventTrigger>
  </Triggers>
  <Principals>
    <Principal id="Author">
      <LogonType>InteractiveToken</LogonType>
      <RunLevel>HighestAvailable</RunLevel>
    </Principal>
  </Principals>
  <Settings>
    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>
    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>
    <AllowHardTerminate>true</AllowHardTerminate>
    <StartWhenAvailable>true</StartWhenAvailable>
    <Enabled>true</Enabled>
    <ExecutionTimeLimit>PT1M</ExecutionTimeLimit>
  </Settings>
  <Actions Context="Author">
    <Exec>
      <Command>C:\Users\&lt;user&gt;\RyzenAdj\apply-settings.bat</Command>
      <WorkingDirectory>C:\Users\&lt;user&gt;\RyzenAdj</WorkingDirectory>
    </Exec>
  </Actions>
</Task>
```

### Import Task (Run as Admin)
```powershell
schtasks /create /tn "RyzenAdjPerformance" /xml "C:\Users\<user>\RyzenAdj\RyzenAdjTask.xml" /f
```

## Quick Benchmark Script

Save to `benchmark.ps1`:
```powershell
Write-Host "=== CPU Benchmark ===" -ForegroundColor Cyan

# Math operations
$start = Get-Date
for ($i = 1; $i -le 100000; $i++) { $r = [math]::Sqrt($i) * [math]::PI }
$time1 = ((Get-Date) - $start).TotalMilliseconds
Write-Host "Math ops (100k): $time1 ms"

# Prime numbers
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
Write-Host "Primes to 20k: $time2 ms ($primes found)"

Write-Host "Total: $($time1 + $time2) ms"
```

Run with: `powershell -ExecutionPolicy Bypass -File benchmark.ps1`

## Compatibility

- **Supported CPUs**: AMD Ryzen 2000-7000 series mobile (Raven Ridge through Phoenix)
- **Tested on**: Ryzen 5 7530U (Cezanne/Zen 3)
- **OS**: Windows 10/11

## Warnings

- Higher power limits = more heat and less battery life
- Monitor temps with `ryzenadj.exe --info` (THM VALUE CORE)
- If temps exceed 90°C sustained, reduce limits
- Settings reset on reboot/sleep without the scheduled task

## Files Location

```
C:\Users\<user>\RyzenAdj\
├── ryzenadj.exe
├── apply-settings.bat
├── RyzenAdjTask.xml
├── benchmark.ps1
└── (supporting DLLs)
```
