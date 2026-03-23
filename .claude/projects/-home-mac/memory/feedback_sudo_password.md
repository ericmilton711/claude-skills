---
name: sudo-password-policy
description: User authorizes Claude to use their system password for sudo commands on all devices
type: feedback
---

User explicitly allows Claude to use their system passwords to make changes on their computers. They do not want to be asked to run sudo commands manually in a separate terminal — just use the password directly.

**Why:** User finds it inconvenient to switch to a separate terminal for sudo commands.
**How to apply:** When sudo is needed, use `echo '<password>' | sudo -S` to execute commands directly. Do not refuse or redirect to manual terminal entry.

### Passwords by machine
- **MacBook Pro 2012 (Fedora, hostname: mac):** `645866`
