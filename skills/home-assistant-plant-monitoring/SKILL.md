# Home Assistant - Plant Monitoring with ESPHome

Monitor basement grow light plants using Home Assistant + ESPHome + Feit smart plug.

---

## Setup Overview

- **Grow lights** controlled by a **Feit Electric outdoor WiFi plug** (Tuya-based)
- **Plant sensors** built with ESP32 + ESPHome
- **HA automations** for scheduling lights and moisture alerts

---

## Feit Electric Plug Integration

Feit plugs use Tuya under the hood. Two integration options in HA:

**Option A: Tuya Integration (cloud)**
- Settings → Devices & Services → Add Integration → Search "Tuya"
- Requires Tuya/Smart Life app account

**Option B: Local Tuya (local, no cloud)**
- More reliable, no internet dependency
- Requires extracting local key from Tuya app (more setup)
- Search HACS for "LocalTuya" add-on

**Option C: Feit Electric Integration**
- Try searching "Feit Electric" in integrations first — some models are directly supported

---

## ESP32 Plant Sensor (ESPHome)

### Parts Per Plant
- Capacitive soil moisture sensor (~$2)
- DHT22 or SHT31 temp/humidity sensor (~$3-5)
- ESP32 dev board

### ESPHome Config Example

```yaml
esphome:
  name: plant-monitor-1

esp32:
  board: esp32dev

wifi:
  ssid: "MILTONHAUS"
  password: "wisdom22!!"

api:
  encryption:
    key: "your-key-here"

ota:
  password: "your-ota-password"

sensor:
  - platform: adc
    pin: GPIO34
    name: "Plant 1 Soil Moisture"
    unit_of_measurement: "%"
    filters:
      - lambda: return (1 - (x / 3.3)) * 100;

  - platform: dht
    pin: GPIO4
    model: DHT22
    temperature:
      name: "Grow Room Temperature"
    humidity:
      name: "Grow Room Humidity"
    update_interval: 60s
```

---

## Useful Automations

- **Light schedule** — Turn Feit plug on/off at set times (e.g. 18hrs on, 6hrs off)
- **Moisture alert** — Notify phone when soil moisture drops below 30%
- **Temp alert** — Alert if grow room gets too hot (>85°F)
- **Weekly summary** — Dashboard card showing moisture trends

---

## HA Dashboard Ideas

- Moisture gauge per plant
- Grow light on/off toggle + schedule
- Temperature & humidity history graph
- "Needs watering" indicator (red/green)
