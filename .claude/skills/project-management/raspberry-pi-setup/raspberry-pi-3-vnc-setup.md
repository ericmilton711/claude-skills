# Raspberry Pi 3 A+ VNC Setup Guide

**Date**: January 6, 2026
**Hardware**: Raspberry Pi 3 Model A+
**Connection**: USB OTG Ethernet (Micro USB to Laptop)
**Windows Laptop**: 192.168.1.9

## Overview
This guide documents the setup process for connecting to a Raspberry Pi 3 A+ from a Windows laptop and installing VNC for remote desktop access.

## Hardware Setup

### Raspberry Pi 3 A+ Specifications
- Model: Raspberry Pi 3 A+
- Connection Method: USB OTG via Micro USB power port
- Target Connection: Direct USB to Windows laptop

### Physical Connection
The Raspberry Pi 3 A+ is connected to the Windows laptop via USB cable:
- **Pi Side**: Micro USB power port (supports USB OTG ethernet gadget mode)
- **Laptop Side**: USB-A port
- **Network Adapter Detected**: Realtek USB GbE Family Controller #2 (Ethernet 6)

## Software Installation

### 1. Raspberry Pi Imager Installation (Windows)

**Download & Install:**
```powershell
# Downloaded from: https://downloads.raspberrypi.org/imager/imager_latest.exe
# Installed to: C:\Program Files\Raspberry Pi Ltd\Imager\rpi-imager.exe
```

**Installation Date**: January 6, 2026
**Version**: Latest (as of Jan 2026)
**Install Location**: `C:\Program Files\Raspberry Pi Ltd\Imager\`

### 2. Flashing Raspberry Pi OS

**Steps:**
1. Insert Micro SD card into Windows laptop (via USB SD card reader)
2. Launch Raspberry Pi Imager
3. Configure settings:
   - **Device**: Raspberry Pi 3
   - **OS**: Raspberry Pi OS (32-bit) - Recommended
   - **Storage**: Select your SD card

4. **Important Configuration** (Click gear icon):
   - Enable SSH (password authentication)
   - Set username/password
   - Configure WiFi:
     - SSID: [Your WiFi Network]
     - Password: [Your WiFi Password]
     - Country: US
   - Set hostname: raspberrypi (or custom name)

5. Click **SAVE** then **WRITE** to flash

**Estimated Time**: 5-10 minutes

## Network Configuration

### Option 1: WiFi Connection (Recommended for Initial Setup)
- IP Range: 192.168.1.x (DHCP assigned)
- Router: 192.168.1.1
- Can find Pi IP from router DHCP table after boot

### Option 2: USB OTG Ethernet
For USB ethernet to work, the Pi needs additional configuration:

**After flashing, before first boot:**
1. Edit `config.txt` on boot partition:
   ```
   dtoverlay=dwc2
   ```

2. Edit `cmdline.txt` (add after `rootwait`):
   ```
   modules-load=dwc2,g_ether
   ```

3. This enables USB gadget mode ethernet

**Windows Network Adapter:**
- Interface: Ethernet 6
- Adapter: Realtek USB GbE Family Controller #2
- Auto-IP: 169.254.x.x (link-local addressing)

## Next Steps

### 1. First Boot
- Insert flashed SD card into Raspberry Pi 3 A+
- Power on via USB connection or separate power supply
- Wait 1-2 minutes for first boot initialization

### 2. Connect via SSH
```bash
# Via WiFi
ssh pi@raspberrypi.local
# or
ssh pi@<IP-address>

# Via USB ethernet (if configured)
ssh pi@raspberrypi.local
```

### 3. Install VNC Server on Raspberry Pi
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install RealVNC Server (usually pre-installed on Raspberry Pi OS)
sudo apt install realvnc-vnc-server -y

# Enable VNC
sudo raspi-config
# Navigate to: Interface Options > VNC > Enable

# Or enable via command line
sudo systemctl enable vncserver-x11-serviced
sudo systemctl start vncserver-x11-serviced
```

### 4. Install VNC Viewer on Windows
```powershell
# Download VNC Viewer from:
# https://www.realvnc.com/en/connect/download/viewer/windows/

# Or install via winget:
winget install RealVNC.VNCViewer
```

### 5. Connect via VNC
1. Open VNC Viewer on Windows
2. Enter Pi address: `raspberrypi.local` or `<IP-address>`
3. Authenticate with Pi username/password
4. Remote desktop connection established!

## Troubleshooting

### Pi Not Detected on Network
- Check WiFi credentials in Raspberry Pi Imager settings
- Verify router DHCP table for new device
- Try direct ethernet connection to router instead of USB

### SSH Connection Refused
- Ensure SSH was enabled during imaging
- Wait longer for first boot (can take 2-3 minutes)
- Check Pi has power LED on and activity LED blinking

### VNC Connection Issues
- Verify VNC server is running: `sudo systemctl status vncserver-x11-serviced`
- Check firewall: `sudo ufw allow 5900`
- Ensure desktop environment is installed (required for VNC)

### USB OTG Not Working
- Verify `config.txt` and `cmdline.txt` modifications
- Check Windows Device Manager for RNDIS or USB ethernet device
- Try different USB cable or port

## Network Topology

```
[Windows Laptop]
    IP: 192.168.1.9
    |
    |-- WiFi --> [Router 192.168.1.1]
    |                   |
    |                   |-- DHCP --> [Raspberry Pi WiFi]
    |
    |-- USB Cable --> [Raspberry Pi USB OTG]
                      (Optional, requires config)
```

## References
- [Raspberry Pi Documentation](https://www.raspberrypi.com/documentation/)
- [RealVNC Documentation](https://help.realvnc.com/hc/en-us)
- [USB OTG Configuration](https://www.raspberrypi.com/documentation/computers/configuration.html#setting-up-a-headless-raspberry-pi)

## Status
- Raspberry Pi Imager installed
- Waiting for SD card flash completion
- VNC Server installation pending
- VNC Viewer installation pending
- Connection testing pending

---
**Last Updated**: January 6, 2026
**Next Session**: Complete SD card flashing and boot Raspberry Pi
