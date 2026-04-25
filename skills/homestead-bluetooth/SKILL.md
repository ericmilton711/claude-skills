# Homestead Bluetooth — Pi Remote Control via BT Serial

Bluetooth RFCOMM serial service on the Homestead Pi (192.168.12.114) for WiFi-free status checks and LED control. Works from Eric's Fedora laptop and Samsung phone.

> **Last verified working:** 2026-04-25 — all commands (`status`, `log`, `ledtest`, `ledsoff`, `wateron`, `wateroff`) confirmed working over Bluetooth from Fedora laptop. No SSH involved — pure Bluetooth RFCOMM.

## Architecture

- **Pi service** (`bt-homestead.service`): systemd unit that fixes the BD address, enables discoverable mode, and runs a Python RFCOMM server on channel 1
- **Laptop client** (`~/homestead-bt.py`): Python script using raw AF_BLUETOOTH sockets — no pybluez needed on Fedora
- **Phone client**: "Serial Bluetooth Terminal" app by Kai Morich on Android — pair and connect to channel 1

## Known Issue: BCM43455 Default BD Address

The Pi 3 A+'s BCM43455 Bluetooth chip ships without a unique BD address burned in. It uses a dummy address `43:45:c0:00:1f:ac` which most scanners ignore. The fix is a vendor-specific HCI command sent before each adapter startup:

```bash
hcitool cmd 0x3F 0x001 0xE9 0x24 0xF6 0xEB 0x27 0xB8
```

This programs the address `B8:27:EB:F6:24:E9`. It must be run every time the adapter restarts (not persistent across reboots). The systemd service handles this via ExecStartPre.

## Pi Side — Service File

**Location:** `/etc/systemd/system/bt-homestead.service`

```ini
[Unit]
Description=Homestead Bluetooth Command Service
After=bluetooth.target
Requires=bluetooth.target

[Service]
Type=simple
ExecStartPre=/usr/bin/hciconfig hci0 up
ExecStartPre=/usr/bin/hcitool cmd 0x3F 0x001 0xE9 0x24 0xF6 0xEB 0x27 0xB8
ExecStartPre=/usr/bin/hciconfig hci0 down
ExecStartPre=/usr/bin/hciconfig hci0 up
ExecStartPre=/usr/bin/hciconfig hci0 piscan
ExecStartPre=/usr/bin/bluetoothctl discoverable on
ExecStartPre=/usr/bin/bluetoothctl pairable on
ExecStart=/usr/bin/python3 -u /home/eric/bt-homestead.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Note: The first `hciconfig hci0 up` is needed so `hcitool cmd` can reach the adapter. Then down/up cycle applies the new BD address.

## Pi Side — Server Script

**Location:** `/home/eric/bt-homestead.py`

```python
#!/usr/bin/env python3
import bluetooth
import subprocess
import os

CHANNEL = 1

LOG_FILE = '/home/eric/homestead.log'

def get_current_states():
    led_state = 'OFF'
    water_state = 'OFF'
    if os.path.exists(LOG_FILE):
        with open(LOG_FILE) as f:
            for line in f:
                if 'LEDs ON' in line and 'TEST' not in line:
                    led_state = 'ON'
                elif 'LEDs OFF' in line:
                    led_state = 'OFF'
                elif 'Irrigation ON' in line:
                    water_state = 'ON'
                elif 'Irrigation OFF' in line:
                    water_state = 'OFF'
    return led_state, water_state

def get_status():
    lines = []
    lines.append('=== HOMESTEAD PI STATUS ===')
    led_state, water_state = get_current_states()
    lines.append(f'LEDs: {led_state}')
    lines.append(f'Irrigation: {water_state}')
    lines.append('')
    r = subprocess.run(['uptime', '-p'], capture_output=True, text=True)
    lines.append(f'Uptime: {r.stdout.strip()}')
    try:
        with open('/sys/class/thermal/thermal_zone0/temp') as f:
            temp = int(f.read().strip()) / 1000
        lines.append(f'CPU Temp: {temp:.1f}C')
    except:
        lines.append('CPU Temp: unknown')
    r = subprocess.run(['free', '-h'], capture_output=True, text=True)
    for l in r.stdout.strip().split('\n'):
        if 'Mem:' in l:
            lines.append(f'Memory: {l.strip()}')
    r = subprocess.run(['df', '-h', '/'], capture_output=True, text=True)
    for l in r.stdout.strip().split('\n'):
        if '/' in l and 'Filesystem' not in l:
            lines.append(f'Disk: {l.strip()}')
    if os.path.exists(LOG_FILE):
        with open(LOG_FILE) as f:
            log = f.read().strip()
        lines.append('--- LOG ---')
        lines.append(log)
    else:
        lines.append('No homestead.log found')
    lines.append('===========================')
    return '\n'.join(lines)

def get_log():
    if os.path.exists(LOG_FILE):
        with open(LOG_FILE) as f:
            return f.read().strip()
    return 'No homestead.log found'

