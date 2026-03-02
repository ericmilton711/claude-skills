# Current Tasks

Last Updated: 2026-01-02

## In Progress

None currently

## Recently Completed ✅

### High Priority 🔴

- [x] **Configure Firefox to Use Pi-hole DNS** (Completed: 2026-01-02)
  - [x] Disable DNS over HTTPS (DoH)
  - [x] Find correct Firefox profile (Y7khd5ii.Profile 1)
  - [x] Set network.trr.mode=5 in preferences
  - [x] Clear Firefox cache
  - [x] Verify DNS queries go through Pi-hole

- [x] **Block YouTube Completely** (Completed: 2026-01-02)
  - [x] Block youtube.com, www.youtube.com, m.youtube.com
  - [x] Block youtu.be shortlinks
  - [x] Block googlevideo.com (video content)
  - [x] Block YouTube support domains (i.ytimg.com, yt3.ggpht.com, etc.)
  - [x] Test blocking is effective
  - [x] Document how to unblock if needed

- [x] **Install Beeper Chat Application** (Completed: 2026-01-02)
  - [x] Download Beeper AppImage (v4.2.367)
  - [x] Install FUSE libraries (fuse-libs)
  - [x] Set up AppImage in /home/mac/.local/bin/
  - [x] Create desktop launcher entry
  - [x] Test application launch

- [x] **Install Pi-hole for Network-Wide Ad Blocking** (Completed: 2026-01-02)
  - [x] Check system requirements
  - [x] Download and run Pi-hole installer
  - [x] Configure Pi-hole settings (admin password, upstream DNS)
  - [x] Set up DNS configuration  - [x] Test ad blocking functionality
  - [x] Configure blocklists (82,907 domains blocked)
  - [x] Set up Pi-hole web admin interface
  - [x] Open firewall ports (HTTP, HTTPS, DNS)

  **Installation Details**:
  - Web Admin: http://192.168.1.7/admin
  - Admin Password: PASSword!?1711
  - DNS Server: 192.168.1.7:53
  - Blocking: 82,907 domains from Steven Black's hosts list
  - Upstream DNS: 1.1.1.1 (Cloudflare), 1.0.0.1
  - Interface: enp2s0f0
  - Pi-hole version: v6.3 (Core), v6.4 (Web), v6.4.1 (FTL)

  **How to Use**:
  - Access web interface: http://192.168.1.7/admin
  - Set this computer's DNS to 192.168.1.7
  - OR set router's DNS to 192.168.1.7 (network-wide)
  - OR update WireGuard to use 192.168.1.7 as DNS

## Backlog (Not Started)

### Network Configuration
- [ ] Document complete network topology
- [ ] Set up network monitoring (optional)
- [ ] Configure firewall rules for Pi-hole
- [ ] Set up automatic Pi-hole updates

### VPN Improvements
- [ ] Configure WireGuard to start on boot (optional)
- [ ] Set up VPN kill switch (optional)
- [ ] Test DNS leak protection
- [ ] Monitor VPN performance

### Documentation
- [ ] Create network diagram
- [ ] Document all IP addresses and services
- [ ] Write troubleshooting guide

## Notes
- WireGuard VPN is active on Lambert interface (192.168.2.2)
- VPN DNS updated to use Pi-hole (192.168.1.7)
- Local network: 192.168.1.0/24
- VPN networks: 192.168.0.0/24, 192.168.2.0/24
- Firefox configured to use system DNS (DoH disabled)
- YouTube completely blocked at DNS level
- Beeper chat app installed and ready to use
- AppImage support enabled with FUSE libraries
