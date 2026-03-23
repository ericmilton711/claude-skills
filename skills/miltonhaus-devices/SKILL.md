---
name: miltonhaus-devices
description: All devices on the MILTONHAUS network — IPs, MACs, hostnames, and what they are
type: reference
---

# MILTONHAUS — All Network Devices

**Last updated:** 2026-03-23
**Network:** DIEMILTONHAUS (T-Mobile, 192.168.12.x)
**Gateway:** TMO-G5AR at 192.168.12.1

> IPs are DHCP-assigned and can drift. MACs are the stable identifiers.

---

## Full Device List

| IP | MAC | Hostname | Device | Owner/Notes |
|----|-----|----------|--------|-------------|
| 192.168.12.1 | 18-60-41-b1-f2-a0 | TMO-G5AR.lan | T-Mobile gateway | Router/admin at http://192.168.12.1 |
| 192.168.12.100 | 14-ac-60-70-69-99 | BRW14AC60706999.lan | Brother printer | Printer #1 |
| 192.168.12.121 | — | — | iPad | Eric's iPad |
| 192.168.12.141 | c2-28-63-78-4f-41 (random) | — | iPad | Kids/other iPad (private MAC) |
| 192.168.12.162 | fc-3c-d7-62-b5-6c | wlan0.lan | Raspberry Pi | MILTONRP3 — secondary Pi-hole |
| 192.168.12.163 | c8-2a-14-51-d5-4a | — | Mac Mini (Fedora) | Pi-hole server — SSH: `ssh mac@192.168.12.163` / pw: 645866 |
| 192.168.12.165 | d0-03-4b-e8-0c-63 | Living-Room.lan | Apple TV | Living room — AirPlay ports 5000/7000 |
| 192.168.12.166 | 60-6d-c7-84-e9-77 | BRW606DC784E977.lan | Brother device | Brother device #2 |
| 192.168.12.177 | cc-b1-48-0d-2d-f4 | TMOBILE_MESH_AP_V2.lan | T-Mobile mesh AP | T-Mobile hardware |
| 192.168.12.189 | 34-36-3b-c6-bc:70 | — | MacBook Pro (Fedora) | SSH: `ssh -i ~/.ssh/id_ed25519 ericmilton@192.168.12.189` |
| 192.168.12.194 | e8-84-a5-82-df-fc | — | Unknown | Unidentified — only responds to ICMP |
| 192.168.12.215 | 3e-e7-96-3e-55-e9 (random) | iPhone.lan | iPhone | Rosemary's iPhone |
| 192.168.12.219 | — | Eric.lan | Windows 11 PC | Eric's main PC |
| 192.168.12.220 | b0-47-e9-e3-78-d0 | host.docker.internal | Chromebook (ChromeOS) | Patrick's Chromebook — running Docker (Linux on ChromeOS) |
| 192.168.12.226 | 06-f0-9a-33-c5-6c (random) | — | Asus laptop (Fedora) | Gianna's laptop — SSH open |
| 192.168.12.237 | b8-c7-5d-12-c0-4d | — | MacBook Pro (macOS) | Rosemary's MacBook Pro |
| 192.168.12.238 | e0-63-da-c7-43-84 | ubnt.lan | Ubiquiti UniFi AP | Pending adoption into UniFi controller |
| 192.168.12.239 | c8-15-4e-2c-03-b7 | kids2.lan | Kids device #2 | Unconfirmed device type |
| 192.168.12.249 | c8-15-4e-1c-a6-ba | kids1.lan | Kids device #1 | Unconfirmed device type |

### Tailscale Node
| Tailscale IP | Device | Notes |
|--------------|--------|-------|
| 100.115.92.195 | Ev's Chromebook | On Tailscale — also has a 192.168.12.x address |

---

## Key Servers

| Device | IP | Role |
|--------|----|------|
| Mac Mini (Fedora) | 192.168.12.163 | Pi-hole DNS, future WireGuard server |
| Patrick's Chromebook | 192.168.12.220 | Docker host — `host.docker.internal` |
| Raspberry Pi | 192.168.12.162 | Secondary Pi-hole |

---

## Network Plan

- **Pi-hole** on Mac Mini (.163) — DIEMILTONHAUS DNS routes through it
- **WireGuard** to be installed on every device so they can reach the Milton Home Page at `http://192.168.0.100:5006/`
- Eric's Windows PC already has WireGuard running (Lambert tunnel → 192.168.0.x and 192.168.2.x)
- Goal: replicate the Lambert WireGuard config on all other devices

---

## How to Rescan

Save as `deepscan.ps1` on a Windows PC and run with `powershell -ExecutionPolicy Bypass -File deepscan.ps1`:

```powershell
$subnet = "192.168.12"
$found = @{}
$jobs = 1..254 | ForEach-Object {
    (New-Object Net.NetworkInformation.Ping).SendPingAsync("$subnet.$_", 1000)
}
[System.Threading.Tasks.Task]::WaitAll($jobs)
arp -a | Select-String "$subnet\.\d+" | Where-Object { $_ -notmatch "static" }
```

For devices that block ping, also TCP-probe ports: 80, 443, 22, 5000, 7000, 62078, 3389, 445
