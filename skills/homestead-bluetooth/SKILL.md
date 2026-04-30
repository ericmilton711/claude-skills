# Homestead Bluetooth — Pi Remote Control via BLE GATT

Bluetooth Low Energy GATT service on the Homestead Pi (192.168.12.114) for WiFi-free status checks, LED control, and irrigation control. Works cross-platform (Linux, Windows, macOS) via the `bleak` Python library.

> **Last verified working:** 2026-04-27 — BLE GATT tested working from Fedora laptop with `bleak`, including reconnects. All commands confirmed. Classic RFCOMM disabled in favor of BLE.

## Architecture

- **Pi BLE GATT server** (`ble-homestead.service`): systemd unit running a Python dbus-based GATT server with write (command) and read/notify (response) characteristics
- **Client library**: `bleak` (pip3 install bleak) — cross-platform BLE, works on Linux/Windows/macOS
- **Pi-Tools app** (`~/Pi-Tools/pi-tools-gui.py`): Tkinter GUI that tries SSH first, falls back to BLE GATT

## Critical Pi BlueZ Configuration

**ALL of these are required for BLE to work reliably. Without them, client BlueZ stacks try BR/EDR and connections fail.**

### 1. LE-Only Controller Mode

**`/etc/bluetooth/main.conf`** — under `[General]`:
```ini
DiscoverableTimeout = 0
Class = 0x000100
Name = homestead
ControllerMode = le
```

### 2. Disable Interfering Plugins

**`/etc/systemd/system/bluetooth.service.d/no-midi.conf`:**
```ini
[Service]
ExecStart=
ExecStart=/usr/libexec/bluetooth/bluetoothd --noplugin=midi,avrcp,sap,a2dp
```

Without `--noplugin=midi`, the MIDI Bluetooth profile disrupts GATT connections with "Failed to read initial request" errors.

### 3. Classic BT Service Disabled

```bash
sudo systemctl stop bt-homestead.service
sudo systemctl disable bt-homestead.service
```

Classic RFCOMM profiles cause BlueZ on clients to prefer BR/EDR transport over BLE.

### 4. After Changes

```bash
sudo systemctl daemon-reload
sudo systemctl restart bluetooth
sudo systemctl restart ble-homestead.service
```

## BLE GATT UUIDs

| UUID | Purpose |
|------|---------|
| `12345678-1234-5678-1234-56789abcdef0` | Service UUID |
| `12345678-1234-5678-1234-56789abcdef1` | Command characteristic (write) |
| `12345678-1234-5678-1234-56789abcdef2` | Response characteristic (read/notify) |

## Pi Side — BLE Service File

**Location:** `/etc/systemd/system/ble-homestead.service`

```ini
[Unit]
Description=Homestead BLE GATT Command Service
After=bluetooth.target
Wants=bluetooth.target

[Service]
Type=simple
ExecStart=/usr/bin/python3 -u /home/eric/ble-homestead.py
Restart=always
RestartSec=5
User=root

[Install]
WantedBy=multi-user.target
```

## Pi Side — BLE GATT Server Script

**Location:** `/home/eric/ble-homestead.py`

Key components:
- `Advertisement` class — BLE advertising as "homestead" with service UUID
- `Service` / `CmdCharacteristic` / `RespCharacteristic` — GATT service tree
- `NoInputNoOutputAgent` — auto-accepts BLE connections without pairing prompts
- `run_command(cmd)` — executes homestead commands and returns response string
- Notification-based response delivery with chunked MTU support

The server registers:
1. A GATT application with BlueZ
2. A BLE advertisement
3. A NoInputNoOutput pairing agent (critical — without this, connections fail with "No agent available")

```python
#!/usr/bin/env python3
"""BLE GATT server for Homestead Pi — wraps the same commands as bt-homestead.py."""

import dbus
import dbus.exceptions
import dbus.mainloop.glib
import dbus.service
import subprocess
import os
import array
from gi.repository import GLib

BLUEZ_SERVICE = "org.bluez"
GATT_MANAGER_IFACE = "org.bluez.GattManager1"
LE_ADVERTISING_MANAGER_IFACE = "org.bluez.LEAdvertisingManager1"
DBUS_OM_IFACE = "org.freedesktop.DBus.ObjectManager"
DBUS_PROP_IFACE = "org.freedesktop.DBus.Properties"
GATT_SERVICE_IFACE = "org.bluez.GattService1"
GATT_CHRC_IFACE = "org.bluez.GattCharacteristic1"
LE_ADVERTISEMENT_IFACE = "org.bluez.LEAdvertisement1"
AGENT_IFACE = "org.bluez.Agent1"
AGENT_MANAGER_IFACE = "org.bluez.AgentManager1"

SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
CMD_CHAR_UUID = "12345678-1234-5678-1234-56789abcdef1"
RESP_CHAR_UUID = "12345678-1234-5678-1234-56789abcdef2"

HOMESTEAD_SCRIPT = "/home/eric/homestead.py"
LOG_FILE = "/home/eric/homestead.log"
```

## Client Side — bleak (Cross-Platform)

**Install:** `pip3 install bleak` (all platforms)

