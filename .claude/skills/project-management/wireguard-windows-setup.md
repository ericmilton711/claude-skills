# WireGuard VPN Setup - Windows PC

## Overview
WireGuard VPN configured on Windows PC to access remote networks (192.168.0.x and 192.168.2.x) through the Lambert VPN server.

**Date Configured:** January 3, 2026
**Computer:** Windows PC (192.168.1.9)
**VPN Name:** Lambert
**VPN IP:** 192.168.2.2/32

---

## Configuration Details

### Network Access
- **Local Network:** 192.168.1.x (MILTONHAUS2)
- **VPN Networks:**
  - 192.168.0.0/24 (Remote network 1)
  - 192.168.2.0/24 (VPN network)
- **VPN Server:** edenredux.servegame.com:51820

### DNS Configuration
- **DNS Server:** 192.168.1.7 (Pi-hole on Mac)
- All DNS queries route through Pi-hole for ad blocking

---

## Installation Steps

### 1. Install WireGuard

Downloaded and installed WireGuard for Windows:
```powershell
Invoke-WebRequest -Uri 'https://download.wireguard.com/windows-client/wireguard-installer.exe' -OutFile 'C:\Users\ericm\Downloads\wireguard-installer.exe'
Start-Process 'C:\Users\ericm\Downloads\wireguard-installer.exe' -ArgumentList '/S' -Wait
```

**Installation Location:** `C:\Program Files\WireGuard\`

### 2. Create Configuration File

Created Lambert.conf at: `C:\Users\ericm\Downloads\Lambert.conf`

```ini
[Interface]
Address = 192.168.2.2/32
DNS = 192.168.1.7
PrivateKey = aGPFmtpCh7zMBhwlRh+rpHHBIweHfIv5Grov80vwSWc=

[Peer]
AllowedIPs = 192.168.0.0/24, 192.168.2.0/24
Endpoint = edenredux.servegame.com:51820
PublicKey = uEh1J4jgbAcqp6XYM9dZMxyFrxezBUZbAwNtX539zhc=
```

**Configuration Details:**
- **Interface Address:** 192.168.2.2/32 (VPN IP assigned to this computer)
- **DNS:** 192.168.1.7 (Routes DNS through Pi-hole)
- **Allowed IPs:** Traffic to 192.168.0.x and 192.168.2.x goes through VPN
- **Endpoint:** VPN server at edenredux.servegame.com port 51820

### 3. Install and Activate Tunnel

Installed the tunnel as a Windows service:
```powershell
& 'C:\Program Files\WireGuard\wireguard.exe' /installtunnelservice 'C:\Users\ericm\Downloads\Lambert.conf'
```

Service name: `WireGuardTunnel$Lambert`

---

## Verification

### Check VPN Connection Status

**Check if tunnel is running:**
```cmd
sc query WireGuardTunnel$Lambert
```

**Check VPN IP assignment:**
```cmd
ipconfig | findstr "192.168.2"
```
Should show: `IPv4 Address. . . . . . . . . . . : 192.168.2.2`

**Test connectivity to remote network:**
```cmd
ping 192.168.0.100
```
Should receive replies if VPN is working.

### Connection Test Results (2026-01-03)
```
Pinging 192.168.0.100 with 32 bytes of data:
Reply from 192.168.0.100: bytes=32 time=57ms TTL=63
Reply from 192.168.0.100: bytes=32 time=72ms TTL=63

Ping statistics for 192.168.0.100:
    Packets: Sent = 2, Received = 2, Lost = 0 (0% loss),
    Approximate round trip times in milli-seconds:
    Minimum = 57ms, Maximum = 72ms, Average = 64ms
