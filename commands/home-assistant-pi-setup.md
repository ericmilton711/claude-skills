# Home Assistant OS - Raspberry Pi Setup

Reference documentation for installing Home Assistant OS on a Raspberry Pi.

---

## Hardware Used

- **Pi Model:** Raspberry Pi 3 A+
- **Storage:** microSD card (16GB+ recommended, 8GB minimum)
- **Network:** Ethernet cable (required for initial setup)
- **Power:** Micro USB power supply

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

---

### Step 3: Boot the Pi

1. Eject microSD from PC
2. Insert into Raspberry Pi
3. Plug in ethernet cable
4. Power on via micro USB

---

### Step 4: Access Home Assistant

Open a browser on your PC and go to:

```
http://homeassistant.local:8123
```

- First boot takes **~5 minutes**
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

| Issue                          | Fix                                                    |
|--------------------------------|--------------------------------------------------------|
| homeassistant.local not loading| Wait 5 min, refresh, or restart PC                     |
| Page loads but slow            | Normal on first boot, give it time                     |
| Can't find microSD in Imager   | Try a different USB port or card reader                |
| Pi won't boot                  | Re-flash the microSD card                              |
| No ethernet activity lights    | Check cable, try different router port                 |
