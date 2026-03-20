# Home Assistant on Lenovo ThinkCentre M900 Tiny

**Last Updated:** 2026-03-19
**Status:** Fedora installed, Claude Code installed, ready for Docker + Home Assistant

---

## Purpose

Run Home Assistant (and ESPHome add-on) for smart home automation, starting with the ESP32 closet lights project.

Replaces the Pi 3 A+ which has insufficient RAM (512MB) for Home Assistant.

---

## Hardware

**Lenovo ThinkCentre M900 Tiny**
- CPU: Intel i5-6500T
- RAM: 8GB
- Storage: 240GB Kingston SSDNOW 300 SATA SSD
- Power: ~10-15W idle (suitable for 24/7 use)
- OS: Fedora Workstation (fresh install, Windows wiped)

---

## Network

- IP: 192.168.12.128
- Username: milton
- Connected via ethernet
- SSH enabled: `sudo systemctl enable --now sshd`

---

## Setup Completed

1. ✅ Booted Fedora from USB (F12 at startup for boot menu, select UEFI USB)
2. ✅ Installed Fedora Workstation — selected "Use entire disk", no encryption
3. ✅ Enabled SSH: `sudo systemctl enable --now sshd`
4. ✅ Installed Claude Code: `curl -fsSL https://claude.ai/install.sh | sh`
5. ✅ Logged into Claude Code with API key

---

## Next Steps

6. Install Docker
7. Install Home Assistant via Docker
8. Set static IP
9. Install ESPHome add-on in HA
10. Flash ESP32 closet lights via ESPHome

---

## Installing Docker (next step)

```bash
sudo dnf install -y dnf-plugins-core
sudo dnf config-manager --add-repo https://download.docker.com/linux/fedora/docker-ce.repo
sudo dnf install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin
sudo systemctl enable --now docker
sudo usermod -aG docker milton
```

Then log out and back in, and verify with:
```bash
docker run hello-world
```

---

## Installing Home Assistant via Docker (after Docker)

```bash
docker run -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=America/Chicago \
  -v /home/milton/homeassistant:/config \
  --network=host \
  ghcr.io/home-assistant/home-assistant:stable
```

Access at: http://192.168.12.128:8123

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/skills/esp32-closet-lights/`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