```

✅ **Connection working perfectly!**

---

## Managing WireGuard

### Start/Stop Tunnel via Command Line

**Start tunnel:**
```cmd
net start WireGuardTunnel$Lambert
```

**Stop tunnel:**
```cmd
net stop WireGuardTunnel$Lambert
```

**Check status:**
```cmd
sc query WireGuardTunnel$Lambert
```

### Using WireGuard GUI

**Launch GUI:**
```cmd
"C:\Program Files\WireGuard\wireguard.exe"
```

From the GUI you can:
- View tunnel status
- Activate/deactivate tunnels
- View connection statistics
- Import/export configurations
- View logs

---

## Accessing Remote Resources

### Example: Web Service on 192.168.0.100

With VPN connected, you can access services on the remote network:

**Web Interface:**
```
http://192.168.0.100:5006
```

**Other services:** Any service on 192.168.0.x or 192.168.2.x networks

---

## Troubleshooting

### VPN Not Connecting

1. **Check service status:**
   ```cmd
   sc query WireGuardTunnel$Lambert
   ```

2. **Restart service:**
   ```cmd
   net stop WireGuardTunnel$Lambert
   net start WireGuardTunnel$Lambert
   ```

3. **Check endpoint is reachable:**
   ```cmd
   ping edenredux.servegame.com
   ```

4. **View WireGuard logs:**
   ```cmd
   "C:\Program Files\WireGuard\wireguard.exe" /dumplog
   ```

### Can't Access Remote Network

1. **Verify VPN IP is assigned:**
   ```cmd
   ipconfig
   ```
   Should show 192.168.2.2 on WireGuard adapter

2. **Check routing:**
   ```cmd
   route print | findstr "192.168.0"
   route print | findstr "192.168.2"
   ```

3. **Test ping to VPN gateway:**
   ```cmd
   ping 192.168.2.1
   ```

4. **Test ping to target network:**
   ```cmd
   ping 192.168.0.1
   ping 192.168.0.100
   ```

### DNS Not Working Through VPN

1. **Check DNS configuration:**
   ```cmd
   ipconfig /all | findstr "DNS"
   ```

2. **Test DNS resolution:**
   ```cmd
   nslookup google.com 192.168.1.7
   ```

3. **Flush DNS cache:**
   ```cmd
   ipconfig /flushdns
   ```

---

## Auto-Start on Boot

The WireGuard tunnel is configured as a Windows service and will automatically start on boot.

**To verify auto-start:**
```cmd
sc qc WireGuardTunnel$Lambert
```

Look for `START_TYPE: 2 AUTO_START`

**To enable auto-start if disabled:**
```cmd
sc config WireGuardTunnel$Lambert start= auto
```

---

## Security Considerations

### Private Key Security
- Private key is stored in: `C:\Users\ericm\Downloads\Lambert.conf`
- **Important:** Keep this file secure - anyone with access to it can impersonate this VPN client
- Consider moving to a more secure location after setup

### Network Security
- VPN provides encrypted tunnel to remote networks
- All traffic to 192.168.0.x and 192.168.2.x is routed through VPN
- DNS queries still go through local Pi-hole (192.168.1.7) for ad blocking

### Firewall
- WireGuard creates a network adapter that Windows Firewall monitors
- Ensure firewall rules allow necessary traffic through WireGuard adapter

---

## Configuration Files Location

- **WireGuard Installation:** `C:\Program Files\WireGuard\`
- **Configuration File:** `C:\Users\ericm\Downloads\Lambert.conf`
- **Setup Script:** `C:\Users\ericm\pihole\setup-wireguard.ps1`

---

## Related Documentation

- **Pi-hole Setup:** See `pihole-setup.md` in this skill
- **Mac WireGuard Config:** `/etc/wireguard/Lambert.conf` on Mac
- **Network Documentation:** See `~/docs/PROJECT.md` on Mac

---

## Summary

✅ **WireGuard VPN successfully configured on Windows PC**
- VPN IP: 192.168.2.2
- Access to 192.168.0.x network: Working
- Access to 192.168.2.x network: Working
- DNS through Pi-hole: Working
- Auto-start on boot: Enabled
- Tested and verified: 2026-01-03

**Use Case:** Access services on remote 192.168.0.x network (e.g., http://192.168.0.100:5006)

---

**Configured:** January 3, 2026
**Configured By:** Claude & Eric
**Status:** ✅ Active and Working
