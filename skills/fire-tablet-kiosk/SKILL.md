# Fire HD 10 Tablet — Kiosk Weather Dashboard

**Device:** Amazon Fire HD 10 (2023)
**Serial:** GR734P04542207TH
**IP:** 192.168.12.172 (static, configured on device)
**MAC:** b6-7f-2b-ae-24-3a (randomized)
**Role:** Weather dashboard display in Firefox (sideloaded), showing http://192.168.12.240/
**Pi-hole group:** 10 (fire-tablet) — default-deny, weather dependencies whitelisted. Currently in group 0 (unrestricted) while setting up Google Voice.
**Google Voice:** Web app at voice.google.com, accessed via "Voice" button in dashboard footer. Account: ericmilton711@gmail.com, number: (856) 354-5644. ID verification pending as of 2026-07-12.
**Android SDK:** 30 (Android 11), ARM64
**Lock screen PIN:** 645866

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

## Firefox (Sideloaded 2026-06-27)

Replaced Fully Kiosk Browser. Firefox APK sideloaded via ADB from Mozilla's archive:

```powershell
# Download (check https://archive.mozilla.org/pub/fenix/releases/ for latest)
Invoke-WebRequest -Uri "https://archive.mozilla.org/pub/fenix/releases/152.0.2/android/fenix-152.0.2-android-arm64-v8a/fenix-152.0.2.multi.android-arm64-v8a.apk" -OutFile "$env:TEMP\firefox.apk"

# Install
& $adb -s 192.168.12.172:<PORT> install "$env:TEMP\firefox.apk"

# Launch with weather dashboard
& $adb -s 192.168.12.172:<PORT> shell am start -a android.intent.action.VIEW -d "http://192.168.12.240/" -n org.mozilla.firefox/.App
```

**Package:** `org.mozilla.firefox`

---

## Screen Pinning (Deployed 2026-06-27)

Firefox is pinned to the screen using Android's lock task mode. Requires PIN (645866) to exit via home or recents button.

```powershell
# Enable screen pinning
& $adb shell settings put secure lock_to_app_enabled 1

# Set PIN
& $adb shell locksettings set-pin 645866

# Find Firefox task ID
& $adb shell "dumpsys activity activities | grep org.mozilla.firefox | grep Task"

# Pin it (replace TASK_ID with the number from above, e.g. 179)
& $adb shell "am task lock TASK_ID"

# Unpin
& $adb shell "am task lock stop"
```

### Immersive mode (hides nav bar)
```powershell
& $adb shell settings put global policy_control "immersive.full=*"
```

### Known limitation
The back button (triangle) can still exit Firefox if there's no browsing history. Home and recents buttons are properly blocked and require PIN. Fully disabling the back button requires root access, which is not available on this device.

---

## Screen Always-On (Deployed 2026-06-27)

Three layers to prevent sleep:

1. **Developer Options > Stay Awake** — toggled on in tablet settings
2. **ADB: stay on while plugged in**
   ```powershell
   & $adb shell settings put global stay_on_while_plugged_in 3
   ```
   (3 = stay on for USB + AC + wireless charging)
3. **ADB: max screen timeout**
   ```powershell
   & $adb shell settings put system screen_off_timeout 2147483647
   ```

---

## Lock Screen Ads Removal (Deployed 2026-06-27)

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

### If ads return
Pay Amazon $2 to remove Special Offers: amazon.com > Manage Your Content and Devices > Devices > select the Fire tablet > Remove lock screen ads.

---

## Developer Options

Enabled on the tablet:
- **USB Debugging:** On
- **Wireless Debugging:** On (authorized on MILTONHAUS network)
- **Stay Awake:** On

To access: Settings > Device Options > Developer Options
(If hidden: Settings > Device Options > About Fire Tablet > tap Serial Number 7 times)

---

## Google Voice (Added 2026-07-12)

Google Voice runs as a web app in Firefox on the tablet (no Google Play Services needed).

- **Account:** ericmilton711@gmail.com
- **Number:** (856) 354-5644
- **Access:** "Voice" button in weather dashboard footer opens voice.google.com in a new tab
- **Status:** ID verification pending as of 2026-07-12. Once approved, sign in at voice.google.com on the tablet.
- **Note:** Google Play Services cannot be sideloaded on this Fire tablet (APK download sites block automated access, Silk browser too old for Cloudflare). Web app is the working solution.

### Opening Firefox with a specific URL via ADB

```powershell
& $adb -s 192.168.12.172:<PORT> shell "am start -a android.intent.action.VIEW -d 'https://voice.google.com' --activity-clear-task org.mozilla.firefox/org.mozilla.fenix.IntentReceiverActivity"
```

The `--activity-clear-task` flag and `IntentReceiverActivity` component are required, otherwise Firefox restores its previous session (the dashboard) instead of opening the URL.

---

## Pi-hole (Group 10)

Default-deny. See `miltonhaus-pihole-rules` skill for allowed domains. Firefox's captive portal check (`detectportal.firefox.com`) is blocked but the weather dashboard loads fine since it's a direct IP (http://192.168.12.240/).

**Note:** As of 2026-07-12, the Fire tablet is temporarily in group 0 (unrestricted) for Google Voice setup. Move back to group 10 once voice.google.com domains are whitelisted.
