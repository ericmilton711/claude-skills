# Home Assistant on Lenovo ThinkCentre M900 Tiny

**Last Updated:** 2026-03-19
**Status:** Docker installed, ready to install Home Assistant

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
- Password: Milton645866
- Connected via ethernet
- SSH enabled

---

## Setup Completed

1. ✅ Booted Fedora from USB (F12 at startup for boot menu, select UEFI USB)
2. ✅ Installed Fedora Workstation — selected "Use entire disk", no encryption
3. ✅ Enabled SSH: `sudo systemctl enable --now sshd`
4. ✅ Installed Claude Code: `curl -fsSL https://claude.ai/install.sh | sh`
5. ✅ Logged into Claude Code with API key
6. ✅ Installed Docker (moby-engine from Fedora repos — NOT docker-ce)
7. ✅ Enabled Docker: `sudo systemctl enable --now docker`

---

## Docker Install Notes

**Important:** Do NOT use the docker-ce repo on Fedora 43 — it conflicts with Fedora's packages.
Use Fedora's built-in moby-engine instead:

```bash
# Remove docker-ce repo if added
sudo rm /etc/yum.repos.d/docker-ce.repo

# Install from Fedora repos
sudo dnf install -y moby-engine docker-compose

# Enable (service is called docker, not moby-engine)
sudo systemctl enable --now docker

# Add user to docker group
sudo usermod -aG docker milton
```

Log out and back in after adding user to docker group, then verify:
```bash
docker run hello-world
```

---

## Next Steps

8. Add milton to docker group and verify Docker works
9. Install Home Assistant via Docker
10. Set static IP
11. Install ESPHome add-on in HA
12. Flash ESP32 closet lights via ESPHome

---

## Installing Home Assistant via Docker

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

## SSH Notes

- SSH works from ThinkCentre terminal to itself
- Claude Code on Windows cannot SSH in interactively (no /dev/tty)
- SSH key copied via `ssh-copy-id` but interactive password SSH from Windows still blocked
- Workaround: run commands directly from ThinkCentre terminal or ThinkCentre's Claude Code

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/skills/esp32-closet-lights/`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
