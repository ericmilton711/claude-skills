# SSD Upgrade & Drive Cloning

description: Guide for upgrading HDD to SSD and cloning drives on Linux systems

## When to Use This Skill

- User wants to speed up an old computer
- User asks about SSD upgrades or recommendations
- User needs to clone a drive
- User is migrating to a new storage device

## Check Current Drive Type

```bash
# Check if HDD (ROTA=1) or SSD (ROTA=0)
lsblk -d -o NAME,ROTA,TYPE,SIZE | grep disk

# Check disk usage
df -h /

# Check connection type
lsblk -o NAME,SIZE,TYPE,TRAN
```

## SSD Recommendations

### Best 2.5" SATA SSDs (for older systems)

| Drive | Capacity | Use Case |
|-------|----------|----------|
| Samsung 870 EVO | 250GB-4TB | Best reliability, 5-year warranty |
| Crucial MX500 | 250GB-4TB | Great value, good performance |
| Kingston A400 | 120GB-960GB | Budget option |

**Buying tip:** 500GB is often the same price or cheaper than 250GB due to manufacturing economics. Check both prices.

### Purchase Links

- Amazon: Search "Samsung 870 EVO [capacity]"
- Walmart, Newegg, Micro Center also carry these

## Hardware Requirements for Cloning

### USB-to-SATA Adapter

Required to connect the new SSD externally during cloning.

**Recommended adapters (~$10):**
- SABRENT EC-SSHD (popular, reliable)
- StarTech USB3S2SAT3CB
- UGREEN SATA to USB 3.0

**Features to look for:**
- USB 3.0 or higher (5Gbps+)
- UASP support for faster transfers
- No external power needed for 2.5" drives

## Cloning Methods

### Method 1: Clonezilla (GUI - Recommended for beginners)

1. Download Clonezilla Live: https://clonezilla.org/downloads.php
2. Create bootable USB:
   ```bash
   # Using dd (replace sdX with your USB drive)
   sudo dd if=clonezilla-live.iso of=/dev/sdX bs=4M status=progress
   ```
3. Connect new SSD via USB adapter
4. Boot from Clonezilla USB
5. Select: `device-device` → `Beginner` → `disk_to_local_disk`
6. Choose source (internal HDD) and target (USB-connected SSD)
7. Wait for clone to complete (speed depends on data size)
8. Shut down, swap drives physically, boot

### Method 2: dd (Command line)

Boot from a Live USB (Fedora, Ubuntu, etc.), then:

```bash
# Identify drives
lsblk

# Clone entire disk (sda=source, sdb=target)
# WARNING: Double-check drive letters! This overwrites target completely
sudo dd if=/dev/sda of=/dev/sdb bs=64M status=progress conv=fsync

# If target is larger, expand partition afterward:
sudo growpart /dev/sdb 3      # Adjust partition number as needed
sudo resize2fs /dev/sdb3      # For ext4 filesystems
sudo xfs_growfs /dev/sdb3     # For XFS filesystems
```

### Method 3: rsync (For same or larger target only)

```bash
# Create partitions on new drive first, then:
sudo rsync -aAXv --exclude={"/dev/*","/proc/*","/sys/*","/tmp/*","/run/*","/mnt/*","/media/*","/lost+found"} / /mnt/newdrive/

# Reinstall bootloader afterward
sudo grub2-install --root-directory=/mnt/newdrive /dev/sdb
```

## Machine-Specific Notes

### Mac Mini (2011-2014)

- Uses standard 2.5" SATA drives
- Requires T6 and T8 Torx screwdrivers
- Logic board removal tool helpful but not required
- Guide: Search "iFixit Mac Mini [year] Hard Drive Replacement"

### Laptops (General)

- Most use 2.5" SATA or M.2 drives
- Check manual for drive type before purchasing
- Some newer laptops (post-2018) use NVMe M.2 only

## Post-Clone Verification

```bash
# After swapping drives and booting:

# Verify you're on the new drive
lsblk

# Check filesystem health
sudo fsck -n /dev/sda3

# Verify boot entries
sudo grub2-mkconfig -o /boot/grub2/grub.cfg  # Fedora
sudo update-grub                              # Ubuntu/Debian
```

## Troubleshooting

### Won't boot after clone

1. Boot from Live USB
2. Chroot into the new drive:
   ```bash
   sudo mount /dev/sda3 /mnt
   sudo mount /dev/sda1 /mnt/boot/efi  # If UEFI
   sudo chroot /mnt
   grub2-install /dev/sda
   grub2-mkconfig -o /boot/grub2/grub.cfg
   exit
   ```

### Clone seems stuck

- `dd` can appear frozen - check with `kill -USR1 $(pgrep ^dd)`
- Large drives take time: ~10-30 min for 50GB of data

### Partition table issues

```bash
# Regenerate partition table UUIDs if needed
sudo sgdisk -G /dev/sdb
```

## Performance Comparison

| Metric | HDD (5400rpm) | HDD (7200rpm) | SATA SSD |
|--------|---------------|---------------|----------|
| Sequential Read | 80-100 MB/s | 100-120 MB/s | 550 MB/s |
| Sequential Write | 80-100 MB/s | 100-120 MB/s | 520 MB/s |
| Random 4K | 0.5-1 MB/s | 1-2 MB/s | 50+ MB/s |
| Boot time | 45-90 sec | 30-60 sec | 10-20 sec |
