# Uninstall Unauthorized Apps

Finds and uninstalls apps that appeared on the desktop without authorization, based on install date.

## Steps

1. **Take a screenshot** to see the current desktop state.

2. **Find recently installed apps** via registry (adjust date as needed):
```powershell
Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*', 'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*', 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*' | Where-Object { $_.InstallDate -ge 'YYYYMMDD' } | Select-Object DisplayName, InstallDate, Publisher | Sort-Object InstallDate -Descending | Format-Table -AutoSize
```

3. **Find recently added desktop shortcuts** (better for apps that don't register InstallDate):
```powershell
Get-ChildItem 'C:\Users\Public\Desktop', 'C:\Users\<user>\Desktop' -ErrorAction SilentlyContinue | Select-Object Name, LastWriteTime, FullName | Sort-Object LastWriteTime -Descending | Format-Table -AutoSize
```

4. **Find uninstall strings** for the offending apps:
```powershell
Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*', 'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*', 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*' | Where-Object { $_.DisplayName -like '*AppName*' } | Select-Object DisplayName, UninstallString, QuietUninstallString | Format-List
```

5. **Run each uninstall string**, then **delete leftover desktop shortcuts**:
```powershell
Start-Process -FilePath "cmd.exe" -ArgumentList '/c "<UninstallString>"' -Wait
Remove-Item 'C:\Users\Public\Desktop\AppName.lnk' -Force
```

6. **Verify** apps are gone from registry and desktop.

## Notes

- Always write PowerShell logic to `.ps1` files and run with `powershell -ExecutionPolicy Bypass -File` — avoids bash `$_` escaping issues.
- Desktop shortcuts in `C:\Users\Public\Desktop` affect all users; check both Public and user-specific desktop.
- NCH Software is known for bundling multiple apps in a single installer — check what was downloaded around the install time.
- Registry `InstallDate` field is often blank for NCH and similar bundled apps; use desktop shortcut `LastWriteTime` as a fallback.
