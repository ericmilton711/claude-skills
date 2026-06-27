# Fire HD 10 Tablet — Kiosk Weather Dashboard

**Device:** Amazon Fire HD 10 (2023)
**Serial:** GR734P04542207TH
**IP:** 192.168.12.172 (static, configured on device)
**MAC:** b6-7f-2b-ae-24-3a (randomized)
**Role:** Weather dashboard display in Fully Kiosk Browser, showing http://192.168.12.240/
**Pi-hole group:** 10 (fire-tablet) — default-deny, weather dependencies whitelisted

---

## ADB Access (Wireless)

ADB is set up over WiFi. No USB data cable needed.

### From Eric's Windows Laptop

```powershell
$adb = "$env:LOCALAPPDATA\platform-tools\adb.exe"

# 1. On the tablet: Settings > Device Options > Developer Options > Wireless debugging > Pair device with pairing code
# 2. Get the pairing IP:port and 6-digit code from the tablet screen
& $adb pair <IP>:<PAIRING_PORT> <CODE>

# 3. Connect using the main wireless debugging IP:port (shown on the Wireless debugging screen, NOT the pairing port)
& $adb connect 192.168.12.172:<PORT>

# 4. Verify
& $adb devices
```

**Note:** The pairing port and connection port are different. Pairing port is shown on the "Pair device" dialog. Connection port is shown on the main Wireless debugging screen.

**ADB installed at:** `%LOCALAPPDATA%\platform-tools\adb.exe`

---

## Screen Always-On (Deployed 2026-06-27)

Three layers to prevent sleep:

1. **Developer Options > Stay Awake** — toggled on in tablet settings
2. **ADB: stay on while plugged in**
   ```powershell
   & $adb -s 192.168.12.172:<PORT> shell settings put global stay_on_while_plugged_in 3
   ```
   (3 = stay on for USB + AC + wireless charging)
3. **ADB: max screen timeout**
   ```powershell
   & $adb -s 192.168.12.172:<PORT> shell settings put system screen_off_timeout 2147483647
   ```

---

## Lock Screen Ads Removal (Attempted 2026-06-27)

### What worked
```powershell
& $adb shell settings put global LOCKSCREEN_AD_ENABLED 0
& $adb shell settings put global advertising_id 0
& $adb shell pm disable-user -k com.amazon.hybridadidservice
```

### What didn't work
Amazon protects `com.amazon.kindle.kso` (Special Offers) on newer Fire OS. These all fail:
```
pm disable-user -k com.amazon.kindle.kso  → SecurityException: Cannot disable a protected package
pm uninstall -k --user 0 com.amazon.kindle.kso  → DELETE_FAILED_INTERNAL_ERROR
pm uninstall -k --user 0 com.amazon.adep  → DELETE_FAILED_INTERNAL_ERROR
pm uninstall -k --user 0 com.amazon.advertisingidsettings  → DELETE_FAILED_INTERNAL_ERROR
```

### If ads still appear
Pay Amazon $2 to remove Special Offers: go to amazon.com > Manage Your Content and Devices > Devices > select the Fire tablet > Remove lock screen ads.

---

## Developer Options

Enabled on the tablet:
- **USB Debugging:** On
- **Wireless Debugging:** On (authorized on MILTONHAUS network)
- **Stay Awake:** On

To access: Settings > Device Options > Developer Options
(If hidden: Settings > Device Options > About Fire Tablet > tap Serial Number 7 times)

---

## Fully Kiosk Browser

Runs in kiosk mode showing the ESP32 weather dashboard at http://192.168.12.240/. See `miltonhaus-pihole-rules` skill for Pi-hole group 10 allowed domains.
