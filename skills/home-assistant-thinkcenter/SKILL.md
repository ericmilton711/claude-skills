# Home Assistant on Lenovo ThinkCentre M900 Tiny

**Last Updated:** 2026-03-20
**Status:** Home Assistant running via Docker

---

## Purpose

Run Home Assistant (and ESPHome add-on) for smart home automation, starting with the ESP32 closet lights project.

Replaces the Pi 3 A+ which has insufficient RAM (512MB) for Home Assistant.

---

## Hardware

**Lenovo ThinkCentre M900 Tiny**
- CPU: Intel i5-6500T
- RAM: 8GB
- Storage: 240GB Kingston SSDNOW 300 SATA SSD (model: SV300S37AZ240G)
- Power: ~10-15W idle (suitable for 24/7 use)
- OS: Fedora Workstation (fresh install, Windows wiped)
- Hostname: localhost-live (default, never changed)

---

## Network

- IP: 192.168.12.128
- Username: milton
- Password: 645866
- Connected via ethernet
- SSH enabled

---

## SSH from Windows

- Password SSH from Windows doesn't work (sshpass doesn't pass password correctly)
- Use SSH key auth instead: `ssh -i ~/.ssh/id_ed25519 milton@192.168.12.128`
- Eric's ed25519 public key is in `/home/milton/.ssh/authorized_keys`
- sudo password: `645866`
- Pass sudo password over SSH: `echo 645866 | sudo -S <command>`

---

## Setup Completed

1. ✅ Booted Fedora from USB (F12 at startup for boot menu, select UEFI USB)
2. ✅ Installed Fedora Workstation — selected "Use entire disk", no encryption
3. ✅ Enabled SSH: `sudo systemctl enable --now sshd`
4. ✅ Installed Claude Code: `curl -fsSL https://claude.ai/install.sh | sh`
5. ✅ Logged into Claude Code with API key
6. ✅ Installed Docker (moby-engine from Fedora repos — NOT docker-ce)
7. ✅ Enabled Docker: `sudo systemctl enable --now docker`
8. ✅ Added milton to docker group: `sudo usermod -aG docker milton`
9. ✅ Verified Docker works: `docker run hello-world`
10. ✅ Installed Home Assistant via Docker (running on port 8123)

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

---

## Home Assistant

Access at: http://192.168.12.128:8123

Docker run command:
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

---

## Next Steps

11. Set static IP
12. Complete Home Assistant onboarding at http://192.168.12.128:8123
13. Install ESPHome add-on in HA
14. Flash ESP32 closet lights via ESPHome

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/skills/esp32-closet-lights/`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
