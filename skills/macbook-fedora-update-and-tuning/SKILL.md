# MacBook Pro — Fedora Update, Wi-Fi Driver & Slow-Startup Tuning

Eric's 2013 MacBook Pro (Fedora 43, Haswell i7-4578U, 8 GB RAM, SATA SSD, btrfs, Broadcom BCM4360 Wi-Fi). Claude Code runs locally on this machine — no SSH. Written 2026-10-11 from a long dnf-upgrade / "it's slow" session.

## 1. dnf conflict: tlp vs tuned-ppd

Symptom: `file /usr/share/dbus-1/.../PowerProfiles... conflicts between attempted installs of tlp-1.9.0 and tuned-ppd`.

Cause: tlp 1.9 ships its own power-profiles D-Bus files, same as tuned-ppd.

Fix (nothing depended on them; TLP stays the power manager):
```bash
rpm -q --whatrequires tuned-ppd      # confirm nothing needs it
sudo dnf remove tuned-ppd            # also removes tuned
sudo dnf upgrade
```
Do not reinstall tuned/tuned-ppd.

## 2. Broadcom Wi-Fi (wl) after a kernel update

The card uses the proprietary `wl` driver from RPM Fusion (`akmod-wl`, `broadcom-wl`). It is built per kernel.

- A new kernel installed *during* `dnf upgrade` can fail to install `kmod-wl` with "Transaction failed: Rpm transaction failed" — that's an rpm-lock collision, not a compile error. The built rpm is in `/var/cache/akmods/wl/`.
- Fix after the dnf transaction ends, no network needed:
  ```bash
  sudo akmods --kernels <kernel-version> --force
  find /lib/modules/<kernel-version> -name 'wl.ko*'    # confirm
  ```
- If Wi-Fi is missing after reboot: `sudo akmods --force`, reboot; or boot the previous kernel from GRUB.
- Before a Fedora major upgrade, confirm RPM Fusion has the release: check `download1.rpmfusion.org/nonfree/fedora/{releases,updates}/<N>/...` for `akmod-wl` and `broadcom-wl`.
- `facetimehd` (webcam DKMS) failed to build on kernel 7.2.9 (strncpy implicit declaration). Cosmetic; webcam works on the older kernel.

## 3. GRUB menu is hidden

`grub2-editenv list` shows `menu_auto_hide=1`. To see the menu once: hold Shift/tap Esc after the chime. To always show it:
```bash
sudo grub2-editenv - unset menu_auto_hide
```
Undo: `sudo grub2-editenv - set menu_auto_hide=1`. Old kernels stay in the menu as a fallback.

## 4. "Twice as slow as months ago" — what the investigation found

Measured, not guessed:
- CPU/temps/SSD/memory all healthy; no throttling; SSD ~420 MB/s; SMART passed.
- Turbo on vs off made **no measurable difference** (benchmark within 2%). Don't promise a 2x from TLP CPU settings.
- Wi-Fi was already 5 GHz ~100+ Mbit/s; `iw ... set power_save off` has no effect with the proprietary `wl` driver. TLP drop-ins in `/etc/tlp.d/` were **not** picked up with TLP 1.8.0 — edit `/etc/tlp.conf` directly (back it up first).
- Real cause of "slow after boot": many background jobs at once — duplicate powertop services, `raid-check` (no RAID), `vboxservice` on bare metal, and `gnome-software --mode=updates` burning ~5 min CPU. btrfs fsync ~5 ms makes Firefox feel laggy.
- Ceiling is the Haswell HD 5000 iGPU driving 2560x1600 plus 8 GB RAM.

Changes applied (all reversible):
```bash
sudo systemctl disable powertop.service raid-check.timer vboxservice.service
gsettings set org.gnome.software allow-updates false
gsettings set org.gnome.software download-updates false
printf '[D-BUS Service]\nName=org.gnome.Software\nExec=/bin/false\n' > ~/.local/share/dbus-1/services/org.gnome.Software.service
```
**Keep `/etc/systemd/system/powertop-autotune.service`** — it is Eric's custom unit and also resets brightness to 100 (his rule: never dim).

TLP: `/etc/tlp.conf` set `CPU_BOOST_ON_{AC,BAT}=1` and `CPU_ENERGY_PERF_POLICY_ON_{AC,BAT}=performance` (backup `/etc/tlp.conf.bak-2026-10-11`; original had boost 0, `balance_power`/`power`). Costs battery when unplugged; revert if battery life matters more.

## 5. Gotchas when running commands here

- `pgrep -E` is invalid; use `ps -eo pid,args | grep -E ...`.
- `/tmp` is tmpfs — I/O latency tests there are meaningless; test in `$HOME`.
- Piping a password to `sudo -S` inside a function breaks `echo x | sudo tee`; use `sudo sh -c 'echo x > /sys/...'`.
- Never reboot or close Firefox without asking Eric to save (he has lost in-progress job applications before).
