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

**CRITICAL:** The `.*` regex deny blocks everything. Exact allow entries (`/api/domains/allow/exact`) DO NOT override regex deny rules. You MUST use **regex allow** rules instead. And the allow rules MUST be assigned to the device's group — group 0 (Default) alone won't work if the device is in a different group.

**Recommended approach — direct DB (most reliable):**
1. SSH into ThinkCentre: `ssh -i ~/.ssh/id_ed25519 milton@192.168.12.136`
2. Find the device's group: `docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "SELECT * FROM client WHERE ip='<IP>';"`
3. Add regex allow domain: `docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('(^|[.])example[.]com$', 2, 1, 'reason');"`
   - type 2 = allow regex
4. Get the new domain's ID: `docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "SELECT id FROM domainlist WHERE domain='(^|[.])example[.]com$';"`
5. Assign to device's group: `docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES (<ID>, <GROUP_ID>);"`
6. Reload: `docker exec pihole pihole reloaddns`

**API approach (fragile — group assignment often fails):**
1. Authenticate → get SID
2. `GET /api/clients` → find the device's group number
3. `POST /api/domains/allow/regex` → add regex allow (but groups default to [0] only)
4. API PUT/DELETE for group changes is unreliable — falls back to treating IDs as domain strings. Use direct DB for group assignment.
5. Check query log for blocked CDN/asset domains the site needs
6. Whitelist those too, then have the user hard-refresh (Ctrl+Shift+R)

**Firefox also needs these regex allows to function:**
- `(^|[.])firefox[.]com$`
- `(^|[.])mozilla[.]com$`
- `(^|[.])mozilla[.]net$`
- `(^|[.])mozilla[.]org$`
- `(^|[.])ipv4only[.]arpa$`

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
| 4 | yti-chromebook | YTI Chromebook — legacy group (device moved to group 7) |
| 5 | ev-chromebook | Ev's Chromebook |
| 6 | ev-temp-unrestricted | Ev's Chromebook - temp full access |
| 7 | tower-of-gondor | Tower of Gondor (ThinkCentre M900) — DEFAULT-ALLOW with specific blocks |
| 8 | gianna-laptop | Gianna's Fedora laptop |
| 9 | eva-laptop | Eva's Windows laptop (.202) — default-deny; Gmail allowed, Chat/Search/YouTube blocked |

## Client Assignments

| IP | Comment | Group |
|----|---------|-------|
| 192.168.12.163 | Mac Mini | 1 (mac-mini) |
| 192.168.12.249 | Kids1 laptop | 2 (kids1) |
| 192.168.12.239 | Kids2 Windows laptop | 3 (kids2) |
| 192.168.12.220 | YTI Chromebook (old IP) | 4 (yti-chromebook) |
| 192.168.12.221 | YTI Chromebook (old IP) | 4 (yti-chromebook) |
| 192.168.12.219 | YTI Chromebook (current IP, 2026-05-05) | 7 (tower-of-gondor) |
| 192.168.12.164 | (unidentified) | 1 (mac-mini) |
| 192.168.12.194 | Ev Chromebook | 6 (ev-temp-unrestricted) |
| 192.168.12.160 | Tower of Gondor (M900) | 7 (tower-of-gondor) |
| 192.168.12.226 | Gianna Fedora laptop | 8 (gianna-laptop) |
| 192.168.12.202 | Eva Windows laptop | 6 (ev-temp-unrestricted) — moved from 9 on 2026-06-13, fully unrestricted |

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

Regex deny `.*` applies to groups: 0, 1, 2, 3, 5 — blocks ALL domains unless explicitly whitelisted.
Group 4 (yti-chromebook) was removed from the deny-all rule on 2026-05-04 — legacy group, no longer actively used.
Group 7 (tower-of-gondor) uses **default-allow with specific blocks** instead of deny-all (see below).

