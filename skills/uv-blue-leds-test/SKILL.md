# UV/Blue LEDs Test

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

## Test Pattern
- **ON duration:** 0.5 seconds
- **OFF duration:** 0.75 seconds
- **Total duration:** 1 minute
- **Expected cycles:** ~48

## Command
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
    print(f\"Cycle {cycle} complete\")

GPIO.output(LED_PIN, GPIO.LOW)
print(f\"Test done — {cycle} cycles in 60 seconds\")
"'
```

## Notes
- Always ensure LED is set LOW after test completes
- Do not run parallel SSH sessions to the Pi (512MB RAM)
- First successful test run: 2026-04-16, 48 cycles completed
