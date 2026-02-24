# Home Assistant Container - Raspberry Pi 3 A+ (512MB RAM)

Use this approach when HA OS runs out of memory on the Pi 3 A+. Runs Home Assistant in Docker on top of Raspberry Pi OS — lighter than HA OS.

---

## Why This Approach

- Pi 3 A+ only has 512MB RAM — too little for HA OS
- HA OS also has a hardware conflict: the SD card and WiFi chip share the SDIO bus on Pi 3 A+, causing `brcmfmac` NAK errors and WiFi dropouts during heavy I/O (e.g. downloading HA Core) — makes HA OS unreliable
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
   - **Do NOT enable Raspberry Pi Connect** — not needed
   - Use **password authentication** (not public key)
6. Click **Save** → **Yes** to apply customization → flash it

> **CRITICAL:** Always use Raspberry Pi Imager's built-in "Edit Settings" to configure WiFi and SSH.
> Do NOT try to manually create `firstrun.sh` or `wpa_supplicant.conf` — it is unreliable on
> Raspberry Pi OS Trixie (2025+) due to path and NetworkManager changes (see Troubleshooting below).

> **Note:** After flashing, the SD card may not show up on Windows — this is normal. You do NOT need
> to access the SD card after flashing. All settings are already baked in. Just eject and insert into Pi.

---

## Step 2: Boot and SSH In

1. Insert SD card into Pi 3 A+, power on via micro USB
2. Wait **3-4 minutes** for first boot (runs setup scripts, may reboot once)
3. Find Pi IP — do a ping sweep first to populate ARP, then check:
   ```bash
   for i in $(seq 1 254); do ping -n 1 -w 100 192.168.12.$i > /dev/null 2>&1 & done; sleep 10; arp -a
   ```
   > **Note:** Pi 3 A+ MAC may be randomized on Trixie — won't always start with `b8:27:eb`

4. SSH in:
   ```
   ssh pi@192.168.12.XXX
   ```
   Or try: `ssh pi@raspberrypi.local`

### If WiFi didn't connect automatically

Even with Imager's Edit Settings, WiFi sometimes doesn't auto-connect on first boot. If the Pi isn't found on the network, connect a monitor and keyboard and log in directly:

```
login: pi
password: raspberry
```

Check WiFi status:
```bash
nmcli dev status
```

If `wlan0` shows `disconnected`, scan and connect manually:
```bash
sudo nmcli dev wifi list
sudo nmcli dev wifi connect MILTONHAUS password 'wisdom22!!'
```
> **Important:** Use single quotes around the password — the `!!` will trigger bash history expansion without them.

Then get the IP:
```bash
hostname -I
```

---

## Step 3: Install Docker on Raspberry Pi OS Trixie

> **WARNING:** The standard `get.docker.com` script does NOT work on Raspberry Pi OS Trixie.
> It tries to use the Raspbian repo which has no Trixie release. Follow this sequence instead:

### 3a: Fix apt sources (Trixie has broken Raspbian repo by default)

```bash
# Remove broken Raspbian Trixie source
sudo rm /etc/apt/sources.list.d/raspbian.sources

# Update apt (now only using archive.raspberrypi.com)
sudo apt-get update -qq
```

### 3b: Bootstrap the Debian archive keyring

```bash
# Temporarily trust Debian repo to install the keyring
sudo tee /etc/apt/sources.list.d/debian-temp.sources << 'EOF'
Types: deb
URIs: http://deb.debian.org/debian/
Suites: trixie
Components: main
Trusted: yes
EOF

sudo apt-get update -qq
sudo apt-get install -y --allow-unauthenticated debian-archive-keyring

# Remove the temporary trusted source
sudo rm /etc/apt/sources.list.d/debian-temp.sources
```

### 3c: Add proper Debian Trixie repo

```bash
sudo tee /etc/apt/sources.list.d/debian.sources << 'EOF'
Types: deb
URIs: http://deb.debian.org/debian/
Suites: trixie trixie-updates
Components: main contrib non-free non-free-firmware
Signed-By: /usr/share/keyrings/debian-archive-keyring.gpg
EOF

sudo apt-get update -qq
```

### 3d: Add Docker's Debian repo and install

```bash
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
echo "deb [arch=armhf signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/debian trixie stable" | sudo tee /etc/apt/sources.list.d/docker.list

sudo apt-get update -qq
sudo apt-get install -y docker-ce docker-ce-cli containerd.io

# Add pi user to docker group
sudo usermod -aG docker $USER
```

---

## Step 4: Run Home Assistant Container

```bash
sudo docker run -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=America/New_York \
  -v /home/pi/homeassistant:/config \
  --network=host \
  ghcr.io/home-assistant/raspberrypi3-homeassistant:stable
```

> Use `raspberrypi3-homeassistant` (32-bit) for Pi 3 A+ with Raspberry Pi OS Lite 32-bit.
> Image is large (~500MB+) — download takes several minutes on WiFi. This is normal.

---

## Step 5: Access Home Assistant

```
http://192.168.12.XXX:8123
```

First start takes a few minutes to download and initialize. Click **"Create my smart home"** to onboard.

---

## Step 6: Install ESPHome as a Separate Container

Since there's no add-on store, run ESPHome in its own container:

```bash
sudo docker run -d \
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

1. Do a ping sweep to populate the ARP table:
   ```bash
   for i in $(seq 1 254); do ping -n 1 -w 100 192.168.12.$i > /dev/null 2>&1 & done; sleep 10; arp -a
   ```
2. Pi 3 A+ is **2.4GHz only** — ensure your router broadcasts 2.4GHz on MILTONHAUS
3. Give it 3-4 minutes on first boot — it runs setup scripts and may reboot once
4. Pi MAC address may be randomized on Trixie — not always `b8:27:eb`
5. If still not found, connect monitor/keyboard and manually connect via nmcli (see Step 2)
6. If all else fails, pull SD card and re-flash using Imager with Edit Settings

### WiFi manual connect (nmcli)

```bash
nmcli dev status                          # check wlan0 state
sudo nmcli dev wifi list                  # scan for networks
sudo nmcli dev wifi connect MILTONHAUS password 'wisdom22!!'  # single quotes required for !!
hostname -I                               # get IP after connecting
```

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