**YTI Chromebook (Patrick's Chromebook)** was moved to group 7 on 2026-05-05. Google search blocked. Gmail temporarily blocked as of 2026-06-02. DNS manually set to 192.168.12.136 on the Chromebook, and Secure DNS (DoH) disabled in `chrome://settings/security`.

---

## Group 7 (tower-of-gondor) — Default-Allow with Specific Blocks

**Devices in this group:**
1. **Tower of Gondor** (Lenovo ThinkCentre M900) at 192.168.12.160 (MAC: 44-85-00-3f-26-7c). Windows, local account `tower-of-gondor\user`, pw: 645866. WiFi DNS set via `netsh interface ip set dns "Wi-Fi" static 192.168.12.136`.
2. **YTI Chromebook** (Patrick's Chromebook, MAC: b0-47-e9-e3-78-d0) at 192.168.12.219 (DHCP, was .220/.221). ChromeOS. DNS manually set to 192.168.12.136 in WiFi settings. Secure DNS (DoH) disabled in `chrome://settings/security`. Added 2026-05-05.

**Approach:** Everything allowed by default. Specific sites blocked via regex deny rules in group 7.

**Blocked sites (regex deny, group 7):**
- `(^|[.])google[.]com$` — All google.com subdomains: search, Chat, Meet, Drive, accounts (ID 216)
- `(^|[.])gmail[.]com$` — Gmail blocked (ID 265, added 2026-06-02, **TEMPORARY**)
- `(^|[.])spotify[.]com$` — Spotify blocked (ID 223, added 2026-05-17)
- `(^|[.])scdn[.]co$` — Spotify CDN blocked (ID 224, added 2026-05-17)
- `(^|[.])apple[.]com$` — Apple Music blocked (ID 225, added 2026-05-17)
- `(^|[.])applemusic[.]com$` — Apple Music alt domain blocked (ID 226, added 2026-05-17)

**Pending:** YTI Chromebook should be migrated to its own default-deny group (like groups 2/3) instead of sharing default-allow Group 7. Needs whitelist of approved sites from Eric.

**Allowed via shared regex allows (group 7 added to existing rules):**
- Firefox: firefox.com, mozilla.com, mozilla.net, mozilla.org, ipv4only.arpa

**Important:** Do NOT add `google.com` as a regex allow for group 7 — it overrides the deny and re-enables Google search.

**To restore Gmail when the temporary block is lifted:**
```bash
ssh -i ~/.ssh/id_ed25519 milton@192.168.12.136
docker exec pihole pihole-FTL sqlite3 /etc/pihole/gravity.db "
  INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES (44, 7);
  INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES (42, 7);
  INSERT OR IGNORE INTO domainlist_by_group (domainlist_id, group_id) VALUES (202, 7);
  DELETE FROM domainlist_by_group WHERE domainlist_id = 265 AND group_id = 7;
"
docker exec pihole pihole reloaddns
```
(Restores: gmail.com ID 44, mail.google.com ID 42, accounts.google.com ID 202 back to Group 7; removes gmail.com deny ID 265)

---

## CRITICAL: Rule precedence & domainlist type enum

**Allow ALWAYS beats deny.** FTL evaluation order: (1) exact allow → (2) regex allow → (3) exact deny → (4) regex deny → (5) gravity. The first match wins. So a domain matched by ANY allow rule for the client's group is permitted even if a deny rule also matches it. Corollary: to block something, do NOT rely on adding a deny if a broad allow already covers it — you must narrow/remove the allow. Adding a deny only helps for domains that fall through to the `.*` catch-all.

**`domainlist.type` enum:** `0`=allow-exact, `1`=deny-exact, `2`=allow-regex (whitelist), `3`=deny-regex (blacklist). The default-deny `.*` rule is type 3. A google.com "allow" for kids is type 2; a google.com search "block" is type 3.

**Stale-cache gotcha:** After ANY rule change you MUST `docker exec pihole pihole reloaddns` (reloads lists AND flushes the DNS cache). A domain allowed for one group (e.g. group 0/localhost from your own `dig` tests, or another kid's group) gets cached and can appear to "resolve" for a default-deny client until the cache is flushed. Always reload, then verify with a FRESH query. On Windows clients also run `ipconfig /flushdns`, and remember **Firefox caches DNS separately** — a stuck tab needs Ctrl+R / Firefox restart.

**Verify from the actual device** (queries Pi-hole as that client's group): Windows `nslookup <domain>. 192.168.12.136` (trailing dot avoids the `.lan` suffix). Blocked = `0.0.0.0` / `::`. For TLS/HTTP issues use `curl.exe -sSIL <https-url>` over SSH.

---

## Playbook: Allow Gmail while keeping Google Search/YouTube blocked

Gmail and Search live on different hostnames, so this IS possible under default-deny. Allow these (type 2 regex) for the device's group — none of them re-enable Search:

```
(^|[.])mail[.]google[.]com$        Gmail inbox
(^|[.])gmail[.]com$                gmail.com entry
(^|[.])accounts[.]google[.]com$    Google sign-in
(^|[.])workspace[.]google[.]com$   *** Gmail landing page — gmail.com redirects a logged-OUT browser here; if blocked => "Server not found" ***
(^|[.])apis[.]google[.]com$        gapi (Gmail web app)
(^|[.])ogs[.]google[.]com$         One Google account bar
(^|[.])play[.]google[.]com$        push/FCM
(^|[.])googleapis[.]com$           backend APIs
(^|[.])gstatic[.]com$              static assets (incl. ssl./fonts.)
(^|[.])googleusercontent[.]com$    avatars/attachments
(^|[.])pki[.]goog$                 *** Google cert OCSP/CRL — if blocked, TLS revocation check fails => "Server not found" with NO cert warning. ALWAYS allow when allowing any Google HTTPS service ***
```

Stays BLOCKED (no allow added → `.*` catches them): `www.google.com` / `google.com` (Search), `youtube.com`.

**Two non-obvious breakers that look like a DNS outage ("Server not found"), diagnosed on Eva's laptop 2026-06-08:**
1. **`pki.goog` blocked** → `curl.exe` shows `(35) schannel CRYPT_E_REVOCATION_OFFLINE`. Cert revocation can't be checked → handshake aborts. Allow `pki.goog`.
2. **`workspace.google.com` blocked** → typing `gmail.com` in a logged-out browser redirects to `workspace.google.com/intl/en-US/gmail` (the landing page) which dies. (curl with no cookies skips this hop and goes straight to the sign-in chain, so it only reproduces in a real browser.) Allow `workspace.google.com`.

**Do NOT** broadly allow `clients[0-9]+.google.com` to "fix" Gmail contacts — it re-enables the Chat signaler (allow beats deny). Gmail email works fine without it.

**Possible next snag:** sign-in reCAPTCHA may load from the blocked `www.google.com`. If login itself stalls at a captcha, add `(^|[.])recaptcha[.]net$` for the group.

**Verified-working chain:** `gmail.com → mail.google.com → accounts.google.com → 200` (never touches www.google.com, so Search stays blocked).

---

## Playbook: Ban Google Chat (keep Gmail email working)

Google Chat is a separate product on its own hosts. Add these **deny-regex (type 3)** to the device's group:

```
(^|[.])chat[.]google[.]com$              Chat web UI + the in-Gmail Chat iframe
(^|[.])chat[.]usercontent[.]google[.]com$ Chat file content
dynamite.*[.]clients6[.]google[.]com$    Chat ("Dynamite" codename) real-time signaler
(^|[.])hangouts[.]google[.]com$          legacy Hangouts/Chat
```

Plus ensure `signaler-pa.clients6.google.com` is NOT allowed (don't add a broad clients6 allow). Result: Gmail email works fully; the Chat/Spaces panel inside Gmail fails to load. Verify: `nslookup chat.google.com. 192.168.12.136` → `0.0.0.0`.

---

## Group 9 (eva-laptop, .202) — current full Google rule set

ALLOW (type 2): accounts, mail, gmail, workspace, apis, ogs, play, googleapis, gstatic (+ssl/fonts), googleusercontent, lh3.googleusercontent, pki.goog, firefox/mozilla, windows update/microsoft, homeschoolconnections (covers caravel.homeschoolconnections.com via wildcard), caravel.software, teachingtextbooks(+app), duolingo, vimeo(+cdn), zoom.us, cloudfront, amazonaws, kiddle, britannica, detectportal.firefox.com.
DENY (type 3): chat.google.com, chat.usercontent.google.com, dynamite*signaler, hangouts.google.com — plus `.*` default-deny (so Search/YouTube/everything-else blocked).

---

## Helper: add an allow/deny rule scoped to a group (direct DB)

```bash
ssh -i ~/.ssh/id_ed25519 milton@192.168.12.136
DB=/etc/pihole/gravity.db
# TYPE: 2=allow-regex, 3=deny-regex ; GROUP: target group id
docker exec pihole pihole-FTL sqlite3 "$DB" "INSERT OR IGNORE INTO domainlist (type,domain,enabled,comment) VALUES (2,'(^|[.])example[.]com$',1,'reason');"
ID=$(docker exec pihole pihole-FTL sqlite3 "$DB" "SELECT id FROM domainlist WHERE type=2 AND domain='(^|[.])example[.]com$';")
docker exec pihole pihole-FTL sqlite3 "$DB" "INSERT OR IGNORE INTO domainlist_by_group (domainlist_id,group_id) VALUES ($ID,9);"
docker exec pihole pihole reloaddns
```

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
