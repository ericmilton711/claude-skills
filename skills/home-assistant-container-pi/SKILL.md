# Home Assistant Container - Raspberry Pi 3 A+ (512MB RAM)

Use this approach when HA OS runs out of memory on the Pi 3 A+. Runs Home Assistant in Docker on top of Raspberry Pi OS — lighter than HA OS.

---

## Why This Approach

- Pi 3 A+ only has 512MB RAM — too little for HA OS
- HA Container runs lighter, no Supervisor overhead
- Trade-off: no add-on store — ESPHome must be installed separately as its own container

---

## Step 1: Flash Raspberry Pi OS Lite

1. Open Raspberry Pi Imager
2. **Choose Device** → Raspberry Pi 3
3. **Choose OS** → Raspberry Pi OS (other) → **Raspberry Pi OS Lite (32-bit)** — no desktop needed
4. **Choose Storage** → your SD card
5. Click **Next** → click **Edit Settings** (yes this time!)
   - Set hostname: `raspberrypi`
   - Set username: `pi` / password: `raspberry` (or custom)
   - **Configure WiFi**: enter MILTONHAUS / wisdom22!!
   - Set WiFi country: **US**
   - Enable SSH (under Services tab)
6. Click **Save** → **Yes** to apply customization → flash it

> **CRITICAL:** Always use Raspberry Pi Imager's built-in "Edit Settings" to configure WiFi and SSH.
> Do NOT try to manually create `firstrun.sh` or `wpa_supplicant.conf` — it is unreliable on
> Raspberry Pi OS Trixie (2025+) due to path and NetworkManager changes (see Troubleshooting below).

---

## Step 2: Boot and SSH In

1. Insert SD card into Pi 3 A+, power on via micro USB
2. Wait ~2 minutes for first boot
3. Find Pi IP via ARP (Pi MAC starts with `b8:27:eb`):
   ```
   arp -a
   ```
4. SSH in:
   ```
   ssh pi@192.168.12.XXX
   ```
   Or try: `ssh pi@raspberrypi.local`

---

## Step 3: Install Docker

```bash
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER
newgrp docker
```

---

## Step 4: Run Home Assistant Container

```bash
docker run -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=America/New_York \
  -v /home/pi/homeassistant:/config \
  --network=host \
  ghcr.io/home-assistant/raspberrypi3-homeassistant:stable
```

> Use `raspberrypi3-homeassistant` (32-bit) for Pi 3 A+ with Raspberry Pi OS Lite 32-bit.

---

## Step 5: Access Home Assistant

```
http://192.168.12.XXX:8123
```

First start takes a few minutes to download and initialize.

---

## Step 6: Install ESPHome as a Separate Container

Since there's no add-on store, run ESPHome in its own container:

```bash
docker run -d \
  --name esphome \
  --restart=unless-stopped \
  -v /home/pi/esphome:/config \
  --network=host \
  ghcr.io/esphome/esphome
```

Access ESPHome dashboard at:
```
http://192.168.12.XXX:6052
```

---

## Integrating ESPHome with HA Container

In HA, go to **Settings** → **Devices & Services** → **Add Integration** → search **ESPHome** and add your devices by IP.

---

## Feit Smart Plug

Same as HA OS — add via **Tuya** or **Local Tuya** integration in HA settings.

---

## Troubleshooting

### Pi not showing up on network after boot

1. Check `arp -a` — look for MAC starting with `b8:27:eb` in the `192.168.12.x` range
2. Pi 3 A+ is **2.4GHz only** — ensure your router broadcasts 2.4GHz on MILTONHAUS
3. Give it 3-4 minutes on first boot — it runs setup scripts and may reboot once
4. If still nothing, pull SD card and re-flash using Imager with Edit Settings

### Raspberry Pi OS Trixie (2025+) WiFi changes

Trixie changed several things from older Pi OS versions:

| Thing | Old (Bullseye/Bookworm) | New (Trixie) |
|-------|------------------------|--------------|
| Boot partition path | `/boot/` | `/boot/firmware/` |
| WiFi manager | wpa_supplicant | NetworkManager |
| Manual WiFi config | `wpa_supplicant.conf` in `/boot/` | NM connection file in `/etc/NetworkManager/system-connections/` |
| `firstrun.sh` cleanup path | `sed -i ... /boot/cmdline.txt` | `sed -i ... /boot/firmware/cmdline.txt` |

**Bottom line:** Don't do any of this manually. Use Raspberry Pi Imager's Edit Settings — it handles all of this correctly internally.

### WiFi country code

Without the WiFi country set, the Pi's radio may be restricted or blocked entirely.
Always set country to **US** in Raspberry Pi Imager's Edit Settings before flashing.

### Pi 3 A+ has no ethernet port

WiFi must be configured before first boot — there is no fallback wired connection.
This is why using Imager's built-in customization is essential.

---

## Notes

- No Supervisor, no add-on store — everything installed as separate Docker containers
- Updates done manually via `docker pull` and restart
- Much lower RAM usage than HA OS — should run fine on 512MB Pi 3 A+
- ESPHome devices auto-discover in HA via the ESPHome integration (add by IP)
- Plant moisture sensors, closet lights, etc. all show up as devices in HA dashboard
