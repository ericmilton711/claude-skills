# Home Assistant on Lenovo ThinkCentre M900 Tiny

**Last Updated:** 2026-03-20
**Status:** Home Assistant, Pi-hole, and WireGuard all running via Docker

---

## Purpose

Central home server running:
- **Home Assistant** — smart home automation (ESP32 closet lights, etc.)
- **Pi-hole** — DNS-based parental controls / ad blocking
- **WireGuard** — VPN tunnel to Lambert network (192.168.0.x)

Replaces the Pi 3 A+ which had insufficient RAM (512MB).

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
11. ✅ Installed Pi-hole via Docker (running on port 80/443/53)
12. ✅ Applied parental controls whitelist to Pi-hole
13. ✅ Installed WireGuard via Docker (Lambert tunnel active)

---

## Docker Install Notes

**Important:** Do NOT use the docker-ce repo on Fedora 43 — it conflicts with Fedora's packages.
Use Fedora's built-in moby-engine instead:

```bash
sudo dnf install -y moby-engine docker-compose
sudo systemctl enable --now docker
sudo usermod -aG docker milton
```

**SELinux note:** All volume mounts require `:z` flag or containers can't read/write files:
```bash
-v /path/on/host:/path/in/container:z
```

---

## Running Containers

### Home Assistant
- Access: http://192.168.12.128:8123
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

### Pi-hole
- Admin: http://192.168.12.128/admin (password: 645866)
- DNS: 192.168.12.128:53
```bash
docker run -d \
  --name pihole \
  --restart=unless-stopped \
  --network=host \
  -e TZ=America/Chicago \
  -e WEBPASSWORD=645866 \
  -v /home/milton/pihole/etc-pihole:/etc/pihole:z \
  -v /home/milton/pihole/etc-dnsmasq.d:/etc/dnsmasq.d:z \
  pihole/pihole:latest
```

### WireGuard
- Config: /home/milton/wireguard/wg0.conf
- VPN IP: 192.168.2.4/32 (Lambert tunnel)
```bash
docker run -d \
  --name wireguard \
  --restart=unless-stopped \
  --cap-add=NET_ADMIN \
  --cap-add=SYS_MODULE \
  -v /home/milton/wireguard:/config/wg_confs:z \
  -e TZ=America/Chicago \
  --network=host \
  lscr.io/linuxserver/wireguard:latest
```

---

## Pi-hole Whitelist (Parental Controls)

Block all domains except whitelist:
```bash
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('.*', 3, 1, 'Block all domains');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('homeschoolconnections.com', 2, 1, 'Homeschool platform');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('caravel.software', 2, 1, 'Homeschool assets');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('cloudfront.net', 2, 1, 'CDN');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('amazonaws.com', 2, 1, 'AWS');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('gstatic.com', 2, 1, 'Google static');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('googleapis.com', 2, 1, 'Google APIs');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('google.com', 2, 1, 'Google');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('vimeo.com', 2, 1, 'Educational videos');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('vimeocdn.com', 2, 1, 'Vimeo CDN');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('api.anthropic.com', 0, 1, 'Claude API');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('anthropic.com', 0, 1, 'Anthropic');
INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('claude.ai', 0, 1, 'Claude');
"
docker exec pihole pihole reloaddns
```

---

## WireGuard Config (/home/milton/wireguard/wg0.conf)

```ini
[Interface]
PrivateKey = 4Ll8kfPH48svJToniFdxqorU3Hcz/lpK1AcWE0JtTEE=
Address = 192.168.2.4/32
PostUp = iptables -t nat -A POSTROUTING -o wg0 -j MASQUERADE
PostDown = iptables -t nat -D POSTROUTING -o wg0 -j MASQUERADE

[Peer]
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
Endpoint = 174.54.51.209:51820
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
PersistentKeepalive = 25
```

---

## Next Steps

14. Complete Home Assistant onboarding at http://192.168.12.128:8123
15. Point router DNS to 192.168.12.128 (Pi-hole)
16. Set static IP on ThinkCentre
17. Install ESPHome add-on in HA
18. Flash ESP32 closet lights via ESPHome

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/skills/esp32-closet-lights/`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
