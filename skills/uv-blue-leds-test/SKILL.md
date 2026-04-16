# UV/Blue LEDs Test

This skill is the source of truth for the Pi status check and LED blink test. The `/pi-status` command reads this skill every time it runs.

## Purpose
Blink the LED on GPIO 17 (SSR #1) to test placement for the UV and Blue bug-attraction LEDs in the chicken run. Currently a green LED is in that position for testing purposes.

## Hardware
- **Pi:** Homestead Pi — `eric@192.168.12.114`
- **GPIO Pin:** 17 (BCM) → SSR #1 → DROK buck (12V→3.2V) → LED circuit
- **Current test LED:** Green (placeholder for 5x UV + 5x Blue LEDs)

## SSH Access
```bash
ssh -T -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114
```

## Current Test Pattern
- **ON duration:** 0.5 seconds
- **OFF duration:** 0.75 seconds
- **Total duration:** 1 minute
- **Expected cycles:** ~48

## Blink Test Command
```bash
timeout 90 ssh -T -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114 'python3 -c "
import RPi.GPIO as GPIO
import time

LED_PIN = 17
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(LED_PIN, GPIO.OUT)

print(\"Starting UV/Blue LED test — 0.5s ON, 0.75s OFF for 1 minute\")
start = time.time()
cycle = 0
while time.time() - start < 60:
    cycle += 1
    GPIO.output(LED_PIN, GPIO.HIGH)
    time.sleep(0.5)
    GPIO.output(LED_PIN, GPIO.LOW)
    time.sleep(0.75)

GPIO.output(LED_PIN, GPIO.LOW)
print(f\"Test done — {cycle} cycles in 60 seconds\")
"'
```

## Status Check Commands
These are run before the LED test every time status is requested:
```bash
timeout 60 ssh -T -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114 \
  'uptime; echo "===LOG==="; cat /home/eric/homestead.log; echo "===MEM==="; free -h; echo "===DISK==="; df -h /; echo "===TEMP==="; vcgencmd measure_temp 2>/dev/null || cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null || echo "n/a"'
```

## Notes
- Always ensure LED is set LOW after test completes
- Never run parallel SSH sessions to the Pi (512MB RAM)
- Pi is very slow for ~3 minutes after reboot — use `timeout 60` and retry if needed
- To change the test pattern, update the "Current Test Pattern" and "Blink Test Command" sections above — `/pi-status` will pick up the changes automatically
- First successful test run: 2026-04-16, 48 cycles completed

## Test History
| Date | Result | Cycles | Notes |
|------|--------|--------|-------|
| 2026-04-16 07:16 | PASS | 48 | First test, pre-reboot |
| 2026-04-16 07:30 | PASS | 48 | Post-reboot verification |
