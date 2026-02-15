# Pi-hole Parental Controls Setup

**Date:** 2026-01-10
**Status:** Active

## Overview

Pi-hole on the Raspberry Pi (192.168.1.7) is configured to block ALL domains except a whitelist. This allows only specific services to be accessed by devices on the network.

## What's Allowed

| Target | Type | Status |
|--------|------|--------|
| http://192.168.0.100:5006 | Direct IP | Works (no DNS needed) |
| api.anthropic.com | Whitelisted | Works (Claude API) |
| anthropic.com | Whitelisted | Works |
| claude.ai | Whitelisted | Works |
| homeschoolconnections.com | Regex whitelist | Homeschool Connections |
| caravel.software | Regex whitelist | Caravel assets/storage |
| cloudfront.net | Regex whitelist | CDN |
| amazonaws.com | Regex whitelist | AWS |
| gstatic.com | Regex whitelist | Google static |
| googleapis.com | Regex whitelist | Google APIs |
| google.com | Regex whitelist | Google |
| vimeo.com | Regex whitelist | Video hosting |
| vimeocdn.com | Regex whitelist | Vimeo CDN |

## What's Blocked

Everything else - all websites, apps, services that use domain names.

## How It Works

1. Pi-hole has a regex rule `.*` that blocks ALL domain queries
2. Specific domains are whitelisted (type 0 in domainlist)
3. Direct IP access (like 192.168.0.100) bypasses DNS entirely
4. All devices using Pi-hole as DNS (via USG DHCP) are affected

## Configuration Details

**Pi-hole Database:** `/etc/pihole/gravity.db`

**Block-all rule:**
- Domain: `.*`
- Type: 3 (regex blacklist)

**Whitelisted domains:**
- api.anthropic.com (type 0 - whitelist)
- anthropic.com (type 0 - whitelist)
- claude.ai (type 0 - whitelist)

## Management Commands

### View current rules
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "SELECT domain, type, comment FROM domainlist;"
```

### Add a whitelisted domain
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('example.com', 0, 1, 'Description');"
sudo pihole reloaddns
```

### Remove a whitelisted domain
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = 'example.com' AND type = 0;"
sudo pihole reloaddns
```

### Disable parental controls (allow all)
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = '.*' AND type = 3;"
sudo pihole reloaddns
```

### Re-enable parental controls (block all)
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('.*', 3, 1, 'Block all domains');"
sudo pihole reloaddns
```

## Domain Types Reference

| Type | Meaning |
|------|---------|
| 0 | Whitelist (exact) |
| 1 | Blacklist (exact) |
| 2 | Whitelist (regex) |
| 3 | Blacklist (regex) |

## Testing

**Test blocked domain:**
```bash
dig +short google.com @127.0.0.1
# Should return: 0.0.0.0
```

**Test whitelisted domain:**
```bash
dig +short api.anthropic.com @127.0.0.1
# Should return: actual IP address
```

## Important Notes

1. **Claude needs api.anthropic.com** - If this is not whitelisted, Claude Code will stop working

2. **Direct IP access always works** - Pi-hole only blocks DNS, so accessing services via IP address (like 192.168.0.100:5006) is not affected

3. **Affects all DHCP devices** - Any device getting DNS from the USG (which points to Pi-hole) will be restricted

4. **To bypass on a device** - Change the device's DNS to something other than 192.168.1.7 (e.g., 8.8.8.8)

## Troubleshooting

### Claude stopped working
Whitelist the API domain:
```bash
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('api.anthropic.com', 0, 1, 'Claude API');"
sudo pihole reloaddns
```

### Need to allow a specific site temporarily
```bash
# Add whitelist
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "INSERT OR IGNORE INTO domainlist (domain, type, enabled, comment) VALUES ('needed-site.com', 0, 1, 'Temporary access');"
sudo pihole reloaddns

# Remove when done
sudo pihole-FTL sqlite3 /etc/pihole/gravity.db "DELETE FROM domainlist WHERE domain = 'needed-site.com';"
sudo pihole reloaddns
```

### Check if parental controls are active
```bash
dig +short google.com @127.0.0.1
# If returns 0.0.0.0 = blocked (controls active)
# If returns real IPs = not blocked (controls inactive)
```

## Related Documentation

- `/home/mac/.claude/skills/project-management/lambert-network-access.md`
- `/home/mac/.claude/skills/project-management/pihole-setup.md`
