SSH into the Homestead Pi and report its status, then run the UV/Blue LED blink test.

**IMPORTANT:** Before running, read the skill at `~/.claude/skills/uv-blue-leds-test/SKILL.md` for the current LED test parameters and status check configuration. That skill is the source of truth — follow whatever it says for the test pattern, timing, and GPIO pin.

Use key auth: ssh -T -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114

## Post-reboot note
The Pi 3 A+ (512MB RAM) is very slow for ~3 minutes after a reboot. If a command times out, wait and retry with a longer timeout (60s+). Never stack parallel SSH sessions.

## Status check
Run these commands (use `timeout 60` on each SSH call):
1. uptime
2. cat /home/eric/homestead.log
3. free -h
4. df -h /
5. vcgencmd measure_temp

Always show the FULL homestead.log output. Report the results in a clean summary: uptime, load, memory, disk, temperature, and the complete homestead log.

## LED blink test
After the status report, run the UV/Blue LED blink test as defined in the `uv-blue-leds-test` skill. Report when the test starts and how many cycles completed.
