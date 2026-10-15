# Windows Laptop — Wi-Fi Band Switching

Eric's Lenovo (Windows 11, MediaTek Wi-Fi 6 MT7921, interface "Wi-Fi 4").

## Current State
**2.4 GHz Only** as of 2026-10-14. Was on 5 GHz since 2026-10-12, switched back due to poor 5 GHz connectivity.

## Diagnosis
```powershell
netsh wlan show interfaces
```

```powershell
Get-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Band Selection" | Format-List DisplayValue
```

## Switching Bands

Requires elevated PowerShell. **Important:** the `-DisplayValue` must include the numbered prefix.

Valid values:
- `"1. Dual Band"` — auto-select
- `"2. 2.4G Only"` — 2.4 GHz only
- `"3. 5G Only"` — 5 GHz only

### Recommended method: script file (avoids quoting issues)

Write a .ps1 file, then run it elevated:
```powershell
# Write the script
Set-Content "$env:TEMP\set_wifi_band.ps1" 'Set-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Band Selection" -DisplayValue "2. 2.4G Only"'

# Run elevated
Start-Process powershell -Verb RunAs -Wait -ArgumentList '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "$env:TEMP\set_wifi_band.ps1"
```

Replace the `-DisplayValue` with whichever band is needed.

### Direct elevated command (if in an admin shell already)
```powershell
Set-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Band Selection" -DisplayValue "2. 2.4G Only"
```

## Gotchas
- **Quoting matters:** passing the command via `-ArgumentList` to `Start-Process` strips quotes. The .ps1 script method avoids this.
- **Value format:** using `"2.4G Only"` without the `"2. "` prefix fails with "No matching display value found."
- UAC prompt must be accepted for elevation to work. If the elevated process runs silently with no change, UAC was dismissed.

## Verify
```powershell
Get-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Band Selection" | Format-List DisplayValue
netsh wlan show interfaces | Select-String "Band|Channel|Receive|Transmit|Signal"
```

## Persistence
- Stored in adapter advanced properties. Survives reboots, sleep, WiFi reconnects.
- Lost if: driver reinstalled, adapter reset to defaults, or OS wiped.

## Notes
- MacBook Pro (Fedora) also has band lock. See `macbook-wifi-5ghz-lock` skill.
- Router is T-Mobile gateway, SSID DIEMILTONHAUS, broadcasts both bands on same SSID.
- 5 GHz: faster (~144 Mbps) but weaker signal (~33%). 2.4 GHz: slower (~17 Mbps) but more reliable.
