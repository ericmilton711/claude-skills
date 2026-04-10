---
name: miltonhaus-wireguard
description: Complete WireGuard Lambert tunnel setup for all MILTONHAUS devices — connects to Lambert Family network to access Milton Homepage and Nextcloud
type: reference
---

# MILTONHAUS WireGuard — Lambert Tunnel

Connects every device to the Lambert Family network via WireGuard, giving access to:
- **Milton Home Page:** http://192.168.0.100:5006 (also observed as 192.168.0.165:5006 — use direct IP)
- **Nextcloud:** http://192.168.0.165:11000
- **Home Assistant:** http://192.168.0.103

> **Why the home page only works on some devices:** It requires WireGuard to be active. Devices without WireGuard set up cannot reach 192.168.0.x at all and will time out.

Access these by direct IP — do NOT use hostnames (Lambert's nginx uses name-based routing).

---

## Server Details

| Field | Value |
|-------|-------|
| Endpoint | edenredux.servegame.com:51820 (174.54.51.209) |
| Server Public Key | uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc= |
| Tunnel subnet | 192.168.2.x |
| Routes | 192.168.0.0/24, 192.168.2.0/24 |

---

## Lambert.conf (The Master Config)

Stored at: `C:\Users\ericm\Downloads\Lambert.conf`

The WireGuard service on the Windows PC was installed by running:
```
wireguard.exe /tunnelservice C:\Users\ericm\Downloads\Lambert.conf
```

The config was pre-made with a keypair already registered on the Lambert server.
**This same config works on ALL devices** — no need to register new keys.

```ini
[Interface]
Address = 192.168.2.2/32
DNS = 192.168.1.104
PrivateKey = aGPFmtpCh7zMBhwlRh+rpHHBIweHfIv5Grov80vwSWc=

[Peer]
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
Endpoint = edenredux.servegame.com:51820
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
```

> Note: AllowedIPs only covers the Lambert subnets — normal internet traffic goes direct, not through the tunnel. Both Milton Homepage AND regular browsing work simultaneously.

---

## Device Status

| Device | IP | Tunnel IP | WireGuard Status | Method |
|--------|----|-----------|-----------------|--------|
| Eric's Lenovo (Windows 11) | 192.168.12.219 | 192.168.2.2 | ✅ DONE | Service (Lambert.conf in C:\Users\ericm\Downloads\) |
| Fedora MacBook Pro | 192.168.12.189 | 192.168.2.2 | ✅ DONE | NetworkManager (GUI toggle) |
| ThinkCentre M900 Tiny | 192.168.1.107 / 192.168.12.136 | 192.168.2.2 | ✅ DONE | Docker container (wg0) — config at /home/milton/wireguard/wg0.conf; uses Lambert.conf keypair |
| Mac Mini (Fedora/Pi-hole) | 192.168.12.163 | 192.168.2.2 | ❌ TODO | SSH install |
| Rosemary's MacBook | 192.168.12.237 | 192.168.2.2 | ❌ TODO | macOS WireGuard app + QR |
| Rosemary's iPhone | 192.168.12.215 | 192.168.2.2 | ❌ TODO | iOS WireGuard app + QR |
| Eric's iPad | 192.168.12.121 | 192.168.2.2 | ❌ TODO | iOS WireGuard app + QR |
| Gianna's Asus (Fedora) | 192.168.12.226 | 192.168.2.2 | ❌ TODO | SSH install |
| Patrick's Chromebook | 192.168.12.221 | 192.168.2.2 | ❌ TODO | Android WireGuard app + QR |
| Ev's Chromebook | 100.115.92.195 | 192.168.2.2 | ❌ TODO | Android WireGuard app + QR |
| kids1 (Lenovo V15 G2 IJL) | 192.168.12.249 | 192.168.2.2 | ✅ DONE | Service (C:\lambert.conf — uses direct IP 174.54.51.209, not hostname) |
| kids2 (Lenovo V15 G2 IJL) | 192.168.12.239 | 192.168.2.2 | ✅ DONE | Service (C:\lambert.conf — same as kids1) |
| iPad (.141) | 192.168.12.141 | 192.168.2.2 | ❌ TODO | iOS WireGuard app + QR |

---

## Setup Methods

### Method 1 — QR Code (phones, tablets, Chromebooks, Macs)

QR code saved at: `C:\Users\ericm\OneDrive\Desktop\screenshots\lambert-qr.png`

Regenerate with:
```python
# Run on Eric's Windows PC
import qrcode
conf = open('C:/Users/ericm/Downloads/Lambert.conf').read()
qr = qrcode.make(conf)
qr.save('lambert-qr.png')
```

On each device:
1. Install **WireGuard** app (App Store / Play Store)
2. Tap **Add Tunnel → Create from QR code**
3. Scan `lambert-qr.png`
4. Toggle on

---

### Method 2 — Fedora via NetworkManager (GUI toggle, no terminal)

SSH in and run:
```bash
# Copy Lambert.conf to machine first (scp or paste)
# Then create NetworkManager connection:
sudo cp lambert.nmconnection /etc/NetworkManager/system-connections/lambert.nmconnection
sudo chmod 600 /etc/NetworkManager/system-connections/lambert.nmconnection
sudo nmcli con reload
```

The `lambert.nmconnection` file:
```ini
[connection]
id=lambert
type=wireguard
interface-name=lambert
autoconnect=false

[wireguard]
private-key=aGPFmtpCh7zMBhwlRh+rpHHBIweHfIv5Grov80vwSWc=

[wireguard-peer.uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=]
endpoint=edenredux.servegame.com:51820
allowed-ips=192.168.0.0/24;192.168.2.0/24;
persistent-keepalive=25

[ipv4]
address1=192.168.2.2/32
method=manual

[ipv6]
method=disabled
```

Toggle from GUI: **network icon (top-right) → VPN → lambert**

Terminal commands if needed:
```bash
sudo nmcli con up lambert      # start
sudo nmcli con down lambert    # stop
```

---

### Method 3 — Fedora via wg-quick (terminal only)

```bash
sudo dnf install -y wireguard-tools
sudo cp lambert.conf /etc/wireguard/lambert.conf
sudo chmod 600 /etc/wireguard/lambert.conf
sudo systemctl enable --now wg-quick@lambert   # start + enable on boot
sudo systemctl stop wg-quick@lambert           # stop
sudo systemctl start wg-quick@lambert          # start
```

---

## Pi-hole DNS Note

The Lambert.conf uses `DNS = 192.168.1.104` (Lambert's Raspberry Pi Pi-hole).
On Fedora machines, wg-quick overrides system DNS when the tunnel is active.

To bypass Pi-hole temporarily on Fedora (e.g. for package installs):
```bash
sudo nmcli con down lambert           # stop tunnel
sudo resolvectl dns wlp3s0 1.1.1.1   # set DNS to Cloudflare
```

When using our own Pi-hole instead, change DNS line to `192.168.12.163`.

---

## Finding the Lambert.conf on Windows

If the config is lost, find it with:
```powershell
Get-WmiObject Win32_Service | Where-Object { $_.Name -like "*WireGuard*" } | Select PathName
```
This reveals the full path passed to wireguard.exe.