def led_test():
    try:
        r = subprocess.run(["sudo", "python3", "-c",
            "import RPi.GPIO as GPIO,time;GPIO.setmode(GPIO.BCM);GPIO.setwarnings(False);GPIO.setup(17,GPIO.OUT);[(GPIO.output(17,True),time.sleep(0.125),GPIO.output(17,False),time.sleep(0.25)) for _ in range(27)];GPIO.cleanup()"],
            capture_output=True, text=True, timeout=15)
        return "LED TEST: 10s blink complete"
    except subprocess.TimeoutExpired:
        return "LED TEST: timed out"
    except Exception as e:
        return f"LED TEST ERROR: {e}"

def leds_off():
    try:
        subprocess.run(['sudo', 'python3', '-c',
            'import RPi.GPIO as GPIO; GPIO.setmode(GPIO.BCM); GPIO.setup(17,GPIO.OUT); GPIO.output(17,GPIO.LOW); GPIO.cleanup()'],
            capture_output=True, text=True, timeout=10)
        return 'LEDs OFF: done'
    except Exception as e:
        return f'LEDs OFF ERROR: {e}'

def water_on():
    try:
        subprocess.run(['sudo', 'python3', '/home/eric/homestead.py', 'irrigate-on'],
            capture_output=True, text=True, timeout=10)
        return 'WATER ON: solenoid opened'
    except Exception as e:
        return f'WATER ON ERROR: {e}'

def water_off():
    try:
        subprocess.run(['sudo', 'python3', '/home/eric/homestead.py', 'irrigate-off'],
            capture_output=True, text=True, timeout=10)
        return 'WATER OFF: solenoid closed'
    except Exception as e:
        return f'WATER OFF ERROR: {e}'

def handle_command(cmd):
    cmd = cmd.strip().lower()
    if cmd == 'status':
        return get_status()
    elif cmd == 'log':
        return get_log()
    elif cmd == 'ledtest':
        return led_test()
    elif cmd == 'ledsoff':
        return leds_off()
    elif cmd == 'wateron':
        return water_on()
    elif cmd == 'wateroff':
        return water_off()
    elif cmd == 'help':
        return 'Commands: status, log, ledtest, ledsoff, wateron, wateroff, help'
    else:
        return f'Unknown command: {cmd}\nType help for available commands'

def main():
    server = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    server.bind(('', CHANNEL))
    server.listen(1)
    print(f'Homestead BT service listening on RFCOMM channel {CHANNEL}')

    while True:
        try:
            print('Waiting for connection...')
            client, addr = server.accept()
            print(f'Connected: {addr}')
            client.send(b'Homestead Pi - type "help" for commands\n> ')

            buf = b''
            while True:
                try:
                    data = client.recv(1024)
                    if not data:
                        break
                    buf += data
                    while b'\n' in buf or b'\r' in buf:
                        sep = b'\n' if b'\n' in buf else b'\r'
                        cmd_bytes, buf = buf.split(sep, 1)
                        buf = buf.lstrip(b'\r\n')
                        cmd = cmd_bytes.decode('utf-8', errors='replace').strip()
                        if not cmd:
                            client.send(b'> ')
                            continue
                        if cmd in ('quit', 'exit'):
                            client.send(b'Goodbye!\n')
                            raise StopIteration
                        result = handle_command(cmd)
                        client.send(f'{result}\n> '.encode())
                except StopIteration:
                    break
                except Exception as e:
                    print(f'Error: {e}')
                    break
            client.close()
            print('Client disconnected')
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f'Connection error: {e}')

    server.close()

if __name__ == '__main__':
    main()
```

## Pi Side — Auto-Accept Pairing Agent

**Location:** `/home/eric/bt-agent.py` (also needs a systemd service for persistence)

```python
#!/usr/bin/env python3
import dbus, dbus.service, dbus.mainloop.glib
from gi.repository import GLib

AGENT_PATH = '/test/agent'

class Agent(dbus.service.Object):
    @dbus.service.method('org.bluez.Agent1', in_signature='', out_signature='')
    def Release(self): pass

    @dbus.service.method('org.bluez.Agent1', in_signature='os', out_signature='')
    def AuthorizeService(self, device, uuid): pass

    @dbus.service.method('org.bluez.Agent1', in_signature='o', out_signature='s')
    def RequestPinCode(self, device): return '0000'

    @dbus.service.method('org.bluez.Agent1', in_signature='ou', out_signature='')
    def RequestConfirmation(self, device, passkey): pass

    @dbus.service.method('org.bluez.Agent1', in_signature='o', out_signature='')
    def RequestAuthorization(self, device): pass

    @dbus.service.method('org.bluez.Agent1', in_signature='o', out_signature='u')
    def RequestPasskey(self, device): return dbus.UInt32(0)

    @dbus.service.method('org.bluez.Agent1', in_signature='ouq', out_signature='')
    def DisplayPasskey(self, device, passkey, entered): pass

    @dbus.service.method('org.bluez.Agent1', in_signature='os', out_signature='')
    def DisplayPinCode(self, device, pincode): pass

    @dbus.service.method('org.bluez.Agent1', in_signature='', out_signature='')
    def Cancel(self): pass

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
bus = dbus.SystemBus()
agent = Agent(bus, AGENT_PATH)
manager = dbus.Interface(bus.get_object('org.bluez', '/org/bluez'), 'org.bluez.AgentManager1')
manager.RegisterAgent(AGENT_PATH, 'NoInputNoOutput')
manager.RequestDefaultAgent(AGENT_PATH)
print('Agent running...')
GLib.MainLoop().run()
```

## Laptop Side — Client Script

**Location:** `~/homestead-bt.py` (on Fedora laptop)

```python
#!/usr/bin/env python3
"""Connect to Homestead Pi over Bluetooth.
Usage: sudo python3 homestead-bt.py [command]
  No args = interactive mode
  With arg = run single command (status, ledtest, ledsoff, help)
"""
import socket, sys, time

