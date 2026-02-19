# Home Assistant Container - Raspberry Pi 3 A+ (512MB RAM)

Use this approach when HA OS runs out of memory on the Pi 3 A+. Runs Home Assistant in Docker on top of Raspberry Pi OS — lighter than HA OS.

---

## Why This Approach

- Pi 3 A+ only has 512MB RAM — too little for HA OS
- HA Container runs lighter, no Supervisor overhead
- Trade-off: no add-on store — ESPHome must be installed separately as its own container

---

## Step 1: Flash Raspberry Pi OS (Lite)

1. Open Raspberry Pi Imager
2. **Choose Device** → Raspberry Pi 3
3. **Choose OS** → Raspberry Pi OS (other) → **Raspberry Pi OS Lite (64-bit)** — no desktop needed
4. **Choose Storage** → your SD card
5. Click **Next** → click **Edit Settings** (yes this time!)
   - Set hostname: `raspberrypi`
   - Set username/password
   - **Configure WiFi**: enter MILTONHAUS / wisdom22!!
   - Enable SSH
6. Flash it

---

## Step 2: Boot and SSH In

1. Insert SD card, power on Pi
2. Wait ~2 min for first boot
3. Find Pi IP via ARP (Pi MAC starts with `b8:27:eb`):
   ```bash
   arp -a | grep "192.168.12"
   ```
4. SSH in:
   ```bash
   ssh pi@192.168.12.XXX
   ```

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
  ghcr.io/home-assistant/raspberrypi3-64-homeassistant:stable
```

---

## Step 5: Access Home Assistant

```
http://192.168.12.XXX:8123
```

First start takes a few minutes.

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

## Notes

- No Supervisor, no add-on store — everything installed as separate Docker containers
- Updates done manually via `docker pull` and restart
- Much lower RAM usage than HA OS — should run fine on 512MB Pi 3 A+
