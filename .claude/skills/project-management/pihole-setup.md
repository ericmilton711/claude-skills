# Pi-hole Setup & Configuration

**Last Updated:** 2026-03-23

## Pi-hole Instances

### 1. Mac (Fedora) — Primary
- **Hostname:** fedora
- **IP Address:** 192.168.12.163
- **Interface:** enp2s0f0
- **Admin URL:** http://192.168.12.163/admin
- **Admin Password:** None (removed)
- **Pi-hole Version:** v6.3 (Core), v6.4 (Web), v6.4.1 (FTL)
- **Upstream DNS:** 1.1.1.1, 1.0.0.1
- **Blocklist:** 79,814 domains
- **Config File:** /etc/pihole/pihole.toml
- **Database:** /etc/pihole/gravity.db
- **Service:** pihole-FTL (systemd)
- **SSH:** `ssh mac@192.168.12.163` (password: 645866)

### 2. Raspberry Pi 3 A+ (MILTONRP3) — Secondary
- **Hostname:** MILTONRP3
- **IP Address:** 192.168.1.104
- **Interface:** wlan0
- **Admin URL:** http://192.168.1.104/admin
- **Pi-hole Version:** v6.3 (Core), v6.4 (Web), v6.4.1 (FTL)
- **Upstream DNS:** 1.1.1.1, 8.8.8.8
- **Blocklist:** 75,488 domains (StevenBlack/hosts)
- **SSH:** `ssh miltonrp3@192.168.1.104` (password: raspberry123)

---

## Parental Controls (Block-All Mode)

Pi-hole is configured to block ALL domains by default, with specific sites whitelisted.

### How It Works
1. Regex rule `.*` (type 3) blocks ALL domain queries
2. Whitelisted domains (type 0 and type 2) are allowed through
3. Direct IP access bypasses DNS entirely

### Whitelisted Domains

| Domain | Type | Purpose |
|--------|------|---------|
| api.anthropic.com | Exact (0) | Claude API |
| anthropic.com | Exact (0) | Anthropic website |
| claude.ai | Exact (0) | Claude web |
| homeschoolconnections.com | Exact (0) | Homeschool Connections |
| caravel.homeschoolconnections.com | Exact (0) | Caravel platform |
| cloudfront.net | Exact (0) | CloudFront CDN |
| homeschoolconnections.zoom.us | Exact (0) | HSC Zoom classes |
| `(\|^)homeschoolconnections\.com$` | Regex whitelist (2) | HSC wildcard |
| `(\|^)cloudfront\.net$` | Regex whitelist (2) | CloudFront wildcard |
| `(\|^)amazonaws\.com$` | Regex whitelist (2) | AWS |
| `(\|^)gstatic\.com$` | Regex whitelist (2) | Google static |
| `(\|^)googleapis\.com$` | Regex whitelist (2) | Google APIs |
| `(\|^)google\.com$` | Regex whitelist (2) | Google |
| `(\|^)vimeo\.com$` | Regex whitelist (2) | Vimeo video |
| `(\|^)vimeocdn\.com$` | Regex whitelist (2) | Vimeo CDN |
| `(\|^)caravel\.software$` | Regex whitelist (2) | Caravel software |
| `(\|^)www\.lifeprint\.com$` | Regex whitelist (2) | Lifeprint (ASL) |
| `(\|^)www\.duolingo\.com$` | Regex whitelist (2) | Duolingo |
| `(\|^)claude\.ai$` | Regex whitelist (2) | Claude AI |

### Blocked Domains (Explicit Blacklist)

| Domain | Type | Purpose |
|--------|------|---------|
| `.*` | Regex blacklist (3) | Block ALL domains |
| `(^|\.)youtube\.com$` | Regex blacklist (3) | Block YouTube |
| `(^|\.)youtubei\.googleapis\.com$` | Regex blacklist (3) | Block YouTube API |
| `(^|\.)youtube-nocookie\.com$` | Regex blacklist (3) | Block YouTube embeds |
| `(^|\.)googlevideo\.com$` | Regex blacklist (3) | Block YouTube video CDN |
| `(^|\.)ytimg\.com$` | Regex blacklist (3) | Block YouTube images |
| `(^|\.)ggpht\.com$` | Regex blacklist (3) | Block YouTube thumbnails |
| googlevideo.com | Exact blacklist (1) | YouTube video CDN |
| googleadservices.com | Exact blacklist (1) | Google ad services |
| s.youtube.com | Exact blacklist (1) | YouTube |
| video-stats.l.google.com | Exact blacklist (1) | YouTube stats |
| youtube.com, www.youtube.com, m.youtube.com | Exact blacklist (1) | YouTube |
| youtu.be | Exact blacklist (1) | YouTube short links |
| i.ytimg.com, yt3.ggpht.com | Exact blacklist (1) | YouTube images |
| youtube-ui.l.google.com | Exact blacklist (1) | YouTube UI |
| youtubei.googleapis.com | Exact blacklist (1) | YouTube API |

