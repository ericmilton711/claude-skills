# Home Assistant OS - Raspberry Pi Setup

Reference documentation for installing Home Assistant OS on a Raspberry Pi.

---

## Hardware Used

- **Pi Model:** Raspberry Pi 3 A+
- **Storage:** microSD card (16GB+ recommended, 8GB minimum)
- **Network:** WiFi (Pi 3 A+ has NO ethernet port — see WiFi config step below)
- **Power:** Micro USB power supply

> **Note:** Pi 3 A+ has no ethernet port. WiFi must be pre-configured on the SD card before first boot (see Step 3b).

---

## Installation Steps

### Step 1: Download & Install Raspberry Pi Imager

Download from: raspberrypi.com/software

Or let Claude download it directly:
```bash
curl -L -o "$USERPROFILE/Downloads/raspberry-pi-imager.exe" "https://downloads.raspberrypi.org/imager/imager_latest.exe"
```

Then run the installer.

---

### Step 2: Flash Home Assistant OS

1. Open Raspberry Pi Imager
2. **Choose Device** → Raspberry Pi 3
3. **Choose OS** → Other specific-purpose OS → Home assistants and home automation → Home Assistant → **Home Assistant OS (RPi 3)**
4. **Choose Storage** → select your microSD card
5. Click **Next** → when asked about OS customization, click **No**
6. Flash it

> **If SD card not detected in Imager:** Claude can format it first using diskpart. SD card will show as drive letter after formatting.

---

### Step 3: Boot the Pi

1. Eject microSD from PC
2. Insert into Raspberry Pi
3. Power on via micro USB

---

### Step 3b: Pre-configure WiFi (Pi 3 A+ only — no ethernet port)

After flashing, **before booting**, put the SD card back in the PC and create a WiFi config file:

The SD card should mount as a drive letter (e.g. D:) with a small FAT32 boot partition (~32MB).

Create this file: `D:\CONFIG\network\my-network`

```ini
[connection]
id=my-network
uuid=72111c67-4a5d-4d5c-925e-f8ee26efb3eb
type=802-11-wireless

[802-11-wireless]
mode=infrastructure
ssid=YOUR_WIFI_NAME

[802-11-wireless-security]
auth-alg=open
key-mgmt=wpa-psk
psk=YOUR_WIFI_PASSWORD

[ipv4]
method=auto

[ipv6]
addr-gen-mode=stable-privacy
method=auto
```

Eject SD card, insert into Pi, power on. HA will connect to WiFi automatically.

---

### Step 4: Access Home Assistant

Open a browser on your PC and go to:

```
http://homeassistant.local:8123
```

- First boot takes **~5 minutes** just to show the landing page
- Full HA Core download and setup takes **20+ minutes** after that
- If DNS error appears during setup, try selecting Google (8.8.8.8) — if that fails, just wait and retry
- If page doesn't load, wait and refresh
- If still not loading, try restarting your PC (helps with mDNS/network discovery)

---

### Step 5: Onboarding

- Create your Home Assistant account
- Set your location and preferences
- Skip adding integrations for now (add later)

---

## Installing ESPHome Add-on

Once HA is running:

1. Go to **Settings** → **Add-ons** → **Add-on Store**
2. Search for **ESPHome**
3. Click **Install**
4. Enable **Start on boot** and **Show in sidebar**
5. Click **Start**

ESPHome dashboard will appear in the HA sidebar.

---

## Troubleshooting

| Issue                          | Fix                                                              |
|--------------------------------|------------------------------------------------------------------|
| homeassistant.local not loading| Wait 5 min, refresh, or restart PC                               |
| Page loads but slow            | Normal on first boot, give it time                               |
| Can't find microSD in Imager   | Have Claude format it with diskpart first                        |
| Pi won't boot                  | Re-flash the microSD card                                        |
| No ethernet activity lights    | Pi 3 A+ has no ethernet — use WiFi config method (Step 3b)       |
| DNS error during setup         | Try Google DNS; if fails, wait and retry — WiFi may still work   |
| Preparing HA screen with logs  | Normal — HA Core is downloading, wait 20+ min                    |
| wlan0 does not exist warning   | Benign during boot if WiFi not yet initialized                   |
