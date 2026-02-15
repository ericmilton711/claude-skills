---
name: credentials
description: Manages LAN and network credentials securely. Use when working with WiFi passwords, router credentials, network device passwords, SSH keys, or when the user mentions LAN, network, router, WiFi, access point, switch, firewall, or network credentials.
allowed-tools: Read, Grep, Glob, Bash(git:*), Bash(ip:*), Bash(nmcli:*), Bash(ssh:*)
---

# LAN Credentials Management Skill

This skill helps manage network and LAN credentials securely, including router passwords, WiFi credentials, network device access, and related security.

## Core Responsibilities

1. **Manage network credentials** - Store and organize LAN device credentials securely
2. **WiFi password management** - Handle WiFi network credentials and sharing
3. **Network device access** - Manage router, switch, AP, and firewall credentials
4. **SSH key management** - Generate and manage SSH keys for network devices
5. **Secure storage** - Ensure network credentials are encrypted and protected
6. **Credential rotation** - Track and remind about password rotation schedules

## Network Credential Types

### Device Credentials
- **Router/Gateway** - Admin passwords, web interface credentials
- **Switches** - Management interface passwords, console passwords
- **Access Points** - Admin credentials, WiFi passwords (WPA2/WPA3 PSK)
- **Firewalls** - Admin access, VPN credentials
- **NAS/File Servers** - SMB/NFS share passwords, admin access
- **Print Servers** - Admin credentials, network printer passwords
- **IoT Devices** - Smart home device credentials, camera passwords

### Network Service Credentials
- **WiFi Networks** - SSID and WPA/WPA2/WPA3 passwords
- **VPN** - OpenVPN, WireGuard, IPSec credentials and keys
- **SSH Keys** - Private keys for remote network device access
- **SNMP** - Community strings (v1/v2c), SNMPv3 credentials
- **RADIUS/802.1X** - Authentication server credentials
- **Network Shares** - SMB, NFS, AFP credentials

### Cloud Network Services
- **DNS** - CloudFlare, Route53, Google DNS API credentials
- **DDNS** - Dynamic DNS service credentials
- **Remote Management** - TeamViewer, AnyDesk, remote access tools

## Secure Storage Recommendations

### Priority order (most to least secure)

1. **Password Managers** (recommended for personal/home networks)
   - Bitwarden, 1Password, KeePassXC
   - Encrypted database with master password
   - Organized folders per network/location
   - Secure sharing for family/team members

2. **Encrypted files** (for automation/scripts)
   - GPG encrypted text files
   - Age encrypted credentials
   - Ansible Vault for configuration management
   - SSH key encryption with passphrase

3. **System keyring** (Linux/macOS)
   - GNOME Keyring, KDE Wallet
   - macOS Keychain
   - Windows Credential Manager
   - NetworkManager connection profiles (encrypted)

4. **Hardware security** (for sensitive environments)
   - YubiKey for SSH authentication
   - Hardware password manager (OnlyKey)
   - TPM-backed credential storage

### Never acceptable
- Plain text files in home directory
- Committed to version control (public or private repos)
- Sticky notes or physical documents near equipment
- Saved in browser without master password
- Shared via unencrypted email/chat
- Default passwords (admin/admin, password, etc.)

## Standard Workflow

### When setting up new network credentials

1. **Inventory network devices**
   ```bash
   # Scan local network
   ip addr show
   sudo nmap -sn 192.168.1.0/24
   avahi-browse -art
   ```

2. **Organize credential storage**
   - Create password manager vault/folder structure
   - Group by network location (home, office, remote site)
   - Tag by device type (router, AP, switch, etc.)

3. **Generate strong passwords**
   - Use password manager generator (20+ characters)
   - Different passwords for each device
   - Avoid patterns based on device name/location

4. **Store additional device information**
   - IP address (static preferred)
   - MAC address
   - Hostname
   - Model/firmware version
   - Admin panel URL
   - Default credentials (for reference)
   - Last password change date

### When managing WiFi credentials

1. **Primary network**
   - Store SSID and WPA2/WPA3 password
   - Note encryption type
   - Guest network credentials (if separate)

2. **QR code generation** (for easy sharing)
   ```bash
   # Generate WiFi QR code
   qrencode -t PNG -o wifi.png "WIFI:T:WPA;S:MyNetworkName;P:MyPassword;;"
   ```

3. **Secure sharing**
   - Use password manager sharing features
   - Generate temporary guest passwords
   - Time-limited access for visitors

### When managing SSH keys for network devices

1. **Generate device-specific SSH keys**
   ```bash
   ssh-keygen -t ed25519 -C "router-home" -f ~/.ssh/router_home
   ssh-keygen -t ed25519 -C "switch-lab" -f ~/.ssh/switch_lab
   ```

