# CLICK PLC Driver - Stop Weekly Reinstall (Windows 11)

## Problem
AutomationDirect CLICK PLUS PLC driver (v1.0.0.0, dated 2013) gets wiped and requires manual reinstall weekly.

Two possible culprits:
1. **Windows Update** replacing the driver with its own version
2. **CLICK software itself** reinstalling its bundled driver on launch

---

## Diagnostic Scripts

### Find the PLC Driver
```powershell
# Save as plc_check.ps1
Get-WmiObject Win32_PnPSignedDriver | Where-Object {
    $_.DeviceName -like '*CLICK*' -or
    $_.DeviceName -like '*FTDI*' -or
    $_.DeviceName -like '*USB Serial*' -or
    $_.Manufacturer -like '*Automation*'
} | Select-Object DeviceName, DriverVersion, DriverDate, Manufacturer | Format-List
```

### Full Diagnostic (scheduled tasks, event logs, install locations)
```powershell
# Save as plc_diagnose.ps1
Write-Host "=== Scheduled Tasks ===" -ForegroundColor Cyan
Get-ScheduledTask | Where-Object {
    $_.TaskName -like '*CLICK*' -or
    $_.TaskName -like '*Automation*' -or
    ($_.Actions | ForEach-Object { $_.Execute }) -like '*CLICK*'
} | Select-Object TaskName, TaskPath, State | Format-Table -AutoSize

Write-Host "`n=== Recent Driver Install Events ===" -ForegroundColor Cyan
Get-WinEvent -LogName "Microsoft-Windows-DeviceSetupManager/Operational" -ErrorAction SilentlyContinue |
    Where-Object { $_.Message -like '*CLICK*' -or $_.Message -like '*AutomationDirect*' } |
    Select-Object TimeCreated, Id, Message | Format-List

Write-Host "`n=== CLICK Driver Files ===" -ForegroundColor Cyan
Get-ChildItem "C:\Program Files (x86)\AutomationDirect" -Recurse -Include "*.inf","*.sys","*.cat" -ErrorAction SilentlyContinue |
    Select-Object FullName, LastWriteTime | Format-Table -AutoSize
```

### Watch for Live Driver Reinstall Activity
```powershell
# Save as plc_watch.ps1 — run this, then open CLICK software or plug in PLC
$logPath = "C:\Windows\INF\setupapi.dev.log"
$lastSize = (Get-Item $logPath).Length
Write-Host "Watching for driver activity... (Ctrl+C to stop)" -ForegroundColor Yellow
while ($true) {
    Start-Sleep -Seconds 5
    $newSize = (Get-Item $logPath).Length
    if ($newSize -gt $lastSize) {
        Write-Host "`n*** Driver activity detected at $(Get-Date) ***" -ForegroundColor Red
        Get-Content $logPath -Tail 30 | Select-String -Pattern "CLICK|AutomationDirect|PLUS|C2Int" | ForEach-Object { Write-Host $_ }
        $lastSize = $newSize
    }
}
```

---

## Fix 1: Stop Windows Update from Replacing Drivers

```powershell
# Save as fix_plc_driver.ps1 — run elevated (Admin)
reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate" /v ExcludeWUDriversInQualityUpdate /t REG_DWORD /d 1 /f
Write-Host "Done. Windows Update will no longer replace device drivers." -ForegroundColor Green
```

**To run elevated from Claude:**
```powershell
Start-Process powershell.exe -ArgumentList '-NoProfile -ExecutionPolicy Bypass -File C:\Users\ericm\fix_plc_driver.ps1' -Verb RunAs -Wait
```

**Verify it took:**
```powershell
reg query "HKLM\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate" /v ExcludeWUDriversInQualityUpdate
# Should show: REG_DWORD  0x1
```

---

## Fix 2: If CLICK Software Is the Culprit

If `plc_watch.ps1` shows driver activity when opening CLICK software:

1. Unplug the PLC USB cable
2. In Device Manager, uninstall the CLICK PLUS device (check "Delete driver software")
3. Right-click `C:\Program Files (x86)\AutomationDirect\CLICK USB Driver\PLUS.inf` → **Install**
4. Replug USB — Windows now "owns" the driver and the software's bundled installer won't override it

---

## Fix 3: CLICK Can't Connect Even Though Windows Sees Device (Prolific PL2303 Chip)

**Symptom:** Windows recognizes the PLC (Device Manager shows it), but CLICK software cannot connect.

**Root cause:** The CLICK PLUS USB interface uses a **Prolific PL2303** chip (VID_067B&PID_2303), NOT FTDI. Prolific driver v3.9.x intentionally blocks counterfeit/clone chips — status shows as "Unknown" in Device Manager.

**Diagnosis:**
```powershell
# Run as script (bash shell mangles $_ inline)
Get-PnpDevice | Where-Object { $_.FriendlyName -like '*CLICK*' -or $_.FriendlyName -like '*FTDI*' } | Select-Object FriendlyName, Status
# Look for Status: Unknown = Prolific blocking the chip
```

**Fix: Install PL2303 Legacy Updater**
1. Download: https://github.com/johnstevenson/pl2303-legacy/releases/latest
   - File: `PL2303LegacyUpdaterSetup-1.1.0.exe`
2. Run installer — choose **"Run Program"** to test first
3. In the app, select **"Legacy PL2303 HXA/XA"** (v3.3.11.152, 2010)
4. Click Next — driver is applied immediately
5. Check what COM port the device is now on (may change, e.g. COM10 → COM11)
6. Update CLICK software communication settings to match the new COM port

**Note:** The installer must be run as admin. Claude cannot click through it via input injection due to UAC/UIPI isolation — user must click through the installer wizard manually.

---

## Key Findings on This Machine (Eric's PC)
- Device: `CLICK PLUS` — uses **Prolific PL2303** chip (VID_067B&PID_2303), not FTDI
- Install path: `C:\Program Files (x86)\AutomationDirect\`
- Two versions installed: Ver3.70 and Ver3.90
- Bundled driver files: `CLICK USB Driver\PLUS.inf` and `CLICK C2-Int USB Driver\C2Int.inf`
- Windows Update fix applied: `ExcludeWUDriversInQualityUpdate = 1`
- Helper scripts saved to: `C:\Users\ericm\plc_check.ps1`, `plc_diagnose.ps1`, `plc_watch.ps1`, `fix_plc_driver.ps1`
- PL2303 Legacy Updater installed — legacy driver v3.3.11.152 applied, PLC now on **COM11**
