# Windows Laptop — Wi-Fi 5 GHz Band Lock

Eric's Lenovo (Windows 11, MediaTek Wi-Fi 6 MT7921, interface "Wi-Fi 4"). Applied 2026-10-12.

## Diagnosis
```powershell
netsh wlan show interfaces
```
Finding: laptop was on DIEMILTONHAUS 2.4 GHz (ch 1), 17 Mbps transmit. Switched to 5 GHz (ch 44), jumped to ~144 Mbps.

## Fix: lock adapter to 5 GHz only
Requires elevated PowerShell:
```powershell
Set-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Band Selection" -DisplayValue "3. 5G Only"
```

## Verify
```powershell
netsh wlan show interfaces | Select-String "Band|Channel|Receive|Transmit|Signal"
# Expect: Band = 5 GHz, Channel = 44
```

## Persistence
- Stored in the adapter's advanced properties. Survives reboots, sleep, WiFi reconnects.
- Lost if: driver is reinstalled, adapter is reset to defaults, or OS is wiped.
- After a driver reinstall: re-run the elevated `Set-NetAdapterAdvancedProperty` command above.

## Undo (if signal too weak in far rooms)
Elevated PowerShell:
```powershell
Set-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Band Selection" -DisplayValue "1. Dual Band"
```
Or set "Preferred Band" to prefer 5 GHz without hard-locking:
```powershell
Set-NetAdapterAdvancedProperty -Name "Wi-Fi 4" -DisplayName "Preferred Band" -DisplayValue "3. Prefer 5GHz band"
```

## Notes
- MacBook Pro (Fedora) also locked to 5 GHz the same day. See `macbook-wifi-5ghz-lock` skill.
- Signal is weaker on 5 GHz (~33% vs ~72% on 2.4 GHz) due to shorter range. Speed is still much better.
- Router is T-Mobile gateway, SSID DIEMILTONHAUS, broadcasts both bands on same SSID.
