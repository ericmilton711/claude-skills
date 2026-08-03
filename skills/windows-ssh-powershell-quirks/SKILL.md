# Windows SSH / PowerShell Quirks

Lessons learned debugging the ntfy-listener on Eva's MSI laptop (2026-07-05). Applies to any SSH work against the kids' Windows laptops (Eva, Gianna, Patrick, Benedict, Tower of Gondor).

## 1. Nested quoting through SSH → cmd.exe → PowerShell is a trap

Windows OpenSSH runs remote commands via `cmd.exe /c "<command>"` by default. Wrapping another `powershell -Command "..."` inside that (which itself may invoke a third `-File` layer) mangles quotes, especially when a path contains a space (e.g. `C:\Users\Eva Milton\...`).

**Fix: use `-EncodedCommand` (base64 UTF-16LE) for anything non-trivial.** Sidesteps all quoting layers entirely.

```bash
python3 -c "
import base64
cmd = '''Your-PowerShell -Here \$WithVariables \"and quotes\"'''
print(base64.b64encode(cmd.encode('utf-16-le')).decode())
"
# then:
ssh -i ~/.ssh/id_ed25519 "eva milton@192.168.12.202" "powershell -EncodedCommand <base64>"
```

Use a Python raw string (`r'''...'''`) when the command contains Windows paths with backslashes, or `\U`/`\A` etc. will trigger Python unicode-escape errors.

The `#< CLIXML` line in the output is normal PowerShell progress-stream noise (module loading) — not an error.

## 2. `param()` must be the very first statement in a .ps1 file

Only comments/blank lines may precede it. If `param(...)` appears anywhere else (e.g. line 61 of a file that starts with `Add-Type` calls), PowerShell parses `param` as a bare command and throws `CommandNotFoundException` — and the parameter is silently never bound, so the script always falls back to its default value.

## 3. `-WindowStyle Hidden` does not reliably hide the console on Windows 11 24H2

Windows Terminal is now the default console host, and it can ignore the `-WindowStyle Hidden` flag passed to `powershell.exe`, leaving a visible flashing/scrolling terminal window even though the flag says hidden.

**Reliable fix: launch through a VBScript wrapper**, which controls window visibility directly and isn't subject to the Windows Terminal quirk:

```vbscript
Set WshShell = CreateObject("WScript.Shell")
WshShell.Run "powershell.exe -ExecutionPolicy Bypass -File ""C:\path\to\script.ps1"" -Arg value", 0, False
```

Point the Startup-folder shortcut (or scheduled task) at `wscript.exe "C:\path\to\wrapper.vbs"` instead of at `powershell.exe` directly.

## 4. SSH sessions land in Session 0 — can't launch GUI into the user's desktop

An interactive Windows login (console) runs in a numbered session (e.g. Session 5/6). An SSH connection is non-interactive and runs in **Session 0**, which has no window station. Anything with a GUI (WinForms, popups, visible windows) launched from an SSH session either fails silently or simply never becomes visible in the real desktop — verify with `query user` (shows the real session) vs checking `SessionId` on the spawned process via `Get-CimInstance Win32_Process | Select SessionId`.

