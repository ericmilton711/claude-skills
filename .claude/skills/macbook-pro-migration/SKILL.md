# MacBook Pro 2012 Migration from Fedora

## Target Machine
- **Model:** 2012 MacBook Pro
- **CPU:** Intel i7 3.0GHz
- **RAM:** 8GB
- **Storage:** 256GB SSD

## Source Machine
- **CPU:** Intel i5-2415M (2011)
- **RAM:** 16GB
- **OS:** Fedora 43 with GNOME

## Migration Options

### Option 1: Clonezilla Full Disk Clone
1. Boot Clonezilla from USB on source machine
2. Clone disk to external drive (image or device-to-device)
3. Restore onto MacBook Pro's 256GB SSD
4. Boot and fix drivers

**Constraint:** Source used space must be ≤ 256GB

### Option 2: Fresh Install + Migrate (Recommended)
1. Fresh Fedora 43 install on the MacBook Pro
2. Export installed packages on source:
   ```bash
   dnf repoquery --installed > packages.txt
   ```
3. Copy home directory:
   ```bash
   rsync -av /home/mac/ user@newmac:/home/mac/
   ```
4. Reinstall packages on target:
   ```bash
   dnf install $(cat packages.txt)
   ```
5. Copy any `/etc/` customizations

## Post-Migration Fixes (Both Options)

### Wi-Fi (Broadcom)
2012 MacBook Pro uses Broadcom Wi-Fi. Have a USB ethernet adapter for first boot.
```bash
sudo dnf install broadcom-wl
# or use b43 driver
```

### Boot (EFI)
- Mac uses EFI; may need `efibootmgr` adjustment
- Hold **Option** key at boot to select Linux partition

### Graphics (Intel HD 4000)
Source has HD 3000, target has HD 4000. Both use `i915` driver. Regenerate initramfs after clone:
```bash
sudo dracut --force
```

### Keyboard / Trackpad
Works out of the box on Fedora. Fix Fn key behavior:
```bash
echo 1 | sudo tee /sys/module/hid_apple/parameters/fnmode
```
To make permanent:
```bash
echo "options hid_apple fnmode=1" | sudo tee /etc/modprobe.d/hid_apple.conf
sudo dracut --force
```

## System Optimizations (Carry Over)
These were applied on the source machine and should be reapplied on fresh install:
1. Disable GNOME animations: `gsettings set org.gnome.desktop.interface enable-animations false`
2. Disable GNOME Software autostart
3. Disable unnecessary services: `abrt*`, `ModemManager`, `iscsi*`, `livesys*`, `qemu-guest-agent`, `mcelog`

## Decision: Wipe macOS, Fresh Fedora Install

**Plan:** Wipe macOS entirely and install Fedora 43. Do NOT keep macOS dual-boot.

### Step-by-Step Order
1. **Before delivery** — Prep on current machine:
   - Create Fedora 43 live USB
   - Export package list: `dnf repoquery --installed > packages.txt`
   - Optionally tar up home directory to external drive
2. **Boot MacBook from USB** — Wipe SSD completely, install Fedora 43
3. **Get networking working** — USB ethernet adapter first, then install Broadcom Wi-Fi drivers
4. **Enable SSH on MacBook:**
   ```bash
   sudo systemctl enable --now sshd
   ```
5. **From the source machine (this computer), SSH in and do the rest:**
   ```bash
   # Copy home directory
   rsync -av /home/mac/ user@<macbook-ip>:/home/mac/
   # Install packages remotely
   ssh user@<macbook-ip> "sudo dnf install -y $(cat packages.txt)"
   ```
6. Claude Code on this machine can drive the entire migration over SSH — just provide the MacBook's IP address
7. **Apply system optimizations** — disable animations, unnecessary services, etc.
8. **Install Claude Code on MacBook** — last step, after everything is configured

### Remote Migration via SSH
The preferred approach is to run Claude Code on the source machine and SSH into the MacBook
for all setup tasks. This avoids needing to sit at the MacBook for the entire process. Requirements:
- Both machines on the same network
- SSH enabled on MacBook (`sshd` running)
- User account created during Fedora install
- Source machine user's SSH key copied to MacBook: `ssh-copy-id user@<macbook-ip>`

### IMPORTANT: Credential & Authentication Setup on MacBook

**Known issue:** Claude Code on new devices has been stubborn about saving usernames and
passwords, requiring manual intervention via an administrator account.

**Before letting Claude Code handle setup remotely, do these steps FIRST on the MacBook
(either at the physical machine or via SSH):**

1. **Ensure the user account has passwordless sudo:**
   ```bash
   # SSH in as root or use the admin account created during install
   echo "mac ALL=(ALL) NOPASSWD:ALL" | sudo tee /etc/sudoers.d/mac
   sudo chmod 440 /etc/sudoers.d/mac
   ```
   This prevents Claude Code from getting blocked on sudo password prompts over SSH.

2. **Set up GNOME Keyring for credential storage:**
   ```bash
   sudo dnf install -y gnome-keyring libsecret
   # Git credential storage using libsecret
   git config --global credential.helper /usr/libexec/git-core/git-credential-libsecret
   ```

3. **Pre-configure git credentials:**
   ```bash
   git config --global user.email "ericmilton711@gmail.com"
   git config --global user.name "ericm"
   ```

4. **If Claude Code still can't save credentials**, fall back to plaintext store:
   ```bash
   git config --global credential.helper store
   # Then do a manual git push/pull to trigger the credential prompt and save it
   ```

5. **For GitHub authentication specifically:**
   ```bash
   # Install GitHub CLI and authenticate
   sudo dnf install -y gh
   gh auth login
   ```
   Do this interactively at the MacBook or via SSH before handing off to Claude Code.

**Why this matters:** On previous devices, Claude Code could not save credentials through
the normal auth flow, requiring manual admin login to fix. Setting up passwordless sudo,
git credential helpers, and `gh auth` BEFORE Claude Code runs avoids this entirely.
