# Home Assistant on Lenovo ThinkCentre M900 Tiny

**Last Updated:** 2026-03-23
**Status:** Home Assistant, Pi-hole, and WireGuard running via Docker. UniFi removed.

---

## Purpose

Central home server running all network services:
- **Home Assistant** — smart home automation (ESP32 closet lights, etc.)
- **Pi-hole** — DNS-based parental controls / ad blocking
- **WireGuard** — VPN tunnel to Lambert network (192.168.0.x)

---

## Hardware

**Lenovo ThinkCentre M900 Tiny**
- CPU: Intel i5-6500T
- RAM: 8GB
- Storage: 240GB Kingston SSDNOW 300 SATA SSD (model: SV300S37AZ240G)
- Power: ~10-15W idle (suitable for 24/7 use)
- OS: Fedora Workstation
- Hostname: unifi (default hostname, no longer running UniFi)

---

## Network

- IP: 192.168.12.136 (MILTONHAUS WiFi network)
- Username: milton
- Password: 645866
- Connected via ethernet to switch
- SSH enabled — reachable from MILTONHAUS WiFi (192.168.12.x)

---

## SSH Access

```bash
ssh milton@192.168.12.136
# password: 645866
# sudo: echo 645866 | sudo -S <command>
```

---

## Auto-Login (Console)

GDM is configured to auto-login as `milton` on boot — no password needed on the monitor.

Config file: `/etc/gdm/custom.conf`
```ini
[daemon]
AutomaticLoginEnable=True
AutomaticLogin=milton
```

TTY1 also configured for auto-login at `/etc/systemd/system/getty@tty1.service.d/override.conf`:
```ini
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin milton --noclear %I $TERM
```

---

## Setup Completed

1. ✅ Fedora installed, SSH enabled
2. ✅ Docker installed (moby-engine — NOT docker-ce)
3. ✅ milton added to docker group
4. ✅ Home Assistant installed via Docker
5. ✅ Pi-hole installed via Docker with parental controls whitelist
6. ✅ WireGuard installed via Docker (Lambert tunnel active)
7. ✅ UniFi removed (container stopped, data deleted)
8. ✅ GDM auto-login configured

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

**DNS note:** ThinkCentre's own DNS must be set to 1.1.1.1 (not Pi-hole) or Docker can't pull images:
```bash
echo 645866 | sudo -S resolvectl dns eno1 1.1.1.1 8.8.8.8
```

---

## Running Containers

### Home Assistant
- Access: http://192.168.12.136:8123
- Also accessible via WireGuard: http://192.168.0.103:8123
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
- Admin: http://192.168.12.136/admin (password: 645866)
- DNS: 192.168.12.136:53
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

## SSD Failure & Replacement (2026-03-23)

**Status: Server powered OFF. Waiting for new SSD.**

- Kingston SV300S37A240G (240GB) is critically failing — SMART shows `SSD_Life_Left FAILING_NOW value: 001 threshold: 010`
- Overall health: FAILED. Drive failure expected within 24 hours.
- Server powered off to prevent data loss
- **Backup created:** `/run/media/milton/2829-C190/miltonhaus_backup_20260323.tar.gz` (584MB) — full `/home/milton/` on flash drive
- **Replacement ordered:** Samsung 870 EVO SATA III SSD 1TB 2.5" — confirmed compatible (same 2.5" SATA III 7mm form factor)

### After SSD Arrives — Reinstall Steps

1. Install Samsung 870 EVO into ThinkCentre M900 Tiny
2. Fresh Fedora Workstation install
3. Restore backup from flash drive: `tar -xzf miltonhaus_backup_20260323.tar.gz -C /home/milton/`
4. Install Docker (moby-engine — NOT docker-ce): `sudo dnf install -y moby-engine docker-compose`
5. Re-run Home Assistant, Pi-hole, WireGuard containers (see "Running Containers" section)
6. Configure GDM auto-login (see "Auto-Login" section)
7. Fix DNS: `nmcli con mod 'Wired connection 1' ipv4.dns '1.1.1.1 8.8.8.8' ipv4.ignore-auto-dns yes`
8. Complete Home Assistant onboarding at http://192.168.12.136:8123
9. Set static IP (currently DHCP — may change after reinstall)

---

## Next Steps

- Receive Samsung 870 EVO — follow reinstall steps above
- Complete Home Assistant onboarding (create account at http://192.168.12.136:8123)
- Install ESPHome add-on in HA
- Flash ESP32 closet lights via ESPHome
- Set static IP on ThinkCentre

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/skills/esp32-closet-lights/`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