PI_ADDR = 'B8:27:EB:F6:24:E9'
CHANNEL = 1
PROMPT = b'> '

def connect():
    sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    sock.settimeout(15)
    sock.connect((PI_ADDR, CHANNEL))
    banner = read_until_prompt(sock)
    return sock, banner

def read_until_prompt(sock, timeout=30):
    buf = b''
    deadline = time.time() + timeout
    while time.time() < deadline:
        remaining = deadline - time.time()
        sock.settimeout(min(remaining, 5))
        try:
            chunk = sock.recv(4096)
            if not chunk:
                break
            buf += chunk
            if buf.endswith(PROMPT):
                return buf[:-len(PROMPT)].decode(errors='replace').strip()
        except socket.timeout:
            continue
    return buf.decode(errors='replace').strip()

def send_cmd(sock, cmd):
    sock.send(f'{cmd}\n'.encode())
    return read_until_prompt(sock)

def main():
    print('Connecting to Homestead Pi via Bluetooth...')
    try:
        sock, banner = connect()
    except Exception as e:
        print(f'Connection failed: {e}')
        print('Make sure Bluetooth is on and Pi is in range.')
        sys.exit(1)

    if len(sys.argv) > 1:
        cmd = ' '.join(sys.argv[1:])
        print(send_cmd(sock, cmd))
        sock.close()
        return

    print(banner)
    while True:
        try:
            cmd = input('> ').strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break
        if not cmd:
            continue
        if cmd in ('quit', 'exit'):
            break
        print(send_cmd(sock, cmd))
    sock.close()
    print('Disconnected.')

if __name__ == '__main__':
    main()
```

## Pi Dependencies

```bash
sudo apt-get install -y python3-bluez libbluetooth-dev python3-dbus python3-gi
```

## Laptop Dependencies (Fedora)

```bash
sudo dnf install -y bluez bluez-deprecated python3-pyserial screen
```

Note: `pybluez` does NOT build on Fedora's Python 3.14. The laptop client uses raw `AF_BLUETOOTH` sockets from Python's stdlib instead.

## Bluetooth Config on Pi

**`/etc/bluetooth/main.conf`** — added under `[General]`:
```ini
DiscoverableTimeout = 0
Class = 0x000100
Name = homestead
```

## Connection Procedure

### From Laptop
1. Enable Bluetooth: `sudo rfkill unblock bluetooth && bluetoothctl power on`
2. Scan: `bluetoothctl --timeout 12 scan on`
3. Trust: `bluetoothctl trust B8:27:EB:F6:24:E9`
4. Connect: `sudo python3 ~/homestead-bt.py status`

Pairing is NOT required for RFCOMM — the Pi's NoInputNoOutput agent auto-accepts. Just scan, trust, and connect.

### From Samsung Phone
1. Install "Serial Bluetooth Terminal" by Kai Morich from Play Store
2. Pair with "homestead" in Android Bluetooth settings
3. Open app, connect to "homestead"
4. Type commands: `status`, `log`, `ledtest`, `ledsoff`, `wateron`, `wateroff`, `help`

## Available Commands

| Command | Description |
|---------|-------------|
| `status` | Full Pi status: LED/irrigation state, uptime, CPU temp, memory, disk, homestead.log |
| `log` | Just the homestead.log contents |
| `ledtest` | Run the LED blink test on GPIO 17 |
| `ledsoff` | Turn off LEDs on GPIO 17 |
| `wateron` | Open solenoid valve (GPIO 27) — starts watering |
| `wateroff` | Close solenoid valve (GPIO 27) — stops watering |
| `help` | List commands |

## Bluetooth Details

- **Pi BD Address:** `B8:27:EB:F6:24:E9` (programmed via HCI vendor command)
- **Chip:** BCM43455 (BCM4345C0)
- **Protocol:** Bluetooth 4.1 Classic (BR/EDR) + BLE
- **Range:** ~10-30 meters
- **RFCOMM Channel:** 1
