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

---

## MILTONHAUS Parental Controls Strategy

**Goal:** Kids' devices locked to approved sites only via Pi-hole. Eric and Rosemary bypass restrictions using ExpressVPN on their devices.

**Kids' devices** — no ExpressVPN. DNS through Pi-hole. Only whitelisted sites work.

**Parent devices** (Eric's Windows laptop, Fedora MacBook Pro, Rosemary's MacBook Pro) — ExpressVPN installed:

| State | Pi-hole filtering | Milton Homepage (WireGuard) |
|---|---|---|
| ExpressVPN OFF | Yes — same restrictions as kids | Available |
| ExpressVPN ON | Bypassed — all sites accessible | **Still available** (see note below) |

**Important:** ExpressVPN and WireGuard actually coexist simultaneously. Because WireGuard is split-tunnel and claims specific routes (`192.168.2.0/24`, `192.168.0.0/24`), Windows longest-prefix matching sends Lambert-bound traffic through WireGuard and everything else through ExpressVPN. They do not conflict. Milton Homepage remains accessible with ExpressVPN on.

The only effect of ExpressVPN is bypassing Pi-hole DNS for general internet traffic (ExpressVPN uses its own DNS `100.64.100.1`).

**Pi-hole setup options:**

- **Router-level (recommended):** Set router (`192.168.12.1`) upstream DNS to Pi-hole (`192.168.12.163`). All devices filtered by default. Kids can't bypass without router access.
- **Device-level:** Manually set DNS to `192.168.12.163` on kids' devices only. Fragile — kids could manually change DNS to bypass.

**Key devices:**
- Pi-hole: Mac Mini at `192.168.12.163`
- Router: `192.168.12.1`
- Eric's laptop: `192.168.12.220`
- Rosemary's MacBook Pro: `192.168.12.109`
- Fedora MacBook Pro: `192.168.12.189`
