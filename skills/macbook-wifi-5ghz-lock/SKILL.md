# MacBook Pro — Wi-Fi Slow Internet / 5 GHz Band Lock

Eric's MacBook Pro (Fedora, wlp3s0). Written 2026-10-12 from a "the internet is ridiculously slow" session.

## Diagnosis (run first)
```bash
ip route show default; nmcli -f IN-USE,BSSID,SSID,CHAN,FREQ,SIGNAL,RATE device wifi list --rescan yes | grep -E "IN-USE|DIEMILTONHAUS"
ping -c5 192.168.12.1; ping -c5 1.1.1.1
dig google.com @192.168.12.136 +time=3; dig google.com @1.1.1.1 +time=3
curl -s -o /dev/null -w "%{speed_download} B/s\n" https://speed.cloudflare.com/__down?bytes=25000000
```
Finding: laptop was joined to DIEMILTONHAUS on 2.4 GHz (ch 1, congested). Two APs broadcast both bands; 5 GHz is ch 44 (5220 MHz). Latency/DNS/Pi-hole were all healthy (gateway ~2 ms, DNS ~30 ms), so DNS is NOT the cause.

## Fix: lock the profile to 5 GHz
```bash
nmcli connection modify DIEMILTONHAUS 802-11-wireless.band a
nmcli connection up DIEMILTONHAUS
iw dev wlp3s0 link | grep freq    # expect 5220
```
Eric confirmed it felt "much faster." My single 25 MB test showed ~85-90 Mbit/s on both bands, so rely on Eric's perceived result, not that benchmark.

## Persistence
- Saved in /etc/NetworkManager/system-connections/DIEMILTONHAUS.nmconnection (band=a, autoconnect=yes).
- Survives reboots, power-off, Fedora upgrades (incl. F43->F44).
- Lost if: network is "Forgotten" and rejoined, band setting changed, or OS wiped/reinstalled.
- After a reinstall: rejoin DIEMILTONHAUS, then re-run the `nmcli connection modify ... band a` line above.
- Undo (e.g. weak signal / drops in far rooms; 5 GHz has shorter range, no fallback to 2.4): `nmcli connection modify DIEMILTONHAUS 802-11-wireless.band ""`
- Router-side alternative that survives anything: separate 5 GHz-only SSID via the T-Mobile app (gateway has no web UI).

## Notes
- Yesterday's (2026-10-11) tuning was boot-time CPU load, unrelated to network speed. See macbook-fedora-update-and-tuning.
- Eric's phone is often on cellular, so "phone is faster" doesn't mean the home connection is faster.