---

## Management Commands

### Service Control
```bash
# Start Pi-hole
sudo systemctl start pihole-FTL

# Stop Pi-hole
sudo systemctl stop pihole-FTL

# Enable on boot
sudo systemctl enable pihole-FTL

# Check status
pihole status
```

### Password Management
```bash
# Remove password (no login required)
sudo pihole setpassword

# Set a password
sudo pihole setpassword mypassword
```

### Domain Management
```bash
# View all rules
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "SELECT domain, type, enabled, comment FROM domainlist;"

# Add whitelisted domain (exact)
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('example.com', 0, 1, 'Description');"
sudo pihole reloaddns

# Add whitelisted domain (regex - allows all subdomains)
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('(\.|^)example\.com$', 2, 1, 'Example wildcard');"
sudo pihole reloaddns

# Remove a domain
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = 'example.com';"
sudo pihole reloaddns

# Disable parental controls (allow all)
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = '.*' AND type = 3;"
sudo pihole reloaddns

# Re-enable parental controls (block all)
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('.*', 3, 1, 'Block all domains');"
sudo pihole reloaddns
```

### Domain Type Reference

| Type | Meaning |
|------|---------|
| 0 | Whitelist (exact) |
| 1 | Blacklist (exact) |
| 2 | Whitelist (regex) |
| 3 | Blacklist (regex) |

---

## Configuring Devices to Use Pi-hole

### Option 1: Router-Wide (Recommended)
1. Access router admin at http://192.168.12.1
2. Set Primary DNS to `192.168.12.163`
3. Save and reboot — all devices automatically use Pi-hole

### Option 2: Per-Device

**Windows (PowerShell as Admin):**
```powershell
# Find your adapter name
Get-NetAdapter

# Set DNS
Set-DnsClientServerAddress -InterfaceAlias 'Wi-Fi' -ServerAddresses 192.168.12.163

# Verify
Get-DnsClientServerAddress -InterfaceAlias 'Wi-Fi' -AddressFamily IPv4
```

**macOS:**
1. System Settings → Network → Wi-Fi → Details → DNS
2. Add `192.168.12.163`, remove others

**iOS/Android:**
1. Wi-Fi settings → tap network → DNS → Manual
2. Set to `192.168.12.163`

**Linux:**
```bash
# Using nmcli
nmcli con mod "CONNECTION_NAME" ipv4.dns "192.168.12.163"
nmcli con up "CONNECTION_NAME"
```

### To Bypass Pi-hole on a Device
Change the device's DNS to `8.8.8.8` or `1.1.1.1`.

---

## Testing & Verification

```bash
# Check if Pi-hole is running
pihole status

# Test blocked domain (should return 0.0.0.0)
dig +short youtube.com @192.168.12.163

# Test whitelisted domain (should return real IP)
dig +short api.anthropic.com @192.168.12.163

# Test from any device
nslookup google.com 192.168.12.163
```

---

## Troubleshooting

### DNS service not running
```bash
sudo systemctl enable --now pihole-FTL
pihole status
```

### Claude stopped working
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('api.anthropic.com', 0, 1, 'Claude API');"
sudo pihole reloaddns
```

### Need temporary access to a site
```bash
# Allow
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('needed-site.com', 0, 1, 'Temporary');"
sudo pihole reloaddns

# Remove when done
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = 'needed-site.com';"
sudo pihole reloaddns
```

---

## Network Information

| Device | IP Address | Role |
|--------|------------|------|
| Router/Gateway | 192.168.12.1 | Network gateway |
| Mac (Fedora) | 192.168.12.163 | Pi-hole DNS (primary) |
| Raspberry Pi | 192.168.1.104 | Pi-hole DNS (secondary, separate network) |

**Network:** MILTONHAUS2
**ISP:** T-Mobile Home Internet

---

**Setup originally completed:** January 3, 2026
**Last verified:** March 23, 2026