Things that do **NOT** solve this:
- `Start-Process -WindowStyle Hidden` from SSH — still Session 0.
- `schtasks /create ... /ru "user" /it` (the flag meant to run a task in the logged-on user's interactive session) — did not actually launch into the interactive session in practice; the "SUCCESS: Attempted to run" message does not guarantee it worked, and the resulting process (if any) was not found afterward.

**The only reliable way to get a process into the real interactive session from here is to have it launch automatically at login** (Startup folder shortcut, or a scheduled task triggered "At log on"). There is no clean remote workaround — this is a genuine OS security boundary, not a scripting problem.

## 5. Processes survive killing your local SSH client — check for zombies

If you run a command over SSH and then kill the **local** `ssh` client process (e.g. via a bash timeout wrapper) to stop watching output, the **remote** process may keep running — especially if it was given a real console (no `-WindowStyle Hidden`). This can leave an orphaned process with a visible window that keeps generating output indefinitely.

Always explicitly re-check and `Stop-Process -Id <pid> -Force` any diagnostic process you spun up, rather than assuming local cleanup killed it remotely.

## 6. Diagnosing "it just times out" connectivity issues

`Invoke-WebRequest` against a **streaming/long-poll endpoint** (like `ntfy.sh/<topic>/json`) will always "time out" even when working correctly, because it waits for the full response body. Test plain pages (`https://ntfy.sh/`) or raw sockets instead:

```powershell
$client = New-Object System.Net.Sockets.TcpClient
$task = $client.ConnectAsync($ip, 443)
$ok = $task.Wait(5000)
if ($ok -and $client.Connected) { "CONNECTED" } else { "TIMED OUT / FAILED" }
$client.Close()
```

Note: in Windows PowerShell 5.1 (.NET Framework), the parameterless `TcpClient()` constructor binds to IPv4 immediately — passing an IPv6 literal address throws `"None of the discovered or specified addresses match the socket address family"`. Not a real network error, just a family mismatch.

Checklist for "one specific site won't load on one specific device" (ruled out in this order, cheapest first):
1. DNS resolution (`nslookup <site>. 192.168.12.136`) — trailing dot avoids `.lan` suffix issues
2. Pi-hole client group / `groups: []` via the API
3. General internet connectivity from the same device (raw TCP to `1.1.1.1:443`)
4. Same test **from a different device on the same LAN** — isolates device-specific vs network-wide
5. WireGuard/VPN routes (`Get-NetRoute`, `Find-NetRoute -RemoteIPAddress <ip>`) — a tunnel with broad `AllowedIPs` can silently capture traffic that should go direct
6. `netsh winhttp show proxy`
7. `Get-NetFirewallRule -Direction Outbound -Action Block -Enabled True`
8. `Get-MpPreference | Select EnableNetworkProtection` (Defender's Network Protection can silently block outbound to unfamiliar IPs)
9. Hosts file (`C:\Windows\System32\drivers\etc\hosts`)

If all of the above check out clean but the block persists for one device only, it's likely a **router-level per-device policy** (mesh router parental-control/threat-protection tier) that isn't visible from the device or from Pi-hole — needs router admin access to confirm.

## 7. `ping` is not a valid "is this laptop on?" test

Windows 11 blocks ICMP echo (ping) via the default Windows Defender Firewall profile — a 100% packet-loss ping does **not** mean the machine is off or unreachable. Confirmed 2026-07-24 on Eva's laptop: ping timed out completely while an SSH command (`ssh "eva milton@192.168.12.202" "echo alive"`) succeeded instantly and the device showed live Pi-hole query traffic. **Use SSH (or a live query in the Pi-hole log) to check reachability, never ping.**

## 8. Reaching a kid's laptop when it's off the home LAN (different Wi-Fi network)

When a kids' laptop is on a foreign network (hotel/hotspot/relative's house) instead of the home LAN, its known home IP (e.g. `192.168.12.239`) usually won't respond — even SSH to that address times out or shows "no route to host," because the address only makes sense on the home subnet.

**Try in this order:**

1. **mDNS hostname first: `ssh user@<hostname>.local`.** Windows advertises its hostname over mDNS on the local network by default (confirmed working 2026-08-03 reaching Benedict's laptop as `kids2.local` on an unfamiliar guest Wi-Fi). This is the cleanest fix — no IP guessing needed, works regardless of what subnet the foreign router handed out.
2. **If mDNS fails, try a same-subnet ARP trick as a fallback:** add a secondary IP address to your own machine's Wi-Fi interface inside the device's *old* subnet (e.g. `sudo ip addr add 192.168.12.50/24 dev wlp3s0` if the target's home IP was `192.168.12.239/24`). Two hosts sharing an IP subnet can ARP and talk directly over the same physical Wi-Fi broadcast domain even though the router's actual DHCP range is completely different. This worked intermittently in testing — treat it as a fallback, not primary, since some routers/APs enforce client isolation that blocks it outright (as seen previously on a phone hotspot).
3. **Don't trust `ping` to rule out reachability** — see lesson 7. A failed ping doesn't mean SSH/mDNS won't work.

**Diagnosing "should I even bother fixing this device's network config?":** before assuming a static IP is stuck and needs fixing, just SSH in and ask it:
```powershell
netsh interface ip set address name="Wi-Fi" source=dhcp
```
If the adapter is already on DHCP, this command is a harmless no-op and reports `DHCP is already enabled on this interface.` — a cheap way to confirm current state without needing to parse `ipconfig` output first.

## Related

- [[reference_ntfy]] — ntfy topic/device map
- [[reference_kids2_laptop]] — Benedict's laptop, reachable via `kids2.local` mDNS when off the home LAN
- Eva's MSI laptop: `eva milton@192.168.12.202`, key auth, username has a space (must be quoted in ssh commands: `"eva milton@192.168.12.202"`)
