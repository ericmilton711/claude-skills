# OctoPrint — Ender 3 Pro Wireless Printing

Send G-code directly to the Creality Ender 3 Pro over the network. No more SD card swapping.

## Hardware Needed
- **Raspberry Pi 4 (2GB)** — dedicated to the printer
- **USB-A to Mini-B cable** — Ender 3 Pro uses Mini USB (not Micro)
- Power supply + microSD card for the Pi (included in starter kits)

## Shopping Links (Amazon)
- [Raspberry Pi 4 Model B 2GB (board only)](https://www.amazon.com/Raspberry-Model-2019-Quad-Bluetooth/dp/B07TD42S27)
- [CanaKit Raspberry Pi 4 Starter Kit - 2GB (everything included)](https://www.amazon.com/CanaKit-Raspberry-Pi-Starter-Kit/dp/B07V2B4W63)
- [Raspberry Pi 4B 2GB Budget Kit](https://www.amazon.com/Raspberry-Pi-2GB-Budget-Kit/dp/B0DJ1HY5KG)

## Setup Steps (once parts arrive)
1. Flash **OctoPi** image to the microSD card (use Raspberry Pi Imager)
2. Configure WiFi in the imager settings before first boot
3. Insert SD into Pi, power on
4. Connect Pi to Ender 3 Pro via USB-A to Mini-B cable
5. Open browser → `http://octopi.local` to access OctoPrint web UI
6. Run the setup wizard (set printer profile for Ender 3 Pro: 220x220x250mm bed)
7. Upload G-code files from `~/Desktop/gcode files/` and print

## Printer Connection Details
- **Printer:** Creality Ender 3 Pro
- **USB Port:** Mini-B
- **Baud Rate:** 115200 (auto-detect usually works)
- **Build Volume:** 220 x 220 x 250 mm

## G-code File Location
All sliced G-code files are on Eric's desktop: `C:\Users\ericm\Desktop\gcode files\`
See the `/gcode-files` skill for the full inventory.

## Optional Upgrades
- **Pi Camera** — live print monitoring and timelapse
- **OctoPrint plugins:** The Spaghetti Detective (AI fail detection), OctoLapse (timelapses), Bed Level Visualizer
- **Command-line upload:** `curl -F "file=@myprint.gcode" http://octopi.local/api/files/local -H "X-Api-Key: YOUR_KEY"`
