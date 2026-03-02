# Completed Tasks

## 2026-01-02

### ✅ Uninstall ExpressVPN
**Completed**: 2026-01-02

- Stopped ExpressVPN service
- Removed /opt/expressvpn directory
- Removed systemd service files
- Removed ExpressVPN groups
- Removed browser helper manifests
- Removed command-line tools
- Cleaned up NetworkManager configuration
- Removed ExpressVPN from GNOME favorites bar

**Result**: ExpressVPN completely removed from system

---

### ✅ Set Up WireGuard VPN (Lambert)
**Completed**: 2026-01-02

Configuration:
- Installed Lambert.conf to /etc/wireguard/
- Set proper file permissions (600, root:root)
- Connected to VPN successfully

VPN Details:
- **Interface**: Lambert
- **VPN IP**: 192.168.2.2
- **DNS Server**: 192.168.2.1
- **Endpoint**: edenredux.servegame.com:51820 (174.54.51.209:51820)
- **Routing**: Split tunneling for 192.168.0.0/24 and 192.168.2.0/24

Management Commands:
- Connect: `sudo wg-quick up Lambert`
- Disconnect: `sudo wg-quick down Lambert`
- Status: `sudo wg show`

**Result**: WireGuard VPN active and working properly with successful handshake

---

### ✅ Install Pi-hole DNS Ad Blocker
**Completed**: 2026-01-02

Installation Steps:
- Downloaded Pi-hole installer from official source
- Configured automated installation with setupVars.conf
- Bypassed SELinux enforcement (PIHOLE_SELINUX=true)
- Installed Pi-hole v6.3 (Core), v6.4 (Web), v6.4.1 (FTL)
- Downloaded and processed 82,907 blocking domains
- Set admin password
- Opened firewall ports (HTTP, HTTPS, DNS)
- Disabled systemd-resolved DNSStubListener

Configuration:
- **Web Interface**: http://192.168.1.7/admin
- **Admin Password**: PASSword!?1711
- **DNS Port**: 53 (listening on 192.168.1.7)
- **Interface**: enp2s0f0 (192.168.1.7)
- **Upstream DNS**: 1.1.1.1, 1.0.0.1 (Cloudflare)
- **Blocklist**: Steven Black's hosts (82,907 domains)
- **Query Logging**: Enabled
- **DNSSEC**: Disabled
- **IPv6**: Enabled

Testing Results:
- ✅ DNS resolution working (google.com → 142.250.81.238)
- ✅ Ad blocking active (doubleclick.net → 0.0.0.0)
- ✅ FTL service running and enabled on boot
- ✅ Web interface accessible

Management Commands:
- Status: `sudo pihole status`
- Enable/Disable: `sudo pihole enable|disable`
- Update blocklists: `sudo pihole -g`
- Change password: `sudo pihole setpassword`
- View logs: `sudo pihole -t`

**Result**: Pi-hole successfully installed and blocking ads. Ready to configure DNS on devices or router.

---

### ✅ Configure Firefox to Use Pi-hole DNS
**Completed**: 2026-01-02

Steps Taken:
- Disabled DNS over HTTPS in Firefox (network.trr.mode = 5)
- Modified correct Firefox profile: Y7khd5ii.Profile 1
- Cleared Firefox cache
- Verified Firefox now queries Pi-hole for DNS

**Result**: Firefox successfully using Pi-hole for all DNS queries.

---

### ✅ Block YouTube Entirely
**Completed**: 2026-01-02

Blocked Domains:
- youtube.com
- www.youtube.com
- m.youtube.com
- youtu.be
- googlevideo.com
- s.youtube.com
- googleadservices.com
- video-stats.l.google.com
- i.ytimg.com
- yt3.ggpht.com
- youtube-ui.l.google.com

Reason: YouTube's sophisticated caching and QUIC protocol bypass DNS-level ad blocking. Complete site block implemented per user request.

**How to Unblock (if needed):**
```bash
sudo pihole allow youtube.com www.youtube.com m.youtube.com youtu.be googlevideo.com s.youtube.com i.ytimg.com yt3.ggpht.com youtube-ui.l.google.com googleadservices.com video-stats.l.google.com
```

**Result**: YouTube completely inaccessible on the network.

---

### ✅ Install Beeper Chat
**Completed**: 2026-01-02

Installation Steps:
- Downloaded Beeper AppImage v4.2.367
- Installed to /home/mac/.local/bin/Beeper.AppImage
- Created desktop entry for application menu
- Made executable and configured launcher

Launch Methods:
- Application menu: Search for "Beeper"
- Command line: `/home/mac/.local/bin/Beeper.AppImage`

**Result**: Beeper chat application successfully installed and available in application menu.

---
