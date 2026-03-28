# Mac Mini — Fedora Setup

**Last Updated:** 2026-03-28
**Status:** Fedora installed. DNS locked to Pi-hole. WiFi disabled. Block-all parental controls active.

---

## Hardware

**Apple Mac Mini (running Fedora Linux)**
- OS: Fedora
- Hostname: fedora
- Username: mac
- Password: 645866

---

## Network

- IP: 192.168.12.163 (DHCP via ethernet)
- Connected via ethernet only — WiFi disabled
- WireGuard active: 192.168.2.2/32 (Lambert tunnel)
- DNS: 192.168.12.136 (Pi-hole — block all)

---

## SSH Access

```bash
ssh mac@192.168.12.163
# password: 645866
# sudo: echo 645866 | sudo -S <command>
```

---

## Setup Completed

1. ✅ Fedora installed
2. ✅ SSH enabled
3. ✅ WiFi disabled (ethernet only)
4. ✅ IPv6 disabled on ethernet connection
5. ✅ DNS set to Pi-hole (192.168.12.136) — static, ignores DHCP
6. ✅ Assigned to Pi-hole `mac-mini` group (block all, no whitelist)
7. ✅ Removed from Pi-hole Default group
8. ✅ WireGuard active (Lambert tunnel 192.168.2.2)

---

## Pi-hole Group

This machine is in the Pi-hole `mac-mini` group (NOT the Default group).
- **Everything blocked** — all domain lookups return 0.0.0.0
- **Direct IPs still work** — Pi-hole only blocks DNS, not IP-based connections
- **Milton Home Page reachable:** http://192.168.0.100:5006 (direct IP via WireGuard)

Tested blocked: google.com, youtube.com, youtu.be, googlevideo.com, ytimg.com — all blocked.

---

## WiFi Disabled

WiFi is disabled and set to not autoconnect on reboot:
```bash
sudo nmcli device disconnect wlp3s0
sudo nmcli device set wlp3s0 autoconnect no
```

To verify:
```bash
nmcli device status
```

---

## DNS Configuration

DNS is hardcoded to Pi-hole via NetworkManager and systemd-resolved:
```bash
sudo nmcli con mod "Profile 1" ipv4.dns "192.168.12.136" ipv4.ignore-auto-dns yes
sudo nmcli con mod "Profile 1" ipv6.method "disabled"
```

systemd-resolved config (`/etc/systemd/resolved.conf`):
```ini
[Resolve]
DNS=192.168.12.136
FallbackDNS=
```

---

## Accessible Sites

| Site | Access Method | Status |
|------|--------------|--------|
| Milton Home Page (http://192.168.0.100:5006) | Direct IP via WireGuard | ✅ Allowed |
| All domains (google.com, youtube.com, etc.) | DNS lookup | ❌ Blocked |

---

## Related

- ThinkCentre Server / Pi-hole: `skills/home-assistant-thinkcenter/SKILL.md`
- MILTONHAUS Network: `skills/miltonhaus-network/SKILL.md`
