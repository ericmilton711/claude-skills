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

> **CRITICAL — Endpoint must use direct IP:** Every device's WireGuard config must use `Endpoint = 174.54.51.209:51820` (direct IP), NOT `edenredux.servegame.com:51820` (hostname). The hostname fails to resolve on Pi-hole-managed devices and also fails on Fedora NetworkManager. This has caused tunnel failures on the Lenovo, MacBook Pro, and ThinkCentre. When setting up any new device, always use the direct IP.

---

## Device Status

| Device | IP | Tunnel IP | WireGuard Status | Method |
|--------|----|-----------|-----------------|--------|
| Eric's Lenovo (Windows 11) | 192.168.12.219 | 192.168.2.2 | ✅ DONE | Service (Lambert.conf in C:\Users\ericm\Downloads\) — **Endpoint must be direct IP** 174.54.51.209:51820, NOT hostname. WireGuard updates can reset the config back to hostname — re-apply fix if tunnel stops after an update. |
| Fedora MacBook Pro | 192.168.12.189 | 192.168.2.2 | ✅ DONE | NetworkManager — config at /etc/NetworkManager/system-connections/lambert.nmconnection; endpoint must be direct IP 174.54.51.209:51820 (fixed 2026-04-10) |
| ThinkCentre M900 Tiny | 192.168.1.107 / 192.168.12.136 | — | ❌ REMOVED | WireGuard removed — shared key caused it to steal tunnel from all client devices via 25s keepalive. ThinkCentre doesn't need Lambert access. |
| Mac Mini (Fedora/Pi-hole) | 192.168.12.163 | 192.168.2.2 | ❌ TODO | SSH install |
| Rosemary's MacBook | 192.168.12.237 | 192.168.2.2 | ❌ TODO | macOS WireGuard app + QR |
| Rosemary's iPhone | 192.168.12.215 | 192.168.2.2 | ❌ TODO | iOS WireGuard app + QR |
| Eric's iPad | 192.168.12.121 | 192.168.2.2 | ❌ TODO | iOS WireGuard app + QR |
| Gianna's Acer (Fedora) | 192.168.12.226 | 192.168.2.2 | ✅ DONE | wg-quick@lambert — config at /etc/wireguard/lambert.conf; enabled on boot; SSH: gianna@192.168.12.226 / wisdom22!! |
| Patrick's Chromebook | 192.168.12.221 | 192.168.2.2 | ❌ TODO | Android WireGuard app + QR |
| Eva's Chromebook | 192.168.12.194 / 100.115.92.195 (Tailscale) | 192.168.2.2 | ✅ DONE | Android WireGuard app — tunnel working; shared key conflict causes slowness when Gianna's laptop is also active (see note below) |
| kids1 (Lenovo V15 G2 IJL) | 192.168.12.249 | 192.168.2.2 | ✅ DONE | Service (C:\lambert.conf — uses direct IP 174.54.51.209, not hostname) |
| kids2 (Lenovo V15 G2 IJL) | 192.168.12.239 | 192.168.2.2 | ✅ DONE | Service (C:\lambert.conf — same as kids1) |
| iPad (.141) | 192.168.12.141 | 192.168.2.2 | ❌ TODO | iOS WireGuard app + QR |

---

## Shared Key Conflict — Why Only One Device Works at a Time

All devices use the **same private key** from Lambert.conf. The Lambert server treats all of them as a single peer and can only route traffic to one IP at a time — whichever device most recently sent a packet claiming to be that peer.

With `PersistentKeepalive=25`, any active device sends a keepalive every 25 seconds, reasserting tunnel ownership and knocking everyone else off.

**Symptoms:**
- Milton Home Page loads slowly or times out when two or more devices have WireGuard active simultaneously
- Gianna's `wg-quick@lambert` is enabled on boot with keepalive=25 — it will steal the tunnel from any other device within 25 seconds

**Current workaround:** Only have one device's WireGuard active at a time. Gianna's laptop is the biggest offender since it auto-connects on boot.

**Permanent fix (not yet done):** Generate a unique keypair for each device and register each as a separate peer on the Lambert server (`mac@192.168.0.1` / `645866`). This allows all devices to use the tunnel simultaneously.

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
