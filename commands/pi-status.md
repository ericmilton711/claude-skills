SSH into the Homestead Pi and report its status.

Use key auth: ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no eric@192.168.12.114

Run these commands:
1. uptime
2. python3 /home/eric/homestead.py status
3. free -h
4. df -h /
5. vcgencmd measure_temp

Report the results in a clean summary: uptime, load, recent log activity, memory, disk, and temperature.
