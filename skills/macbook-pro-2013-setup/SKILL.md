# MacBook Pro 2013 - Fedora Setup

Setup notes and specs for Eric's MacBook Pro (Late 2013) running Fedora 43.

## Hardware Specs

| Component | Details |
|-----------|---------|
| Model | MacBook Pro 11,1 (13-inch, Late 2013) |
| CPU | Intel Core i7-4578U @ 3.0 GHz (dual-core, 4 threads) |
| RAM | 7.7 GB (soldered — cannot be upgraded) |
| Storage | ~232 GB SSD |
| OS | Fedora 43 |

## GitHub Setup on This Machine

- Repo cloned to: `~/claude-skills/`
- Remote URL configured with PAT for push access
- GitHub username: ericmilton711

### Git Config
```bash
git config --global user.email "ericmilton711@gmail.com"
git config --global user.name "ericmilton711"
```

## Network

- This machine IP: check with `ip addr`
- Lenovo laptop (40GB RAM): 192.168.2.2
- Mac Mini (Fedora): 192.168.12.163

## SSH Access to Other Machines

```bash
# Lenovo laptop
ssh ericmilton@192.168.2.2

# Mac Mini
ssh ericmilton@192.168.12.163
```

## Performance Notes

- RAM is limited at 7.7 GB — avoid heavy multitasking
- CPU-intensive apps (e.g. Cura slicer) will cause noticeable slowdowns
- SSD storage is fast and healthy (~223 GB free as of 2026-03-27)
- Cura 3D printing software not recommended on this machine due to RAM constraints

## Conversation Log (2026-03-27)

- Checked disk space: 232 GB SSD, 223 GB free
- Confirmed RAM is 7.7 GB and soldered (no upgrade possible)
- Decided against installing Cura due to RAM limitations
- SSHed into Lenovo (192.168.2.2) — connected OK, no GitHub repos found there
- Attempted SSH to Mac Mini (192.168.12.163) — auth issues
- Cloned claude-skills repo from GitHub
- Configured GitHub PAT for push access
