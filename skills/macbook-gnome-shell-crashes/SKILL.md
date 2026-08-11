# MacBook Pro — gnome-shell Crash Diagnosis

Recurring gnome-shell crashes on Eric's MacBook Pro (2013, Fedora 43, Intel Haswell Iris 5100 iGPU). This machine is where Claude Code itself runs — **no SSH needed, just run commands locally.** Eric gets an ntfy push (topic `MILTONHAUS-Reminders`) when it crashes and asks Claude to pull a fresh backtrace.

**Do NOT recommend reinstalling Fedora.** X11 fallback is also unavailable — Fedora 43 shipped Wayland-only GNOME sessions, no `xorg-x11-server-Xorg` GNOME session package exists. Exhaust journalctl/coredumpctl diagnosis first.

## Standard diagnosis commands

```bash
journalctl --list-boots --no-pager                      # find the boot(s) in question
journalctl -b -N -p err --no-pager                       # errors in that boot
coredumpctl list gnome-shell --no-pager                   # list all gnome-shell coredumps
coredumpctl info gnome-shell -1 --no-pager | grep -A 40 "Stack trace of thread"   # backtrace of latest crash
gsettings get org.gnome.shell disable-user-extensions     # check crash-recovery side effect (see below)
gnome-extensions list --enabled                            # confirm what's actually active
dnf check-update mutter gnome-shell mesa-dri-drivers       # check for a fix already shipped
```

To find the exact crash moment and what was happening right before it:
```bash
journalctl -b 0 --no-pager -S "<a few min before>" -U "<crash time>" | grep -iE "suspend|resume|lid|hdmi|monitor|hotplug"
journalctl -b 0 --no-pager -S "<crash time -1min>" -U "<crash time +2min>" | grep -iE "gnome-shell|coredump"
```

## Known crash signatures so far

**1. 2026-08-09 SIGABRT + 2026-08-10 SIGSEGV — Wayland client-destroy path.**
Backtrace: `wl_client_destroy → remove_and_destroy_resource` (libwayland-server) called from a GJS/mutter callback. Prime suspect: the **burn-my-windows** extension, which hooks window/actor destruction for close animations — same code path. Not proven (no exact upstream bug match), but disabling it is the right mitigation regardless. Related but non-matching data points: GNOME/mutter GitLab #2404 (open bug, `wl_resource_destroy → wl_list_remove`, random crashes on window open/close); Burn-My-Windows GitHub #135 (memory *leak*, not crash, same code path).
**Fix applied:** `dnf update -y gnome-shell mutter` (49.8→49.9 / 49.6→49.7, needs logout/login) + `gnome-extensions disable burn-my-windows@schneegans.github.com`.

**2. 2026-08-11 07:20 SIGSEGV — native/KMS backend, different signature, fix above did NOT prevent it.**
Backtrace: `drmIoctl → drmModeCloseFB → meta_drm_buffer_finalize → g_object_unref → meta_frame_native_release → clutter_frame_unref → meta_onscreen_native_promote_posted_frame → notify_view_crtc_presented`. This is mutter's native GPU backend releasing a DRM framebuffer during normal frame presentation — nothing to do with window-close animations, so burn-my-windows is ruled out for this one. Happened spontaneously (no suspend/resume/lid/display-hotplug event beforehand), mid-morning (breaks the "only crashes in the evening" pattern from signature #1). Journal shows `Device '/dev/dri/card1' prefers shadow buffer` — this old Haswell iGPU uses mutter's GBM shadow-buffer rendering path, a less-common code path historically prone to native-backend bugs on older Intel hardware. A same-function-family Launchpad bug (#2069565, `meta_drm_buffer_get_width` SIGSEGV) turned out to be a hybrid NVIDIA/Nouveau multi-GPU issue, fixed upstream in mutter 47 — doesn't match this single-GPU Intel machine already on mutter 49.7, so ruled out as the same bug.
**No fix identified yet.** Already on latest mutter/gnome-shell/mesa. One improvement: gnome-shell auto-restarted itself via GDM in ~3 seconds this time — no freeze, no manual reboot needed.
**Next step if it recurs:** process-of-elimination disable the remaining active extensions (dash-to-dock, weatherornot) one at a time — last rendering-adjacent candidates. If it still recurs with all extensions off, this is a bare mutter/i915 native-backend bug on this hardware with no user-level fix; wait for a mesa/kernel/mutter update.

## Side effect to always check after ANY crash

Each crash can trigger GNOME's own `org.gnome.Shell-disable-extensions.service` ("Disable GNOME Shell extensions after failure"), which sets `org.gnome.shell disable-user-extensions` to `true` — a global master switch that force-disables ALL user extensions regardless of the per-extension list. This is why an extension (e.g. Dash to Dock) can silently stop working after a crash even though it was never touched. Check with:
```bash
gsettings get org.gnome.shell disable-user-extensions
gsettings set org.gnome.shell disable-user-extensions false   # if it got flipped to true
```

## Also found, not a crash cause but worth checking once

`wg-quick@lambert.service` was enabled and failing/auto-restarting every 10s since boot (`wg-quick: 'lambert' already exists`) — a redundant leftover of the WireGuard "lambert" tunnel, which is properly managed via NetworkManager on this machine. Disabled with `sudo systemctl disable --now wg-quick@lambert.service`; confirmed NetworkManager's own `lambert` connection stayed connected (Lambert/Nextcloud/Home-Assistant access unaffected — see `miltonhaus-wireguard` skill).
