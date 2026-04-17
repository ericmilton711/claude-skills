---
name: miltonhaus-pihole-rules
description: Pi-hole default-deny whitelist config for kids devices — blocks everything, allows only approved sites plus Matt's WireGuard services
type: reference
---

# Pi-hole: Default-Deny Whitelist Configuration

**Server:** ThinkCentre M700 at 192.168.12.136 (NOT the Mac Mini)
Pi-hole blocks ALL domains by default. Only explicitly whitelisted domains resolve.
Kids access Matt's services (Milton, Nextcloud) by direct IP over WireGuard — no DNS needed.

---

## Accessing Pi-hole — REST API (No SSH Required)

Pi-hole v6 has a full REST API. Use HTTP from any device on the network — no need to SSH into the ThinkCentre.

### Authenticate
```bash
curl -s -X POST http://192.168.12.136/api/auth \
  -d '{"password":"645866"}' -H "Content-Type: application/json"
```
Returns a session ID (`sid`) — use it in all subsequent requests as a header: `-H "sid: <SID>"`

### Common API Endpoints
```
GET  /api/clients              — list all clients and their group assignments
GET  /api/groups               — list all groups
GET  /api/domains              — list all allow/deny domain rules and their group assignments
GET  /api/queries?client=<IP>  — query log filtered by client IP
POST /api/domains/allow/exact  — add an exact-match whitelist entry
     body: {"domain":"example.com","comment":"reason","groups":[4]}
```

### Workflow: Unblock a Site for a Specific Device
1. Authenticate → get SID
2. `GET /api/clients` → find the device's group number
3. `POST /api/domains/allow/exact` → add domain to that group
4. Check query log for blocked CDN/asset domains the site needs
5. Whitelist those too, then have the user hard-refresh (Ctrl+Shift+R)

---

## Matt's Services (Access by IP over WireGuard — NO DNS needed)

| Service | URL |
|---------|-----|
| Milton Home Page | http://192.168.0.165:5006 |
| Nextcloud | http://192.168.0.165:11000 |

> Use direct IP:port only. Matt's nginx uses name-based routing — hostnames won't work from outside his LAN.

---

## Pi-hole Groups

| Group ID | Name | Description |
|----------|------|-------------|
| 0 | Default | Default group |
| 1 | mac-mini | Mac Mini - block all |
| 2 | kids1 | Kids1 laptop - limited whitelist |
| 3 | kids2 | Kids2 Windows laptop |
| 4 | patricks-chromebook | Patrick's Chromebook |
| 5 | ev-chromebook | Ev's Chromebook |
| 6 | ev-temp-unrestricted | Ev's Chromebook - temp full access |

## Client Assignments

| IP | Comment | Group |
|----|---------|-------|
| 192.168.12.163 | Mac Mini | 1 (mac-mini) |
| 192.168.12.249 | Kids1 laptop | 2 (kids1) |
| 192.168.12.239 | Kids2 Windows laptop | 3 (kids2) |
| 192.168.12.220 | Patrick Chromebook | 4 (patricks-chromebook) |
| 192.168.12.221 | Patrick Chromebook (alt IP) | 4 (patricks-chromebook) |
| 192.168.12.164 | (unidentified) | 1 (mac-mini) |
| 192.168.12.194 | Ev Chromebook | 6 (ev-temp-unrestricted) |

---

## Per-Device Whitelists

### Patrick's Chromebook (Group 4) — Allowed Domains

**Educational:**
- homeschoolconnections.com, caravel.software
- teachingtextbooks.com, teachingtextbooksapp.com
- duolingo.com
- vimeo.com, vimeocdn.com
- kiddle.co, www.kiddle.co
- britannica.com, www.britannica.com, cdn.britannica.com

**Britannica support domains (required for site to function):**
- static.cloudflareinsights.com
- fonts.googleapis.com
- www.googleapis.com
- www.googletagmanager.com
- www.googletagservices.com
- launchpad-wrapper.privacymanager.io
- dev.visualwebsiteoptimizer.com

**Zoom (Homeschool Connections classes):**
- zoom.us, homeschoolconnections.zoom.us, us02web.zoom.us
- ssrweb.zoom.us, ssrweb-cf.zoom.us, st1.zoom.us, us02st1.zoom.us
- explore.zoom.us, us.telemetry.zoom.us
- cdn.cookielaw.org, ssl.gstatic.com

**CDN/infra:**
- cloudfront.net, amazonaws.com

---

## Default Deny Rule

Regex deny `.*` applies to groups: 0, 1, 2, 3, 4, 5 — blocks ALL domains unless explicitly whitelisted.

---

## Whitelist: System Essentials

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

## Whitelist: OS Updates (add as needed per device type)

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

## WireGuard DNS Setting

In each device's WireGuard `.conf` file, the `DNS =` line should point to the Pi-hole:
```
DNS = 192.168.12.136
```

This ensures Pi-hole filters even when VPN is active.

---

## Testing Checklist

1. Enable WireGuard on device
2. http://192.168.0.165:5006 — Milton loads
3. http://192.168.0.165:11000 — Nextcloud loads
4. https://youtube.com — BLOCKED
5. Check Pi-hole query log via API: `GET /api/queries?client=<IP>&blocked=true`
