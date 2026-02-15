# Lambert Network Access via Raspberry Pi VPN Gateway

**Date:** 2026-01-09
**Status:** Working

## Overview

The Raspberry Pi (pi.hole) at 192.168.1.7 acts as a VPN gateway to access the Lambert network (192.168.0.x) at edenredux.servegame.com. Local devices can route traffic through the Pi to reach the remote network.

## Network Topology

```
Local Network (192.168.1.x)              Lambert Network (192.168.0.x)
┌─────────────────────────┐              ┌─────────────────────────┐
│ USG 3P (192.168.1.1)    │              │ UDM Pro (192.168.0.1)   │
│ - Router/Gateway        │              │ - Router/Gateway        │
│ - DHCP Server           │              │                         │
└─────────────────────────┘              │ Server (192.168.0.100)  │
                                         │ - Port 5006 service     │
┌─────────────────────────┐              └─────────────────────────┘
│ Raspberry Pi            │                         ▲
│ (192.168.1.7)           │    WireGuard VPN        │
│ - Pi-hole DNS           │◄────────────────────────┘
│ - WireGuard Client      │    192.168.2.2 ◄──► 174.54.51.209:51820
│ - VPN Gateway           │
└─────────────────────────┘

┌─────────────────────────┐
│ Local Devices           │
│ - Route 192.168.0.0/24  │──► via 192.168.1.7
│   through Pi            │
└─────────────────────────┘
```

## Configuration Files

### WireGuard on Pi (/etc/wireguard/wg0.conf)

```ini
[Interface]
PrivateKey = aGPFmtpCh7zMBhwlRh+rpHHBIweHfIv5Grov80vwSWc=
Address = 192.168.2.2/32
# NAT for local network to access Lambert
PostUp = iptables -t nat -A POSTROUTING -o wg0 -j MASQUERADE
PostDown = iptables -t nat -D POSTROUTING -o wg0 -j MASQUERADE

[Peer]
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
Endpoint = 174.54.51.209:51820
PersistentKeepalive = 25
```

**Note:** This uses the "Lambert" identity (192.168.2.2). The Pi's original identity (192.168.2.5, public key `F7wbGnZ/Kjrs97HC0F2NZnRjm7BUv5jiSpWM9drB1ig=`) is not configured on the server.

### Pi's Original Config (Backup)

Saved at `/etc/wireguard/wg0.conf.backup`:
```ini
[Interface]
PrivateKey = GD87QVkfLbD+DflmlqaDI5pcVwUAajcsMWMewmCtCFk=
Address = 192.168.2.5/32

[Peer]
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
Endpoint = edenredux.servegame.com:51820
PersistentKeepalive = 25
```

To restore original config (requires adding Pi as peer on server first):
```bash
sudo cp /etc/wireguard/wg0.conf.backup /etc/wireguard/wg0.conf
sudo systemctl restart wg-quick@wg0
```

## How to Access Lambert Network from Local Devices

### Windows (Run as Administrator)

**Temporary route (lost on reboot):**
```cmd
route add 192.168.0.0 mask 255.255.255.0 192.168.1.7
```

**Permanent route:**
```cmd
route -p add 192.168.0.0 mask 255.255.255.0 192.168.1.7
```

**Test:**
```cmd
ping 192.168.0.1
```

### Mac/Linux

**Temporary:**
```bash
sudo ip route add 192.168.0.0/24 via 192.168.1.7
```

**Permanent (Linux - add to /etc/network/interfaces or NetworkManager):**
```bash
# NetworkManager
nmcli connection modify "Your Connection" +ipv4.routes "192.168.0.0/24 192.168.1.7"
```

### iPhone/Android

Cannot add custom routes without jailbreak/root. Use a VPN app or access via another device.

## Services on Lambert Network

| IP | Port | Service |
|----|------|---------|
| 192.168.0.1 | 443 | UDM Pro Web UI |
| 192.168.0.100 | 5006 | Web Service |
| 192.168.0.100 | 22 | SSH (password unknown) |

## Troubleshooting

### Check VPN is connected on Pi
```bash
sudo wg show
```
Should show "latest handshake" within the last 2 minutes and non-zero "received" bytes.

### Check NAT is working on Pi
```bash
sudo iptables -t nat -L POSTROUTING -n -v
```
Should show MASQUERADE rule for wg0.

### Restart VPN on Pi
```bash
sudo systemctl restart wg-quick@wg0
```

### Firewalld Fix (2026-01-10)
Firewalld was blocking forwarded traffic from local devices to the VPN. Fix:
```bash
# Disable firewalld (interferes with NAT)
sudo systemctl stop firewalld
sudo systemctl disable firewalld

# NAT rule is handled by WireGuard PostUp in /etc/wireguard/wg0.conf
```
If forwarding stops working after updates, check if firewalld got re-enabled.

### Test from Pi
```bash
ping 192.168.0.1
ping 192.168.0.100
```

## Known Issues

1. **Pi using Lambert identity** - The Pi is using the Lambert config (192.168.2.2) instead of its own identity (192.168.2.5) because the server doesn't have the Pi's public key configured.

2. **Cannot SSH to Lambert devices** - SSH credentials for devices on 192.168.0.x are unknown. Tried passwords `645866` and `PASSword!?1711` without success.

3. **DHCP static routes don't work** - Adding DHCP option 121 for static routes breaks internet access because it overrides the default gateway. Devices must add routes manually.

## To-Do (Future)

1. **Add Pi's public key to Lambert VPN server** - Once SSH access to 192.168.0.x is recovered, add:
   ```ini
   [Peer]
   # Raspberry Pi
   PublicKey = F7wbGnZ/Kjrs97HC0F2NZnRjm7BUv5jiSpWM9drB1ig=
   AllowedIPs = 192.168.2.5/32
   ```

2. **Recover SSH credentials** - Need physical access or password recovery for devices at Lambert.

3. **Make USG config persistent** - If any USG changes are needed, use config.gateway.json on the UniFi controller.

## Parental Controls

Pi-hole is configured to block ALL domains except a whitelist. This means only `http://192.168.0.100:5006` (direct IP) and whitelisted domains (Claude API) are accessible.

See: `/home/mac/.claude/skills/project-management/pihole-parental-controls.md`

**Quick commands:**
```bash
# Disable parental controls
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = '.*' AND type = 3;"
sudo pihole reloaddns

# Re-enable parental controls
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('.*', 3, 1, 'Block all domains');"
sudo pihole reloaddns
```

## Related Documentation

- `/home/mac/.claude/skills/project-management/device-inventory-wireguard-pihole.md`
- `/home/mac/.claude/skills/project-management/wireguard-windows-setup.md`
- `/home/mac/.claude/skills/project-management/raspberry-pi-wireguard-pihole-setup.md`
- `/home/mac/.claude/skills/project-management/pihole-parental-controls.md`
