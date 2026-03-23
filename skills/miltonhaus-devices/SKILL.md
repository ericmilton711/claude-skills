---
name: miltonhaus-devices
description: All devices on the MILTONHAUS network — IPs, MACs, hostnames, and what they are
type: reference
---

# MILTONHAUS — All Network Devices

**Last scanned:** 2026-03-23
**Scanner:** Eric's Windows PC (192.168.12.219) via PowerShell ping sweep

---

## How to Rescan

```powershell
# Save as scan.ps1 and run with: powershell -ExecutionPolicy Bypass -File scan.ps1
$jobs = 1..254 | ForEach-Object {
    (New-Object Net.NetworkInformation.Ping).SendPingAsync("192.168.12.$_", 300)
}
[System.Threading.Tasks.Task]::WaitAll($jobs)
arp -a | Select-String "192.168.12\.\d+ " | Where-Object { $_ -notmatch "static" }
```

Then look up hostnames:
```powershell
[System.Net.Dns]::GetHostEntry("192.168.12.x").HostName
```

---

## 192.168.12.x — DIEMILTONHAUS (T-Mobile Network)

| IP | MAC Address | Hostname | Identity |
|----|-------------|----------|----------|
| 192.168.12.1 | 18-60-41-b1-f2-a0 | TMO-G5AR.lan | T-Mobile gateway (G5AR) |
| 192.168.12.100 | 14-ac-60-70-69-99 | BRW14AC60706999.lan | Brother printer |
| 192.168.12.162 | fc-3c-d7-62-b5-6c | wlan0.lan | Raspberry Pi (MILTONRP3?) — has Pi-hole |
| 192.168.12.163 | b8-c7-5d-12-c0-4d | (none) | Mac Mini — Pi-hole server (SSH: mac@192.168.12.163 / 645866) |
| 192.168.12.165 | d0-03-4b-e8-0c-63 | Living-Room.lan | Smart home device (TBD) |
| 192.168.12.166 | 60-6d-c7-84-e9-77 | BRW606DC784E977.lan | Brother printer #2 |
| 192.168.12.177 | cc-b1-48-0d-2d-f4 | TMOBILE_MESH_AP_V2.lan | T-Mobile mesh access point |
| 192.168.12.189 | — | — | Fedora MacBook Pro (SSH key: ericmilton / may be offline) |
| 192.168.12.219 | — | Eric.lan | Eric's Windows PC (this machine) |
| 192.168.12.237 | c8-2a-14-51-d5-4a | (none) | Unknown — possibly Rosemary's Mac |
| 192.168.12.238 | e0-63-da-c7-43-84 | ubnt.lan | **Ubiquiti UniFi AP** |

> Note: IPs above are DHCP-assigned and can change. MACs are stable identifiers.

---

## Routing Table (Eric's Windows PC)

| Destination | Next Hop | Interface | Meaning |
|-------------|----------|-----------|---------|
| 0.0.0.0/0 | 192.168.12.1 | Wi-Fi 4 | Default route via T-Mobile |
| 192.168.0.0/24 | direct | Lambert | Lambert neighbor network (Milton Home Page lives here) |
| 192.168.2.0/24 | direct | Lambert | Lambert secondary subnet |

**192.168.0.100:5006** = Milton Home Page — currently only reachable because this PC has a physical Lambert network connection.
**Goal:** Replace Lambert dependency with WireGuard tunnel through Mac Mini (192.168.12.163).

---

## Known Devices by Location/Role

| Device | IP | Notes |
|--------|----|-------|
| T-Mobile Gateway | 192.168.12.1 | Admin at http://192.168.12.1 |
| Mac Mini (Fedora) | 192.168.12.163 | Pi-hole, future WireGuard server |
| UniFi AP | 192.168.12.238 | ubnt — pending adoption into UniFi controller |
| Raspberry Pi | 192.168.12.162 | wlan0.lan — secondary Pi-hole |
| Eric's PC | 192.168.12.219 | Windows 11 |
| Rosemary's Mac | 192.168.12.237? | TBC — was .109 before |

---

## Devices Still To Identify

- 192.168.12.165 — Living-Room.lan
- 192.168.12.237 — no hostname (Rosemary's Mac?)
