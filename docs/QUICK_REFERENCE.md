# Quick Reference Guide

Last Updated: 2026-01-02

## System Information
- **OS**: Fedora Linux 43
- **Hostname**: (current system)
- **Local IP**: 192.168.1.7
- **VPN IP**: 192.168.2.2

## Services & Applications

### Pi-hole DNS Ad Blocker
- **Web Interface**: http://192.168.1.7/admin
- **Admin Password**: PASSword!?1711
- **DNS Server**: 192.168.1.7:53
- **Blocking**: 82,907 domains
- **Status**: `sudo pihole status`
- **Enable/Disable**: `sudo pihole enable|disable`
- **Update Blocklists**: `sudo pihole -g`
- **View Logs**: `sudo pihole -t`

### WireGuard VPN (Lambert)
- **Interface**: Lambert
- **Server**: edenredux.servegame.com:51820
- **VPN IP**: 192.168.2.2
- **DNS**: 192.168.1.7 (Pi-hole)
- **Config**: /etc/wireguard/Lambert.conf
- **Connect**: `sudo wg-quick up Lambert`
- **Disconnect**: `sudo wg-quick down Lambert`
- **Status**: `sudo wg show`
- **Allowed Networks**: 192.168.0.0/24, 192.168.2.0/24

### Beeper Chat
- **Version**: 4.2.367
- **Location**: /home/mac/.local/bin/Beeper.AppImage
- **Launch**: Search "Beeper" in app menu or run `/home/mac/.local/bin/Beeper.AppImage`
- **Desktop Entry**: /home/mac/.local/share/applications/beeper.desktop

## Network Configuration

### IP Addresses
- **Local Network**: 192.168.1.0/24
- **This Computer**: 192.168.1.7 (enp2s0f0)
- **VPN Network**: 192.168.2.0/24
- **VPN IP**: 192.168.2.2 (Lambert)
- **Remote Network**: 192.168.0.0/24 (via VPN)

### DNS Configuration
- **Primary DNS**: 192.168.1.7 (Pi-hole)
- **Upstream DNS**: 1.1.1.1, 1.0.0.1 (Cloudflare)
- **Firefox**: DoH disabled (network.trr.mode=5)

## Blocked Content

### YouTube - Completely Blocked
Blocked domains:
- youtube.com, www.youtube.com, m.youtube.com
- youtu.be
- googlevideo.com
- s.youtube.com
- i.ytimg.com
- yt3.ggpht.com
- youtube-ui.l.google.com
- googleadservices.com
- video-stats.l.google.com

**To Unblock YouTube** (if needed):
```bash
sudo pihole allow youtube.com www.youtube.com m.youtube.com youtu.be googlevideo.com s.youtube.com i.ytimg.com yt3.ggpht.com youtube-ui.l.google.com googleadservices.com video-stats.l.google.com
```

## Firefox Configuration
- **Active Profile**: Y7khd5ii.Profile 1
- **Profile Path**: /home/mac/.mozilla/firefox/Y7khd5ii.Profile 1
- **DNS over HTTPS**: Disabled (network.trr.mode=5)
- **DNS Prefetch**: Disabled

## Important Files & Locations
- **Project Docs**: /home/mac/docs/
- **Task Lists**: /home/mac/tasks/
- **WireGuard Config**: /etc/wireguard/Lambert.conf
- **Pi-hole Config**: /etc/pihole/
- **Firefox Profile**: /home/mac/.mozilla/firefox/Y7khd5ii.Profile 1/
- **Beeper App**: /home/mac/.local/bin/Beeper.AppImage

## Troubleshooting

### Pi-hole Not Blocking
1. Check Pi-hole is running: `sudo pihole status`
2. Verify DNS is set to 192.168.1.7: `resolvectl status`
3. Check Firefox DoH is disabled: about:config → network.trr.mode = 5
4. Clear browser cache
5. Test with: `dig @192.168.1.7 doubleclick.net` (should return 0.0.0.0)

### VPN Not Connecting
1. Check VPN status: `sudo wg show`
2. Check config exists: `ls -l /etc/wireguard/Lambert.conf`
3. Try reconnecting: `sudo wg-quick down Lambert && sudo wg-quick up Lambert`
4. Check endpoint is reachable: `ping edenredux.servegame.com`

### Beeper Won't Launch
1. Check FUSE is installed: `rpm -q fuse-libs`
2. If missing: `sudo dnf install fuse-libs`
3. Check AppImage is executable: `ls -l /home/mac/.local/bin/Beeper.AppImage`
4. Make executable: `chmod +x /home/mac/.local/bin/Beeper.AppImage`

## System Credentials
- **Sudo Password**: 645866
- **Pi-hole Admin**: PASSword!?1711

## Documentation
For detailed information, see:
- `/home/mac/docs/PROJECT.md` - Project overview and status
- `/home/mac/tasks/TODO.md` - Current and upcoming tasks
- `/home/mac/tasks/DONE.md` - Completed tasks archive
