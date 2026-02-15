# Raspberry Pi 3 A+ Camera Setup

**Date**: January 29, 2026
**Hardware**: Raspberry Pi 3 Model A+
**Camera**: OV5647 (Camera Module v1, 5MP)
**OS**: Raspberry Pi OS (Bookworm/64-bit)

## Camera Detected

```
0 : ov5647 [2592x1944 10 bit GBRG] (/base/soc/i2c0mux/i2c@1/ov5647@36)
```

## Setup Summary

### Physical Connection
- Camera connected to CSI port (between HDMI and audio jack)
- Blue side of ribbon cable facing Ethernet/USB ports

### Software Installation

On newer Raspberry Pi OS (Bookworm), the camera tools are `rpicam-apps` (not `libcamera-apps`):

```bash
sudo apt update && sudo apt install -y rpicam-apps
```

### Basic Commands

```bash
# List detected cameras
rpicam-hello --list-cameras

# Take a photo
rpicam-still -o photo.jpg

# Take a photo with specific resolution
rpicam-still -o photo.jpg --width 2592 --height 1944

# 5 second preview (requires display)
rpicam-hello -t 5000

# Record video (10 seconds)
rpicam-vid -t 10000 -o video.h264

# Convert h264 to mp4
ffmpeg -i video.h264 -c copy video.mp4
```

## Notes

- No camera option in `raspi-config` on newer OS - camera is enabled by default
- Legacy `raspistill`/`raspivid` commands replaced by `rpicam-*` commands
- OV5647 is the Camera Module v1 sensor (5MP, max 2592x1944)

---

## Future Possible Projects

### Webcam / Streaming
- **MJPG-streamer**: Simple HTTP streaming for local network viewing
- **MediaMTX (rtsp-simple-server)**: RTSP/WebRTC streaming server
- **Pi Camera Web Interface**: Web-based control panel

```bash
# Quick MJPG stream example
sudo apt install -y mjpg-streamer
mjpg_streamer -i "input_libcamera.so" -o "output_http.so -w /usr/share/mjpg-streamer/www"
# Access at http://<pi-ip>:8080
```

### Timelapse Photography
```bash
# Take a photo every 60 seconds for 24 hours
rpicam-still -t 86400000 --timelapse 60000 -o timelapse_%04d.jpg

# Create video from images
ffmpeg -framerate 30 -pattern_type glob -i 'timelapse_*.jpg' -c:v libx264 timelapse.mp4
```

### Motion Detection
- **Motion**: Classic Linux motion detection software
- **MotionEye**: Web-based frontend for Motion
- **Frigate**: NVR with AI object detection (requires more resources)

```bash
# Install Motion
sudo apt install -y motion

# Config file at /etc/motion/motion.conf
```

### Security Camera / Baby Monitor
- Combine streaming + motion detection
- Add notifications via email/Telegram/Pushover
- Record on motion events

### Machine Learning / Computer Vision
- **TensorFlow Lite**: Object detection on Pi
- **OpenCV**: Image processing and computer vision
- **Picamera2**: Python library for camera control

```bash
# Install picamera2 for Python
sudo apt install -y python3-picamera2
```

### Print Monitoring (3D Printer)
- **OctoPrint**: 3D printer control with camera support
- Monitor prints remotely
- Timelapse of prints

### Nature/Wildlife Camera
- Motion-triggered capture
- IR camera module for night vision
- Battery/solar powered setup

---

**Last Updated**: January 29, 2026
