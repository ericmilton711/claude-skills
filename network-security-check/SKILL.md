# Network Security Check (Windows)

Check WireGuard VPN status, tunnel type, and Pi-hole DNS filtering on Windows.

---

## 1. Check if WireGuard is running

```powershell
Get-Service -Name WireGuard* -ErrorAction SilentlyContinue
Get-Process -Name wireguard* -ErrorAction SilentlyContinue
```

Look for active services and the tunnel name (e.g., `WireGuardTunnel: Lambert`).

---

## 2. Check tunnel type (split vs. full)

```powershell
Get-NetRoute -InterfaceAlias "Lambert" | Select-Object DestinationPrefix, NextHop, RouteMetric | Format-Table -AutoSize
```

- **Split-tunnel**: Only specific subnets (e.g., `192.168.2.0/24`) route through WireGuard. General internet traffic goes out normally.
- **Full-tunnel**: `0.0.0.0/0` appears in the routes — all traffic goes through the VPN.

Eric's Lambert tunnel is **split-tunnel** — only routes `192.168.2.0/24` and `192.168.0.0/24` (Lambert home network subnets). Used for accessing Nextcloud and Milton Homepage remotely, not for general internet privacy.

---

## 3. Check what DNS server is in use

```powershell
Get-DnsClientServerAddress | Where-Object { $_.AddressFamily -eq 2 } | Select-Object InterfaceAlias, ServerAddresses | Format-Table -AutoSize
```

Note: Use `powershell -File - << 'EOF' ... EOF` syntax when piping PowerShell through bash to avoid extglob issues with `Where-Object`.

Eric's Pi-hole is at `192.168.12.163`. If Wi-Fi shows `192.168.12.1` (router), Pi-hole may not be active unless the router forwards DNS to it.

---

## 4. Test if Pi-hole is actually filtering

```powershell
Resolve-DnsName "doubleclick.net" -ErrorAction SilentlyContinue | Select-Object Name, Type, IPAddress
```

- **Pi-hole blocking**: Returns `0.0.0.0` or no result
- **Not filtered**: Returns real Google/ad network IPs (e.g., `142.251.x.x`)

---

## Fix: Route DNS through Pi-hole

Option A — Set this laptop's DNS manually to `192.168.12.163` in Wi-Fi adapter settings.

Option B — Configure the router (`192.168.12.1`) to use `192.168.12.163` as upstream DNS so all devices get Pi-hole automatically.

---

## Notes

- PowerShell `Where-Object` with bash heredoc requires `powershell -File -` syntax, not inline `-Command` with `Where-Object { $_.Property }` — the `$_` gets mangled by bash extglob.
- Eric's network: MILTONHAUS WiFi, Pi-hole on Mac Mini at `192.168.12.163`, router at `192.168.12.1`.