**Minimal client example:**
```python
import asyncio
import bleak

BLE_SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
BLE_CMD_UUID     = "12345678-1234-5678-1234-56789abcdef1"
BLE_RESP_UUID    = "12345678-1234-5678-1234-56789abcdef2"

async def send_cmd(cmd):
    device = await bleak.BleakScanner.find_device_by_filter(
        lambda d, ad: BLE_SERVICE_UUID.lower() in [
            u.lower() for u in (ad.service_uuids or [])],
        timeout=10,
    )
    if not device:
        raise ConnectionError("Pi BLE not found")

    chunks = []
    done = asyncio.Event()
    def on_notify(sender, data):
        chunks.append(data.decode(errors="replace"))
        done.set()

    async with bleak.BleakClient(device, timeout=20) as client:
        await client.start_notify(BLE_RESP_UUID, on_notify)
        await client.write_gatt_char(BLE_CMD_UUID, cmd.encode(), response=True)
        try:
            await asyncio.wait_for(done.wait(), timeout=30)
            await asyncio.sleep(1)
        except asyncio.TimeoutError:
            pass
        await client.stop_notify(BLE_RESP_UUID)
    return "".join(chunks)

result = asyncio.run(send_cmd("status"))
print(result)
```

## Pi-Tools App BLE Integration

The Pi-Tools GUI (`~/Pi-Tools/pi-tools-gui.py`) has BLE built in:
- `ble_available()` — checks if bleak is importable
- `ble_send_cmd(cmd)` — scans for Pi, connects, sends command, reads response
- `_run_in_output(title, command, bt_cmd=None)` — tries SSH first, falls back to BLE
- Each button passes `bt_cmd` parameter: status, log, ledtest, wateron, wateroff, shutdown

## Available Commands

| Command | Description |
|---------|-------------|
| `status` | Full Pi status: LED/irrigation state, uptime, CPU temp, memory, disk, homestead.log |
| `log` | Just the homestead.log contents |
| `ledtest` | Run the LED blink test on GPIO 17 |
| `ledsoff` | Turn off LEDs on GPIO 17 |
| `wateron` | Open solenoid valve (GPIO 27) — starts watering |
| `wateroff` | Close solenoid valve (GPIO 27) — stops watering |
| `shutdown` | Safe shutdown — turns off LEDs and solenoid, then powers off |
| `help` | List commands |

## Bluetooth Details

- **Adapter:** Edimax BT-8500 USB dongle (RTL8761BU, BT 5.0) — replaced built-in BCM43438 on 2026-04-30 for better range
- **Pi BD Address:** `08:BE:AC:4D:39:71` (Edimax hardware MAC)
- **Old BD Address:** `B8:27:EB:F6:24:E9` (built-in BCM43438, now disabled via `dtoverlay=disable-bt` in config.txt)
- **Protocol:** BLE only (ControllerMode = le)
- **Range:** ~50-100 meters (USB dongle with better antenna vs built-in chip)
- **Advertising name:** `homestead`
- **ESP32 connects by name**, not MAC — no ESP32 code change needed

## Troubleshooting

### "failed to discover services, device disconnected" (bleak on Linux)
- **Cause:** Pi running in dual-mode (classic + BLE), or MIDI plugin interfering, or no pairing agent
- **Fix:** Ensure ALL four config items above are applied, then restart bluetooth + ble-homestead

### "BREDR.ProfileUnavailable" error
- **Cause:** Client BlueZ has Pi cached as classic BR/EDR device
- **Fix:** `bluetoothctl remove 08:BE:AC:4D:39:71`, wait 5 seconds, then retry

### BLE command works once but fails on reconnect
- **Cause:** BlueZ caches the device after first connection
- **Fix:** This was fixed by ControllerMode=le + disabled plugins. If it recurs, remove cached device between connections

### "No agent available for request type 2" (Pi logs)
- **Cause:** NoInputNoOutput agent not registered
- **Fix:** Ensure ble-homestead.py includes the NoInputNoOutputAgent class and registers it in main()

## Pi Dependencies

```bash
sudo apt-get install -y python3-dbus python3-gi
```

## Client Dependencies

```bash
pip3 install bleak    # Linux, Windows, macOS
```

## Legacy: Classic RFCOMM (disabled)

The classic Bluetooth RFCOMM service (`bt-homestead.service`, `/home/eric/bt-homestead.py`) still exists on the Pi but is disabled. It only worked from Linux (using `AF_BLUETOOTH` sockets). If needed for phone access via "Serial Bluetooth Terminal" app, it can be re-enabled — but ControllerMode must be changed back to `dual` and the MIDI plugin issue will return for BLE clients.

The laptop RFCOMM client is at `~/homestead-bt.py` on the Fedora laptop.

## Known Issue: BCM43438 Default BD Address

The Pi 3 A+'s BCM43438 Bluetooth chip ships without a unique BD address. It uses a dummy address which most scanners ignore. The fix is a vendor-specific HCI command sent before each adapter startup:

```bash
hcitool cmd 0x3F 0x001 0xE9 0x24 0xF6 0xEB 0x27 0xB8
```

This programs the address `B8:27:EB:F6:24:E9`. The classic bt-homestead.service handled this in ExecStartPre. Since we're now using BLE-only, this address should already be set from initial bluetooth service startup.
