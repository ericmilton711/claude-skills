SSH into the Homestead Pi and report its status.

Use key auth: ssh -T -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114

Run these commands (one SSH session at a time, use `timeout` on each):
1. uptime
2. cat /home/eric/homestead.log
3. free -h
4. df -h /
5. vcgencmd measure_temp

Always show the FULL homestead.log output. Report the results in a clean summary: uptime, load, memory, disk, temperature, and the complete homestead log.

Then run the UV/Blue LEDs blink test — GPIO 17, 0.5s ON, 0.75s OFF, for 1 minute:
```
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
Report when the blink test starts and how many cycles completed.