2. **Copy to network devices**
   ```bash
   ssh-copy-id -i ~/.ssh/router_home.pub admin@192.168.1.1
   ```

3. **Configure SSH config**
   ```
   # ~/.ssh/config
   Host router
       HostName 192.168.1.1
       User admin
       IdentityFile ~/.ssh/router_home
       StrictHostKeyChecking yes

   Host switch
       HostName 192.168.1.10
       User admin
       IdentityFile ~/.ssh/switch_lab
   ```

4. **Secure private keys**
   - Always use passphrase encryption
   - Restrict permissions: `chmod 600 ~/.ssh/router_home`
   - Back up encrypted to password manager

## Example Secure Setup

### Password Manager Entry (Bitwarden/KeePassXC format)

```
Name: Home Router (TP-Link AX6000)
Username: admin
Password: [Generated 24-char password]
URL: https://192.168.1.1
Notes:
  IP: 192.168.1.1
  MAC: AA:BB:CC:DD:EE:FF
  Model: AX6000 v1.2
  Firmware: 1.3.1
  Default user/pass: admin/admin (DO NOT USE)
  Last changed: 2026-01-02
  Next rotation: 2026-07-02 (6 months)

Tags: router, home, network
Folder: Home Network/Devices
```

### WiFi Credentials Template

```
Name: Home WiFi - Main Network
Type: WiFi Network
SSID: MyHomeNetwork
Password: [Generated WPA3 password]
Security: WPA3-Personal
Notes:
  Frequency: 5GHz + 2.4GHz
  Guest SSID: MyHomeNetwork-Guest
  Guest Password: [Different password]
  QR Code: [Attached image]

Tags: wifi, home, main-network
```

### Encrypted Credentials File (for scripts)

```bash
# Create encrypted file with GPG
cat > network_creds.txt <<'EOF'
ROUTER_IP=192.168.1.1
ROUTER_USER=admin
ROUTER_PASS=SecurePassword123!
SWITCH_IP=192.168.1.10
SWITCH_USER=admin
SWITCH_PASS=DifferentSecurePass456!
EOF

gpg --symmetric --cipher-algo AES256 network_creds.txt
rm network_creds.txt

# Later, decrypt and source
gpg --decrypt network_creds.txt.gpg > /tmp/creds.tmp
source /tmp/creds.tmp
shred -u /tmp/creds.tmp
```

### .gitignore (for network automation projects)
```
# Credential files
*.gpg
*.key
*.pem
*.p12
*.pfx
credentials.json
secrets.json
network_passwords.txt
ansible_vault_pass.txt

# SSH keys
id_rsa*
id_ed25519*
*.pub

# Network configs with passwords
wpa_supplicant.conf
NetworkManager/system-connections/*

# Backup configs (may contain credentials)
router_backup_*.cfg
switch_config_*.txt
```

## Response Templates

### When network credentials detected in files
```
⚠️  SECURITY ALERT: Network credentials detected in plain text

Location: <file>:<line>
Type: <WiFi password / Router credential / SSH key>
Severity: <CRITICAL/HIGH/MEDIUM/LOW>

Recommendation:
1. Remove credentials from this file immediately
2. Store in password manager (Bitwarden, KeePassXC, etc.)
3. If for automation, use encrypted GPG file
4. Add credential files to .gitignore
5. Rotate the exposed credential

Example secure storage:
[Show password manager or GPG encryption approach]
```

### When helping set up network credential storage
```
I'll help you set up secure network credential management:

1. Choosing storage method (password manager recommended)
2. Creating organized structure for network devices
3. Generating strong, unique passwords
4. Setting up SSH keys for device access
5. Configuring .gitignore for credential protection

[Proceed with setup]
```

### When WiFi password is requested
```
I'll help you manage WiFi credentials securely:

1. Store SSID and password in password manager
2. Generate QR code for easy device connection
3. Set up guest network with separate password
4. Document encryption type and frequency bands

[Show commands and setup]
```

## Common Network Credential Patterns

### Detection patterns for scanning
- WiFi passwords: `wpa_passphrase`, `psk=`, `password=`, SSID with password nearby
- Router credentials: `admin_password`, `router_pass`, `192.168.x.x` with password
- SNMP: `community=`, `snmp_community`, `public`, `private`
- VPN: `vpn_password`, `wireguard` keys, `openvpn` credentials
- Network shares: `smb://`, `nfs:`, with usernames/passwords

### Safe patterns (usually false positives)
- Documentation examples with placeholder passwords
- Configuration templates with `CHANGEME` or `your-password-here`
- Comments explaining password requirements
- Variable names like `wifi_password` without actual values

## Git Integration (Network Automation Projects)

