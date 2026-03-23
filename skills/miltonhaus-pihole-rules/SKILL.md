---
name: miltonhaus-pihole-rules
description: Pi-hole default-deny whitelist config for kids devices — blocks everything, allows only approved sites plus Matt's WireGuard services
type: reference
---

# Pi-hole: Default-Deny Whitelist Configuration

Pi-hole blocks ALL domains by default. Only explicitly whitelisted domains resolve.
Kids access Matt's services (Milton, Nextcloud) by direct IP over WireGuard — no DNS needed.

---

## Matt's Services (Access by IP over WireGuard — NO DNS needed)

| Service | URL |
|---------|-----|
| Milton Home Page | http://192.168.0.165:5006 |
| Nextcloud | http://192.168.0.165:11000 |

> Use direct IP:port only. Matt's nginx uses name-based routing — hostnames won't work from outside his LAN.

---

## Pi-hole Setup

### Step 1 — Block Everything (Default Deny)

In Pi-hole admin (http://192.168.12.163/admin):
- Go to **Domains → Deny**
- Add regex deny: `.*`

This blocks ALL domains. Nothing resolves unless explicitly allowed.

### Step 2 — Whitelist: System Essentials

Devices need these to function (connectivity checks, time sync):

```
time.apple.com
time.windows.com
time.nist.gov
connectivity-check.ubuntu.com
connectivitycheck.gstatic.com
captive.apple.com
msftconnecttest.com
www.msftconnecttest.com
detectportal.firefox.com
```

### Step 3 — Whitelist: Nextcloud App

```
nextcloud.com
```

### Step 4 — Whitelist: OS Updates (add as needed per device type)

**Apple:**
```
mesu.apple.com
updates.cdn-apple.com
swscan.apple.com
appldnld.apple.com
```

**Android/Chromebook:**
```
play.googleapis.com
dl.google.com
connectivitycheck.gstatic.com
```

**Windows:**
```
update.microsoft.com
download.windowsupdate.com
windowsupdate.com
```

---

## Pi-hole Domain Type Reference

| Type | Meaning |
|------|---------|
| 0 | Whitelist (exact) |
| 1 | Blacklist (exact) |
| 2 | Whitelist (regex) |
| 3 | Blacklist (regex) — `.*` goes here |

Add via sqlite:
```bash
# Whitelist exact domain
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db \
  "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('example.com', 0, 1, 'reason');"
sudo pihole reloaddns
```

---

## Per-Device Groups (Advanced — configure later)

Pi-hole supports different rules per device. Under **Clients → Groups**:
- Assign devices by IP or MAC
- Older kids can have more sites allowed
- Query log shows exactly which domain was blocked when something breaks

---

## WireGuard DNS Setting

In each device's WireGuard `.conf` file, the `DNS =` line should point to the Pi-hole:
```
DNS = 192.168.12.163
```

This ensures Pi-hole filters even when VPN is active.

---

## Testing Checklist

1. Enable WireGuard on device
2. http://192.168.0.165:5006 — Milton loads
3. http://192.168.0.165:11000 — Nextcloud loads
4. https://youtube.com — BLOCKED
5. Check Pi-hole query log (Dashboard → Query Log) for real-time blocks
