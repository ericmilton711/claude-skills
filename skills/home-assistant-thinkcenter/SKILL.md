# Home Assistant on Lenovo ThinkCentre M700 Tiny

**Last Updated:** 2026-03-28
**Status:** SSD replaced (Samsung 870 EVO 1TB). Fresh Fedora install. Home Assistant, Pi-hole, and WireGuard running via Docker. Static IP: 192.168.12.136. Pi-hole group management active for Mac Mini.

---

## Purpose

Central home server running all network services:
- **Home Assistant** — smart home automation (ESP32 closet lights, etc.)
- **Pi-hole** — DNS-based parental controls / ad blocking
- **WireGuard** — VPN tunnel to Lambert network (192.168.0.x)

---

## Hardware

**Lenovo ThinkCentre M700 Tiny**
- CPU: Intel i5-6500T
- RAM: 8GB
- Storage: 1TB Samsung 870 EVO SATA III SSD (replaced Kingston 240GB — failed 2026-03-23)
- Power: ~10-15W idle (suitable for 24/7 use)
- OS: Fedora Workstation (fresh install 2026-03-28)
- Hostname: miltonhaus-server

---

## Network

- IP: 192.168.12.136 (static)
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

1. ✅ Samsung 870 EVO 1TB installed (replaced failed Kingston 240GB)
2. ✅ Fedora installed (UEFI), SSH enabled
3. ✅ Hostname set to miltonhaus-server
4. ✅ Docker installed (moby-engine — NOT docker-ce)
5. ✅ milton added to docker group
6. ✅ Backup restored from flash drive (miltonhaus_backup_20260323.tar.gz)
7. ✅ Home Assistant running via Docker (config restored)
8. ✅ Pi-hole running via Docker (config restored)
9. ✅ WireGuard running via Docker (config restored)
10. ✅ GDM auto-login configured (no password on boot)
11. ✅ DNS set to 1.1.1.1 / 8.8.8.8 (static, ignores DHCP)
12. ✅ Static IP set to 192.168.12.136
13. ✅ Pi-hole port 53 conflict fixed (systemd-resolved DNSStubListener=no)
14. ✅ Firewall opened for port 53 (TCP + UDP)
15. ✅ Pi-hole group management configured for Mac Mini (block all)
16. ✅ Auto-suspend/sleep disabled (server stays on permanently)
17. ⬜ Complete Home Assistant onboarding at http://192.168.12.136:8123

---

## Power Management (Keep Server Always On)

Sleep/suspend masked permanently — server will never auto-sleep or shut down:
```bash
sudo systemctl mask sleep.target suspend.target hibernate.target hybrid-sleep.target
```

logind config (`/etc/systemd/logind.conf`):
```ini
[Login]
HandleLidSwitch=ignore
HandleLidSwitchExternalPower=ignore
IdleAction=ignore
IdleActionSec=0
```

Apply with:
```bash
sudo systemctl restart systemd-logind
```

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

## Pi-hole Setup Notes

### Port 53 Conflict Fix (IMPORTANT)
On fresh Fedora installs, `systemd-resolved` occupies port 53 and Pi-hole can't bind to it.
Fix by disabling the stub listener:

```bash
sudo bash -c "cat > /etc/systemd/resolved.conf << EOF
[Resolve]
DNS=1.1.1.1 8.8.8.8
FallbackDNS=
DNSStubListener=no
EOF"
sudo systemctl restart systemd-resolved
docker restart pihole
```

Also open port 53 in the firewall:
```bash
sudo firewall-cmd --permanent --add-port=53/tcp
sudo firewall-cmd --permanent --add-port=53/udp
sudo firewall-cmd --reload
```

### Default Group — Whitelist (applies to all clients not in a specific group)

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

### Group Management (Per-Device Control)

**Mac Mini (192.168.12.163)** — in group `mac-mini`, NOT in Default group.
- Block-all rule (`.*`) assigned, no whitelist — everything is blocked except direct IPs.
- Milton Home Page (`192.168.0.100:5006`) still reachable since it's a direct IP (bypasses DNS).

To recreate mac-mini group from scratch:
```bash
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "
INSERT OR IGNORE INTO 'group' (id, enabled, name, description) VALUES (1, 1, 'mac-mini', 'Mac Mini - block all');
INSERT OR IGNORE INTO client (ip, comment) VALUES ('192.168.12.163', 'Mac Mini');
INSERT OR IGNORE INTO client_by_group (client_id, group_id) VALUES ((SELECT id FROM client WHERE ip='192.168.12.163'), 1);
INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES ((SELECT id FROM domainlist WHERE domain='.*' AND type=3), 1);
DELETE FROM client_by_group WHERE client_id=(SELECT id FROM client WHERE ip='192.168.12.163') AND group_id=0;
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

## SSD Replacement (2026-03-23 → 2026-03-28)

**Status: COMPLETE. New SSD installed and fully restored.**

- Kingston SV300S37A240G (240GB) failed — replaced with Samsung 870 EVO 1TB
- Backup was restored from flash drive (miltonhaus_backup_20260323.tar.gz)
- All containers restored and running

---

## Next Steps

- ✅ Set static IP to 192.168.12.136
- ⬜ Complete Home Assistant onboarding at http://192.168.12.136:8123
- ⬜ Install ESPHome add-on in HA
- ⬜ Flash ESP32 closet lights via ESPHome

---

## Related Projects

- ESP32 Closet Lights (`~/.claude/skills/esp32-closet-lights/`)
- MILTONHAUS Network (`~/.claude/skills/miltonhaus-network/SKILL.md`)
