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
4. **Run the migration** — rsync home dir, reinstall packages from list
5. **Install Claude Code** — only after the system is fully configured
6. **Apply system optimizations** — disable animations, unnecessary services, etc.

### Why This Order
- Claude Code needs a working OS first — nothing useful it can do during imaging
- Migration steps are simple copy/paste commands, follow from the skill or phone
- Fresh install avoids all driver mismatch issues from cloning