### Before commits
Always scan staged changes for network credentials:
```bash
git diff --staged | grep -iE "(password|psk|wpa_passphrase|community|ssh.*key|192\.168|10\.|172\.16)"
```

### Removing committed network credentials
If network credentials were committed:
1. **Change the password/credential immediately** on all affected devices!
2. Remove from git history using `git filter-branch` or `BFG Repo-Cleaner`
3. Force push (coordinate with team if shared repo)
4. Update .gitignore to prevent recurrence
5. Audit who had access to the repository

## Platform-specific Guidance

### Ansible (Network Automation)
```yaml
# ❌ BAD - Hardcoded credentials
- hosts: routers
  vars:
    ansible_user: admin
    ansible_password: MyRouterPass123

# ✅ GOOD - Ansible Vault
- hosts: routers
  vars:
    ansible_user: admin
    ansible_password: "{{ vault_router_password }}"
```

Create encrypted vault:
```bash
ansible-vault create group_vars/routers/vault.yml
# Add: vault_router_password: MyRouterPass123
```

### Python (Network Scripts)
```python
# ❌ BAD
router_ip = "192.168.1.1"
router_user = "admin"
router_pass = "MyPassword123"

# ✅ GOOD - Using keyring
import keyring
router_pass = keyring.get_password("network_devices", "router_192.168.1.1")

# ✅ GOOD - Using encrypted file
import gnupg
gpg = gnupg.GPG()
with open('creds.gpg', 'rb') as f:
    decrypted = gpg.decrypt_file(f)
    creds = eval(decrypted.data)
```

### Shell Scripts (Network Configuration)
```bash
# ❌ BAD
ROUTER_PASS="MyPassword123"
curl -u admin:$ROUTER_PASS http://192.168.1.1/api

# ✅ GOOD - Prompt for password
read -sp "Router password: " ROUTER_PASS
echo
curl -u admin:$ROUTER_PASS http://192.168.1.1/api
unset ROUTER_PASS

# ✅ GOOD - Use GPG encrypted file
source <(gpg --decrypt network_creds.gpg 2>/dev/null)
curl -u admin:$ROUTER_PASS http://192.168.1.1/api
```

### NetworkManager (WiFi on Linux)
```bash
# Connections stored encrypted in:
# /etc/NetworkManager/system-connections/

# View saved WiFi passwords (requires root)
sudo grep -r "psk=" /etc/NetworkManager/system-connections/

# ❌ Don't copy these files to repos or share unencrypted
# ✅ Document the SSID, but get password from password manager
```

## Additional Resources

For network credential management, consider creating:
- `scripts/network-scan.sh` - Discover and inventory network devices
- `scripts/backup-configs.sh` - Backup device configs (encrypted)
- `scripts/check-default-creds.sh` - Scan for devices using default passwords
- `scripts/rotate-wifi-password.sh` - Automate WiFi password rotation
- `network-inventory.md` - Document of all network devices (no passwords!)

### Useful Tools

- **Password Managers**: Bitwarden (open source), KeePassXC (offline), 1Password
- **Encryption**: GPG, Age encryption
- **Network Scanning**: nmap, arp-scan, avahi-browse
- **SSH Management**: ssh-agent, keychain
- **QR Codes**: qrencode (WiFi passwords)
- **Credential Scanning**: truffleHog, git-secrets

## Network Security Best Practices

### Regular Maintenance
1. **Rotate credentials every 6-12 months**
   - Update password manager with new passwords
   - Record rotation date in notes

2. **Audit device access**
   - Review who has network credentials
   - Remove access for former employees/household members
   - Check for unauthorized devices on network

3. **Update firmware regularly**
   - Check for security updates
   - Test updates in off-hours
   - Keep firmware version in password manager notes

4. **Monitor for default credentials**
   - New IoT devices often use default passwords
   - Change immediately after setup
   - Disable if device doesn't support password change

### Emergency Procedures

If credentials are compromised:
1. **Immediately change passwords** on affected devices
2. **Check device logs** for unauthorized access
3. **Scan network** for unknown devices
4. **Update password manager** with new credentials
5. **Notify team/family** if shared network
6. **Consider factory reset** if device behavior is suspicious

## Success Criteria

A network has good credential hygiene when:
- ✅ All device credentials stored in password manager
- ✅ No default passwords in use
- ✅ Unique passwords for each network device
- ✅ SSH keys used instead of passwords where possible
- ✅ WiFi uses WPA3 or WPA2 with strong password (20+ chars)
- ✅ Guest network isolated from main network
- ✅ Credential rotation schedule documented
- ✅ No plaintext credentials in scripts or config files
- ✅ Network diagram/inventory maintained (without passwords)
- ✅ Family/team members know how to access shared credentials securely
