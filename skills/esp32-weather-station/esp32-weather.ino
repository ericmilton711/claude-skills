#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Update.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <time.h>
#include "esp_task_wdt.h"

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define FORECAST_DAYS 6
#define WDT_TIMEOUT_S 30

const char* ssid = "DIEMILTONHAUS";
const char* password = "wisdom22!!";
const char* ntpServer = "pool.ntp.org";

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

float tempC = 0, tempF = 0, dhtHumidity = 0;
bool sensorConnected = false;
unsigned long lastSensorRead = 0;
unsigned long lastWifiCheck = 0;
int wifiFailCount = 0;

struct WxData {
  String oTemp = "--";
  String oHigh = "--";
  String oLow = "--";
  String oHum = "--";
  String oWind = "--";
  String desc = "Loading...";
  String sunrise = "--:--";
  String sunset = "--:--";
  String fcDay[FORECAST_DAYS];
  String fcHigh[FORECAST_DAYS];
  String fcLow[FORECAST_DAYS];
  String fcDesc[FORECAST_DAYS];
};

WxData wx;
SemaphoreHandle_t wxMutex;

volatile bool otaInProgress = false;



String nwsDescToEmoji(const String& desc) {
  String d = desc;
  d.toLowerCase();
  if (d.indexOf("thunder") >= 0) return "&#9889;";
  if (d.indexOf("snow") >= 0 || d.indexOf("blizzard") >= 0) return "&#127784;&#65039;";
  if (d.indexOf("rain") >= 0 || d.indexOf("shower") >= 0 || d.indexOf("drizzle") >= 0) return "&#127783;&#65039;";
  if (d.indexOf("fog") >= 0 || d.indexOf("mist") >= 0 || d.indexOf("haze") >= 0) return "&#127787;&#65039;";
  if (d.indexOf("cloud") >= 0 || d.indexOf("overcast") >= 0) return "&#9925;";
  if (d.indexOf("partly") >= 0) return "&#9925;";
  if (d.indexOf("sunny") >= 0 || d.indexOf("clear") >= 0) return "&#9728;&#65039;";
  return "&#127780;&#65039;";
}

bool nwsFetch(const char* url, JsonDocument& doc, JsonDocument* filter = nullptr) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("User-Agent", "(miltonhaus-weather, ericmilton711@gmail.com)");
  http.addHeader("Accept", "application/geo+json");
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  http.useHTTP10(true);
  int code = http.GET();
  bool ok = false;
  if (code == 200) {
    DeserializationError err = filter
      ? deserializeJson(doc, http.getStream(), DeserializationOption::Filter(*filter))
      : deserializeJson(doc, http.getStream());
    if (!err) ok = true;
    else Serial.printf("NWS JSON error: %s\n", err.c_str());
  } else {
    Serial.printf("NWS fetch failed (%s): %d\n", url, code);
  }
  http.end();
  return ok;
}

void fetchCurrentObs(WxData& d) {
  JsonDocument filter;
  filter["properties"]["temperature"]["value"] = true;
  filter["properties"]["relativeHumidity"]["value"] = true;
  filter["properties"]["windSpeed"]["value"] = true;
  filter["properties"]["textDescription"] = true;

  JsonDocument doc;
  if (!nwsFetch("https://api.weather.gov/stations/KLNS/observations/latest", doc, &filter)) return;

  JsonObject props = doc["properties"];

  float tempCVal = props["temperature"]["value"].as<float>();
  float tempFVal = tempCVal * 9.0 / 5.0 + 32.0;
  d.oTemp = String(tempFVal, 1);

  float rh = props["relativeHumidity"]["value"].as<float>();
  d.oHum = String((int)rh);

  float windKmh = props["windSpeed"]["value"].as<float>();
  float windMph = windKmh * 0.621371;
  d.oWind = String(windMph, 1);

  const char* desc = props["textDescription"];
  if (desc) {
    d.desc = nwsDescToEmoji(String(desc)) + " " + String(desc);
  }

  Serial.println("NWS current observations updated");
}

void fetchForecast(WxData& d) {
  JsonDocument filter;
  JsonObject pf = filter["properties"]["periods"].add<JsonObject>();
  pf["isDaytime"] = true;
  pf["temperature"] = true;
  pf["name"] = true;
  pf["shortForecast"] = true;

  JsonDocument doc;
  if (!nwsFetch("https://api.weather.gov/gridpoints/CTP/128,27/forecast", doc, &filter)) return;

  JsonArray periods = doc["properties"]["periods"];

  int dayIdx = 0;
  bool todayHighSet = false;
  bool todayLowSet = false;

  for (JsonObject p : periods) {
    bool daytime = p["isDaytime"].as<bool>();
    int temp = p["temperature"].as<int>();
    const char* name = p["name"];
    const char* shortFc = p["shortForecast"];

    if (!todayHighSet && daytime) {
      d.oHigh = String(temp);
      todayHighSet = true;
      continue;
    }
    if (!todayLowSet && !daytime) {
      d.oLow = String(temp);
      todayLowSet = true;
      if (!todayHighSet) {
        todayHighSet = true;
        d.oHigh = "--";
      }
      continue;
    }

    if (daytime && dayIdx < FORECAST_DAYS) {
      d.fcDay[dayIdx] = String(name).substring(0, 3);
      d.fcHigh[dayIdx] = String(temp);
      d.fcDesc[dayIdx] = nwsDescToEmoji(String(shortFc)) + " " + String(shortFc);
    }
    if (!daytime && dayIdx < FORECAST_DAYS) {
      d.fcLow[dayIdx] = String(temp);
      dayIdx++;
    }
  }

  Serial.println("NWS forecast updated");
}

void fetchSunriseSunset(WxData& d) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, "https://api.sunrise-sunset.org/json?lat=39.98&lng=-76.28&formatted=0");
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  http.useHTTP10(true);
  int code = http.GET();
  if (code == 200) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getStream())) {
      String sr = doc["results"]["sunrise"].as<String>();
      String ss = doc["results"]["sunset"].as<String>();
      auto utcIsoToLocal = [](const String& iso) -> String {
        int y, mo, d, h, mi, s;
        sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &s);
        struct tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon = mo - 1;
        t.tm_mday = d;
        t.tm_hour = h;
        t.tm_min = mi;
        t.tm_sec = s;
        // Temporarily set TZ to UTC so mktime treats input as UTC
        setenv("TZ", "UTC0", 1);
        tzset();
        time_t epoch = mktime(&t);
        setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
        tzset();
        struct tm lt;
        localtime_r(&epoch, &lt);
        int lh = lt.tm_hour;
        char buf[12];
        const char* ampm = (lh >= 12) ? "PM" : "AM";
        if (lh == 0) lh = 12;
        else if (lh > 12) lh -= 12;
        snprintf(buf, sizeof(buf), "%d:%02d %s", lh, lt.tm_min, ampm);
        return String(buf);
      };

      d.sunrise = utcIsoToLocal(sr);
      d.sunset = utcIsoToLocal(ss);
      Serial.printf("Sunrise: %s  Sunset: %s\n", d.sunrise.c_str(), d.sunset.c_str());
    }
  } else {
    Serial.printf("Sunrise API failed: %d\n", code);
  }
  http.end();
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  WxData fresh;
  fetchCurrentObs(fresh);
  fetchForecast(fresh);
  fetchSunriseSunset(fresh);
  if (xSemaphoreTake(wxMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    wx = fresh;
    xSemaphoreGive(wxMutex);
  }
}

void weatherTask(void* param) {
  for (;;) {
    fetchWeather();
    vTaskDelay(pdMS_TO_TICKS(600000));
  }
}

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    sensorConnected = false;
  } else {
    sensorConnected = true;
    tempC = t;
    tempF = t * 9.0 / 5.0 + 32.0;
    dhtHumidity = h;
  }
}


const char page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>MILTONHAUS Weather</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link href="https://fonts.googleapis.com/css2?family=Comfortaa:wght@400;700&display=swap" rel="stylesheet">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    html, body { height: 100%; }
    body {
      font-family: 'Comfortaa', sans-serif; color: #eef1ea;
      background: radial-gradient(120% 140% at 15% 0%, #2c3d31 0%, #16201a 55%, #0e150f 100%);
      height: 100dvh; display: flex; flex-direction: column; overflow: hidden;
    }
    .icon { width: 1em; height: 1em; display: inline-block; vertical-align: -0.15em; stroke: currentColor; stroke-width: 1.6; fill: none; stroke-linecap: round; stroke-linejoin: round; }
    /* ---- Top bar ---- */
    .topbar { flex: 0 0 auto; padding: clamp(10px,1.8vh,18px) clamp(14px,2vw,24px) clamp(6px,1vh,10px); display: flex; justify-content: space-between; align-items: center; }
    .tb-title { font-weight: 700; font-size: clamp(0.95rem,2.2vh,1.1rem); letter-spacing: 3px; text-transform: uppercase; color: #a9b8ac; display: flex; align-items: center; gap: 8px; }
    .tb-title .icon { width: 1.1em; height: 1.1em; }
    .tb-right { display: flex; align-items: center; gap: clamp(10px,1.6vw,16px); }
    .hourly-pill { background: rgba(216,180,92,0.14); border: 1px solid rgba(216,180,92,0.4); color: #d8b45c; border-radius: 20px; padding: clamp(6px,1.2vh,8px) clamp(12px,1.6vw,16px); font-family: inherit; font-size: clamp(0.68rem,1.5vh,0.75rem); font-weight: 700; display: flex; align-items: center; gap: 6px; cursor: pointer; }
    .hourly-pill:active { transform: scale(0.96); }
    .tb-clock { text-align: right; line-height: 1.15; }
    .tb-clock-t { font-size: clamp(1.3rem,2.8vh,1.7rem); font-weight: 700; color: #eef1ea; }
    .tb-clock-d { font-size: clamp(0.62rem,1.3vh,0.72rem); color: #7f9284; }
    .fs-btn { background: rgba(255,255,255,0.08); border: none; color: #a9b8ac; border-radius: 8px; padding: 4px 8px; font-size: 1.1rem; cursor: pointer; line-height: 1; }
    .fs-btn:active { background: rgba(255,255,255,0.16); }
    :fullscreen .fs-btn, :-webkit-full-screen .fs-btn { display: none; }
    /* ---- Layout: fills the viewport, never scrolls ---- */
    .body { flex: 1; min-height: 0; padding: clamp(4px,1vh,8px) clamp(14px,2vw,24px) clamp(12px,2vh,20px); display: grid; grid-template-columns: 2.1fr 1.1fr; gap: clamp(12px,1.8vw,20px); }
    /* ---- Hero: current conditions, no boxed card ---- */
    .hero { position: relative; display: flex; flex-direction: column; gap: clamp(10px,1.6vh,18px); padding: clamp(4px,1vh,10px) 4px 4px; min-height: 0; }
    .hero-watermark { position: absolute; top: -30px; right: -16px; width: 200px; height: 200px; color: rgba(216,180,92,0.05); stroke-width: 1; z-index: 0; pointer-events: none; }
    .hero-loc, .hero-row, .hero-meta, .hero-events, .fc-strip { position: relative; z-index: 1; }
    .hero-loc { font-size: clamp(0.75rem,1.7vh,0.85rem); letter-spacing: 2px; text-transform: uppercase; color: #5f7466; }
    .hero-row { display: flex; align-items: center; gap: clamp(12px,2vw,18px); }
    .icon-chip { width: clamp(66px,10.5vh,98px); height: clamp(66px,10.5vh,98px); border-radius: 50%; background: linear-gradient(145deg,#d8b45c,#a9832f); display: flex; align-items: center; justify-content: center; flex: 0 0 auto; box-shadow: 0 6px 18px rgba(180,140,50,0.35); }
    .icon-chip .icon { width: 50%; height: 50%; color: #1a1408; }
    .hero-temp { font-size: clamp(5rem, min(20vh,25vw), 10rem); font-weight: 700; line-height: 1.15; letter-spacing: -2px; }
    .hero-temp .u { font-size: 0.3em; color: #7f9284; vertical-align: super; }
    .hero-meta { padding-left: clamp(80px,13vh,116px); display: flex; gap: clamp(10px,1.6vw,16px); align-items: center; flex-wrap: wrap; }
    .hero-cond { font-size: clamp(1.1rem,2.5vh,1.35rem); font-weight: 700; color: #d8b45c; }
    .hero-hl { font-size: clamp(0.88rem,2vh,1.05rem); color: #7f9284; }
    /* ---- Today's events (family calendar) ---- */
    .hero-events { display: flex; flex-direction: column; gap: 8px; padding: clamp(10px,1.8vh,16px) 0 0 clamp(80px,13vh,116px); margin-top: clamp(90px,16vh,170px); border-top: 1px solid rgba(255,255,255,0.08); }
    .hero-events-label { font-size: 0.7rem; letter-spacing: 2px; text-transform: uppercase; color: #5f7466; margin-bottom: 2px; }
    .event-row { display: flex; align-items: baseline; gap: 10px; font-size: clamp(0.85rem,1.9vh,0.98rem); }
    .event-time { font-weight: 700; color: #d8b45c; font-size: 0.85em; min-width: 58px; flex-shrink: 0; }
    .event-title { color: #c7d4cb; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
    .event-row.placeholder .event-title { color: #6c8074; font-style: italic; }
    /* ---- 6-day forecast ---- */
    .fc-strip { display: flex; align-items: flex-end; gap: clamp(5px,0.8vw,8px); flex: 1; min-height: 0; }
    .fc-item { flex: 1; min-width: 0; display: flex; flex-direction: column; align-items: center; justify-content: center;
      border-radius: 16px; padding: clamp(6px,1.2vh,12px) 2px; text-align: center;
      background: rgba(255,255,255,0.03); border: 1px solid rgba(255,255,255,0.06); cursor: pointer; transition: background 0.15s; }
    .fc-item:active { transform: scale(0.96); }
    .fc-day { font-size: clamp(0.75rem,1.7vh,0.85rem); color: #8fa294; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; margin-bottom: clamp(3px,0.7vh,6px); }
    .fc-icon { width: clamp(16px,3vh,20px); height: clamp(16px,3vh,20px); color: #8fa294; margin-bottom: clamp(3px,0.7vh,6px); }
    .fc-hi { font-weight: 700; font-size: clamp(1.05rem,2.3vh,1.3rem); }
    .fc-lo { font-size: clamp(0.8rem,1.8vh,0.95rem); color: #6c8074; }
    /* ---- Right rail: indoor + conditions + kids ---- */
    .rail { display: flex; flex-direction: column; gap: clamp(8px,1.4vh,14px); min-height: 0; }
    .glass { background: rgba(255,255,255,0.045); border: 1px solid rgba(255,255,255,0.08); border-radius: 20px; padding: clamp(10px,1.8vh,16px) clamp(12px,1.8vw,18px); }
    .glass-title { font-size: 0.72rem; letter-spacing: 2px; text-transform: uppercase; color: #7f9284; font-weight: 700; margin-bottom: clamp(6px,1.2vh,10px); }
    .indoor-row { display: flex; align-items: center; gap: clamp(10px,1.8vw,14px); }
    .ring { position: relative; width: clamp(48px,9vh,64px); height: clamp(48px,9vh,64px); flex: 0 0 auto; }
    .ring-bg {
      width: 100%; height: 100%; border-radius: 50%;
      background: conic-gradient(from 225deg,
        #d8b45c 0deg, #e2703a var(--deg, 0deg),
        rgba(255,255,255,0.08) var(--deg, 0deg), rgba(255,255,255,0.08) 270deg,
        transparent 270deg);
      -webkit-mask: radial-gradient(transparent 60%, #000 61%);
      mask: radial-gradient(transparent 60%, #000 61%); }
    .ring-bg.offline { background: conic-gradient(rgba(255,255,255,0.08) 0deg 360deg); }
    .ring-val { position: absolute; inset: 0; display: flex; align-items: center; justify-content: center; font-size: clamp(0.7rem,1.7vh,0.85rem); font-weight: 700; }
    .indoor-detail .t1 { font-size: clamp(1rem,2.4vh,1.3rem); font-weight: 700; }
    .indoor-detail .t2 { font-size: clamp(0.62rem,1.5vh,0.72rem); color: #7f9284; }
    .stat-line { display: flex; justify-content: space-between; align-items: center; padding: clamp(5px,1vh,7px) 0; border-bottom: 1px solid rgba(255,255,255,0.06); }
    .stat-line:last-of-type { border-bottom: none; }
    .stat-line .lbl { display: flex; align-items: center; gap: 8px; font-size: clamp(0.85rem,1.9vh,1rem); color: #a9b8ac; }
    .stat-line .lbl .icon { width: 17px; height: 17px; }
    .stat-line .val { font-size: clamp(0.92rem,2.1vh,1.08rem); font-weight: 700; }
    .led-toggle { display: flex; background: rgba(255,255,255,0.06); border-radius: 14px; padding: 4px; margin-top: 6px; }
    .led-btn { flex: 1; border: none; background: none; font-family: inherit; font-weight: 700; font-size: clamp(0.68rem,1.6vh,0.78rem); padding: clamp(7px,1.4vh,9px) 0; border-radius: 11px; cursor: pointer; color: #a9b8ac; }
    .led-btn.active { background: #5fce8b; color: #0f1a13; }
    .led-btn:active { transform: scale(0.96); }
    /* ---- Kids ---- */
    .kid-strip { display: grid; grid-template-columns: repeat(3,1fr); grid-template-rows: repeat(2,1fr); gap: clamp(6px,1vh,8px); flex: 1; min-height: 0; }
    .kid-chip { display: flex; flex-direction: column; align-items: center; justify-content: center; gap: clamp(4px,0.8vh,6px);
      border-radius: 16px; background: rgba(255,255,255,0.04); border: 1px solid rgba(255,255,255,0.07);
      padding: 4px 2px; cursor: pointer; transition: background 0.15s; min-width: 0; }
    .kid-chip:active { transform: scale(0.96); background: rgba(255,255,255,0.08); }
    .kid-avatar { width: clamp(30px,6vh,40px); height: clamp(30px,6vh,40px); border-radius: 50%; display: flex; align-items: center; justify-content: center;
      font-weight: 700; font-size: clamp(0.8rem,1.8vh,0.95rem); color: #0f1a13; background: linear-gradient(145deg,#d8b45c,#a9832f); flex-shrink: 0; }
    .kid-chip-name { font-size: clamp(0.72rem,1.6vh,0.82rem); font-weight: 700; color: #dfe6df; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; max-width: 100%; }
    /* ---- Footer ---- */
    .footer { flex: 0 0 auto; text-align: center; padding: clamp(3px,0.8vh,6px) 0; color: #5f7466; font-size: clamp(0.58rem,1.2vh,0.68rem); }
    .footer a { color: #d8b45c; }
    /* ---- Narrow phones: stack, allow scroll ---- */
    @media (max-width: 700px) {
      body { height: auto; min-height: 100dvh; overflow-y: auto; }
      .body { display: flex; flex-direction: column; padding: 8px 14px 16px; }
      .hero-watermark { display: none; }
      .fc-strip { flex: none; flex-wrap: wrap; }
      .fc-item { flex: 1 1 30%; }
      .rail { flex: none; }
      .kid-strip { flex: none; min-height: 140px; }
      .tb-title { letter-spacing: 1px; }
    }
    /* ---- Overlays (hourly / day detail / kid detail) ---- */
    .overlay { display: none; position: fixed; inset: 0; background: #12190f; z-index: 100; overflow-y: auto; padding: 18px; }
    .overlay.open { display: block; }
    .overlay-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; max-width: 1200px; margin-left: auto; margin-right: auto; }
    .overlay-header h2 { color: #d8b45c; font-size: clamp(1rem,2.4vh,1.3rem); }
    .close-btn { background: linear-gradient(135deg,#d8b45c,#a9832f); color: #1a1408; border: none; border-radius: 30px; padding: clamp(10px,1.6vh,18px) clamp(24px,3vw,48px); font-family: inherit; font-size: clamp(1rem,2.2vh,1.5rem); font-weight: 700; cursor: pointer; }
    .close-btn:active { transform: scale(0.96); }
    .overlay-body { max-width: 1200px; margin: 0 auto; }
    .hourly-row { background: rgba(255,255,255,0.04); border: 1px solid rgba(255,255,255,0.08); border-radius: 12px; padding: 11px 16px; margin-bottom: 7px; display: flex; justify-content: space-between; align-items: center; font-size: 1em; }
    .hourly-row .hr-time { font-weight: 700; min-width: 80px; }
    .hourly-row .hr-desc { color: #a9b8ac; font-size: 0.88em; flex: 1; display: flex; align-items: center; justify-content: center; gap: 6px; }
    .hourly-row .hr-temp { color: #d8b45c; font-weight: 700; }
    .hourly-row .hr-wind { color: #6fb8dd; font-size: 0.88em; min-width: 70px; text-align: right; }
    .overlay-loading { text-align: center; color: #7f9284; padding: 40px; }
    .day-period { background: rgba(255,255,255,0.04); border: 1px solid rgba(255,255,255,0.08); border-radius: 14px; padding: 16px 20px; margin-bottom: 12px; }
    .dp-name { font-size: clamp(0.75rem,1.8vh,0.95rem); color: #d8b45c; text-transform: uppercase; letter-spacing: 2px; font-weight: 700; margin-bottom: 4px; }
    .dp-temp { font-size: clamp(2rem,5vh,3rem); font-weight: 700; color: #eef1ea; margin-bottom: 8px; }
    .dp-detail { color: #c7d4cb; font-size: clamp(0.88rem,2vh,1.1rem); line-height: 1.5; margin-bottom: 10px; }
    .dp-stats { display: flex; gap: clamp(12px,2vw,24px); flex-wrap: wrap; }
    .dp-stats span { font-size: clamp(0.78rem,1.7vh,0.95rem); color: #a9b8ac; }
    .kid-section { background: rgba(255,255,255,0.04); border: 1px solid rgba(255,255,255,0.08); border-radius: 14px; padding: 16px 20px; margin-bottom: 12px; }
    .kid-section-title { font-size: clamp(0.75rem,1.8vh,0.95rem); color: #d8b45c; text-transform: uppercase; letter-spacing: 2px; font-weight: 700; margin-bottom: 12px; }
    .chore-item { display: flex; align-items: center; gap: 14px; color: #eef1ea; font-size: clamp(0.9rem,2vh,1.1rem); padding: 10px 2px; border-bottom: 1px solid rgba(255,255,255,0.08); line-height: 1.3; cursor: pointer; }
    .chore-item:last-child { border-bottom: none; }
    .chore-check { width: 24px; height: 24px; flex-shrink: 0; accent-color: #d8b45c; cursor: pointer; }
    .chore-done span { text-decoration: line-through; color: #6c8074; }
    .schedule-text p { color: #eef1ea; font-size: clamp(0.9rem,2vh,1.1rem); line-height: 1.8; margin: 0; }
  </style>
</head>
<body>
  <div class="topbar">
    <div class="tb-title"><svg class="icon" viewBox="0 0 24 24"><path d="M3 11l9-8 9 8"/><path d="M5 10v10h14V10"/></svg>MILTONHAUS Weather</div>
    <div class="tb-right">
      <button class="hourly-pill" onclick="showHourly()"><svg class="icon" viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 3"/></svg>Hourly</button>
      <div class="tb-clock"><div class="tb-clock-t" id="clock">--:--</div><div class="tb-clock-d" id="date">Loading...</div></div>
      <button class="fs-btn" id="fsBtn" onclick="goFullscreen()" title="Fullscreen">&#x26F6;</button>
    </div>
  </div>
  <div class="body">
    <div class="hero">
      <svg class="hero-watermark icon" viewBox="0 0 24 24"><circle cx="12" cy="12" r="4.5"/><path d="M12 2v3M12 19v3M4.2 4.2l2.1 2.1M17.7 17.7l2.1 2.1M2 12h3M19 12h3M4.2 19.8l2.1-2.1M17.7 6.3l2.1-2.1"/></svg>
      <div class="hero-loc">Willow Street, PA</div>
      <div class="hero-row">
        <div class="icon-chip" id="oIconChip"></div>
        <div class="hero-temp"><span id="oTemp">--</span><span class="u">&deg;F</span></div>
      </div>
      <div class="hero-meta">
        <div class="hero-cond" id="oCond">Loading...</div>
        <div class="hero-hl" id="oHL">H: -- / L: --</div>
      </div>
      <div class="hero-events">
        <div class="hero-events-label">Today &mdash; Family Calendar</div>
        <div id="calEvents"><div class="event-row placeholder"><span class="event-title">Loading...</span></div></div>
      </div>
      <div class="fc-strip" id="forecast"></div>
    </div>
    <div class="rail">
      <div class="glass">
        <div class="glass-title">Indoor</div>
        <div class="indoor-row">
          <div class="ring"><div class="ring-bg" id="gaugeRing"></div><div class="ring-val"><span id="tempF">--</span>&deg;</div></div>
          <div class="indoor-detail">
            <div class="t1"><span id="dhtH">--</span>% humidity</div>
            <div class="t2" id="status">Loading...</div>
          </div>
        </div>
      </div>
      <div class="glass">
        <div class="glass-title">Conditions</div>
        <div class="stat-line"><div class="lbl"><svg class="icon" viewBox="0 0 24 24"><path d="M6.5 18a4 4 0 0 1-.5-7.97A5.5 5.5 0 0 1 16.6 8.1 4.5 4.5 0 0 1 16 18H6.5Z"/></svg>Humidity</div><div class="val"><span id="oHum">--</span>%</div></div>
        <div class="stat-line"><div class="lbl"><svg class="icon" viewBox="0 0 24 24"><path d="M3 8h11a3 3 0 1 0-3-3M3 16h15a3 3 0 1 1-3 3M3 12h9"/></svg>Wind</div><div class="val"><span id="oWind">--</span> mph</div></div>
        <div class="stat-line"><div class="lbl"><svg class="icon" viewBox="0 0 24 24"><path d="M12 2v4M4.9 6.9l2.8 2.8M2 16h3M19 16h3M16.3 9.7l2.8-2.8M6 16a6 6 0 0 1 12 0"/></svg>Sunrise</div><div class="val" id="sunrise">--:--</div></div>
        <div class="stat-line"><div class="lbl"><svg class="icon" viewBox="0 0 24 24"><path d="M6 16a6 6 0 0 1 12 0M2 16h3M19 16h3M12 2v4"/></svg>Sunset</div><div class="val" id="sunset">--:--</div></div>
        <div class="stat-line" style="border-bottom:none;padding-bottom:0;"><div class="lbl"><svg class="icon" viewBox="0 0 24 24"><path d="M9 18h6M10 21h4M12 3a6 6 0 0 0-3.6 10.8c.6.45 1 1.15 1 1.95V16h5.2v-.25c0-.8.4-1.5 1-1.95A6 6 0 0 0 12 3Z"/></svg>Chicken Lights <span id="chiOffline" style="display:none;color:#d8836b;font-size:0.62rem;margin-left:4px;">(offline)</span></div></div>
        <div class="led-toggle">
          <button class="led-btn" id="ledOnBtn" onclick="chickenOn()">ON</button>
          <button class="led-btn" id="ledOffBtn" onclick="chickenOff()">OFF</button>
        </div>
      </div>
      <div class="kid-strip">
        <div class="kid-chip" onclick="showKid(0)"><div class="kid-avatar">B</div><div class="kid-chip-name">Benedict</div></div>
        <div class="kid-chip" onclick="showKid(1)"><div class="kid-avatar">E</div><div class="kid-chip-name">Evangelina</div></div>
        <div class="kid-chip" onclick="showKid(2)"><div class="kid-avatar">G</div><div class="kid-chip-name">Gianna</div></div>
        <div class="kid-chip" onclick="showKid(3)"><div class="kid-avatar">P</div><div class="kid-chip-name">Patrick</div></div>
        <div class="kid-chip" onclick="showKid(4)"><div class="kid-avatar">C</div><div class="kid-chip-name">Clementine</div></div>
        <div class="kid-chip" onclick="showKid(5)"><div class="kid-avatar">A</div><div class="kid-chip-name">Adelaide</div></div>
      </div>
    </div>
  </div>
  <div class="footer">NWS Weather every 10 min &bull; <a href="/update">OTA Update</a></div>
  <div class="overlay" id="hourlyOverlay">
    <div class="overlay-header">
      <h2>Hourly Forecast &mdash; Next 24 Hours</h2>
      <button class="close-btn" onclick="closeHourly()">Back</button>
    </div>
    <div class="overlay-body" id="hourlyList"><div class="overlay-loading">Loading...</div></div>
  </div>
  <div class="overlay" id="dayOverlay">
    <div class="overlay-header">
      <h2 id="dayOverlayTitle">Day Detail</h2>
      <button class="close-btn" onclick="closeDayDetail()">Back</button>
    </div>
    <div class="overlay-body" id="dayDetail"><div class="overlay-loading">Loading...</div></div>
  </div>
  <div class="overlay" id="kidOverlay">
    <div class="overlay-header">
      <h2 id="kidOverlayName">...</h2>
      <div style="display:flex;align-items:center;gap:12px;">
        <a href="http://192.168.12.136:8181/kids-admin" target="_blank" style="color:#d8b45c;font-size:0.85rem;text-decoration:none;display:flex;align-items:center;gap:5px;"><svg class="icon" viewBox="0 0 24 24"><path d="M16.5 3.5a2.1 2.1 0 0 1 3 3L7 19l-4 1 1-4Z"/></svg>Edit</a>
        <button class="close-btn" onclick="closeKid()">Back</button>
      </div>
    </div>
    <div class="overlay-body" id="kidDetail"><div class="overlay-loading">Loading...</div></div>
  </div>
  <script>
    void(function(){var c=document.getElementById('clock'),d=document.getElementById('date');setInterval(function(){var n=new Date();c.textContent=n.toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:true,timeZone:'America/New_York'});d.textContent=n.toLocaleDateString('en-US',{weekday:'short',month:'short',day:'numeric',timeZone:'America/New_York'});},1000)}());
    var ICONS={
      sun:'<svg class="icon" viewBox="0 0 24 24"><circle cx="12" cy="12" r="4.5"/><path d="M12 2v3M12 19v3M4.2 4.2l2.1 2.1M17.7 17.7l2.1 2.1M2 12h3M19 12h3M4.2 19.8l2.1-2.1M17.7 6.3l2.1-2.1"/></svg>',
      cloud:'<svg class="icon" viewBox="0 0 24 24"><path d="M6.5 18a4 4 0 0 1-.5-7.97A5.5 5.5 0 0 1 16.6 8.1 4.5 4.5 0 0 1 16 18H6.5Z"/></svg>',
      rain:'<svg class="icon" viewBox="0 0 24 24"><path d="M6.5 15a4 4 0 0 1-.5-7.97A5.5 5.5 0 0 1 16.6 5.1 4.5 4.5 0 0 1 16 15H6.5Z"/><path d="M8 18l-1 3M12 18l-1 3M16 18l-1 3"/></svg>',
      snow:'<svg class="icon" viewBox="0 0 24 24"><path d="M6.5 13a4 4 0 0 1-.5-7.97A5.5 5.5 0 0 1 16.6 3.1 4.5 4.5 0 0 1 16 13H6.5Z"/><path d="M9 17v4M15 17v4M12 18v4"/></svg>',
      fog:'<svg class="icon" viewBox="0 0 24 24"><path d="M3 9h18M3 13h18M3 17h12"/></svg>',
      thunder:'<svg class="icon" viewBox="0 0 24 24"><path d="M6.5 13a4 4 0 0 1-.5-7.97A5.5 5.5 0 0 1 16.6 3.1 4.5 4.5 0 0 1 16 13H13l-2 5h4l-4 6 1-5H9l1-6Z"/></svg>'
    };
    var condIcon=function(desc){var d=(desc||'').toLowerCase();if(d.indexOf('thunder')>=0)return ICONS.thunder;if(d.indexOf('snow')>=0||d.indexOf('blizzard')>=0)return ICONS.snow;if(d.indexOf('rain')>=0||d.indexOf('shower')>=0||d.indexOf('drizzle')>=0)return ICONS.rain;if(d.indexOf('fog')>=0||d.indexOf('mist')>=0||d.indexOf('haze')>=0)return ICONS.fog;if(d.indexOf('cloud')>=0||d.indexOf('overcast')>=0||d.indexOf('partly')>=0)return ICONS.cloud;if(d.indexOf('sunny')>=0||d.indexOf('clear')>=0)return ICONS.sun;return ICONS.cloud;};
    void(function(){var u=function(){fetch('/data').then(function(r){return r.json()}).then(function(d){var s=document.getElementById('status');var ring=document.getElementById('gaugeRing');if(d.sensor){s.textContent='Sensor online';ring.classList.remove('offline');document.getElementById('tempF').textContent=d.tempF.toFixed(1);document.getElementById('dhtH').textContent=d.dhtH.toFixed(0);var fr=Math.max(0,Math.min(1,(d.tempF-40)/50));ring.style.setProperty('--deg',(fr*270)+'deg')}else{s.textContent='Sensor not connected';ring.classList.add('offline');document.getElementById('tempF').textContent='--';document.getElementById('dhtH').textContent='--'}document.getElementById('oTemp').textContent=d.oTemp;document.getElementById('oHL').textContent='H: '+d.oHigh+'° / L: '+d.oLow+'°';document.getElementById('oHum').textContent=d.oHum;document.getElementById('oWind').textContent=d.oWind;var parts=d.oDesc.split(' ');parts.shift();document.getElementById('oCond').textContent=parts.join(' ');document.getElementById('oIconChip').innerHTML=condIcon(d.oDesc);document.getElementById('sunrise').textContent=d.sunrise;document.getElementById('sunset').textContent=d.sunset;var fc=document.getElementById('forecast');var h='';for(var i=0;i<d.forecast.length;i++){var f=d.forecast[i];h+='<div class="fc-item" onclick="showDayDetail(\''+f.day+'\')"><div class="fc-day">'+f.day+'</div>'+condIcon(f.desc)+'<div class="fc-hi">'+f.hi+'°</div><div class="fc-lo">'+f.lo+'°</div></div>'}fc.innerHTML=h}).catch(function(){document.getElementById('status').textContent='Connection lost'})};u();setInterval(u,5000)}());
  function showHourly(){document.getElementById('hourlyOverlay').className='overlay open';document.getElementById('hourlyList').innerHTML='<div class="overlay-loading">Loading...</div>';fetch('https://api.weather.gov/gridpoints/CTP/128,27/forecast/hourly',{headers:{'Accept':'application/geo+json'}}).then(function(r){return r.json()}).then(function(d){var p=d.properties.periods;var h='';var count=Math.min(p.length,24);for(var i=0;i<count;i++){var t=new Date(p[i].startTime);var hr=t.getHours();var ampm=hr>=12?'PM':'AM';if(hr===0)hr=12;else if(hr>12)hr-=12;var timeStr=hr+':00 '+ampm;h+='<div class="hourly-row"><div class="hr-time">'+timeStr+'</div><div class="hr-desc">'+condIcon(p[i].shortForecast)+' '+p[i].shortForecast+'</div><div class="hr-temp">'+p[i].temperature+'°</div><div class="hr-wind">'+p[i].windSpeed+'</div></div>'}document.getElementById('hourlyList').innerHTML=h}).catch(function(){document.getElementById('hourlyList').innerHTML='<div class="overlay-loading">Failed to load hourly forecast</div>'});}
  function closeHourly(){document.getElementById('hourlyOverlay').className='overlay';}
  function showDayDetail(day){document.getElementById('dayOverlay').className='overlay open';document.getElementById('dayOverlayTitle').textContent='Loading...';document.getElementById('dayDetail').innerHTML='<div class="overlay-loading">Loading...</div>';fetch('https://api.weather.gov/gridpoints/CTP/128,27/forecast',{headers:{'Accept':'application/geo+json'}}).then(function(r){return r.json();}).then(function(d){var periods=d.properties.periods.filter(function(p){return p.name.substring(0,3)===day;});if(!periods.length){document.getElementById('dayDetail').innerHTML='<div class="overlay-loading">No data available</div>';return;}document.getElementById('dayOverlayTitle').textContent=periods[0].name.split(' ')[0]+' Forecast';var h=periods.map(function(p){var precip=(p.probabilityOfPrecipitation&&p.probabilityOfPrecipitation.value!=null)?p.probabilityOfPrecipitation.value+'%':'--';var humidity=(p.relativeHumidity&&p.relativeHumidity.value!=null)?Math.round(p.relativeHumidity.value)+'%':'--';return'<div class="day-period"><div class="dp-name">'+p.name+'</div><div class="dp-temp">'+p.temperature+'\xB0F</div><div class="dp-detail">'+p.detailedForecast+'</div><div class="dp-stats"><span>Wind: '+p.windDirection+' '+p.windSpeed+'</span><span>Precip: '+precip+'</span><span>Humidity: '+humidity+'</span></div></div>';}).join('');document.getElementById('dayDetail').innerHTML=h;}).catch(function(){document.getElementById('dayDetail').innerHTML='<div class="overlay-loading">Failed to load day detail</div>';});}
  function closeDayDetail(){document.getElementById('dayOverlay').className='overlay';}
  function chickenOn(){fetch('/chicken-on').then(function(){setTimeout(updateChicken,600);});}
  function chickenOff(){fetch('/chicken-off').then(function(){setTimeout(updateChicken,600);});}
  function updateChicken(){fetch('/chicken-status').then(function(r){return r.json();}).then(function(d){var onBtn=document.getElementById('ledOnBtn'),offBtn=document.getElementById('ledOffBtn'),tag=document.getElementById('chiOffline');if(!d.ok){onBtn.classList.remove('active');offBtn.classList.remove('active');tag.style.display='inline';}else{tag.style.display='none';onBtn.classList.toggle('active',d.on);offBtn.classList.toggle('active',!d.on);}}).catch(function(){document.getElementById('ledOnBtn').classList.remove('active');document.getElementById('ledOffBtn').classList.remove('active');document.getElementById('chiOffline').style.display='inline';});}
  setInterval(updateChicken,5000);updateChicken();
  var kidsData=[];
  var kidsWeekStart='';
  function mondayKey(d){var day=d.getDay();var diff=(day===0?-6:1-day);var m=new Date(d);m.setDate(d.getDate()+diff);return m.getFullYear()+'-'+String(m.getMonth()+1).padStart(2,'0')+'-'+String(m.getDate()).padStart(2,'0');}
  function saveKids(){fetch('http://192.168.12.136:8181/kids/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({kids:kidsData,weekStart:kidsWeekStart})}).catch(function(){});}
  function loadKids(){fetch('http://192.168.12.136:8181/kids').then(function(r){return r.json()}).then(function(d){kidsData=d.kids||[];var wk=mondayKey(new Date());if(d.weekStart!==wk){kidsData.forEach(function(k){k.choreDone={};});kidsWeekStart=wk;saveKids();}else{kidsWeekStart=wk;}}).catch(function(){});}
  loadKids();setInterval(loadKids,120000);
  function loadCalendar(){fetch('http://192.168.12.136:8182/calendar').then(function(r){return r.json()}).then(function(d){var el=document.getElementById('calEvents');if(!d.events||!d.events.length){el.innerHTML='<div class="event-row placeholder"><span class="event-title">No events today</span></div>';return;}var h='';for(var i=0;i<d.events.length;i++){var ev=d.events[i];h+='<div class="event-row"><span class="event-time">'+ev.time+'</span><span class="event-title">'+ev.title+'</span></div>';}el.innerHTML=h;}).catch(function(){document.getElementById('calEvents').innerHTML='<div class="event-row placeholder"><span class="event-title">Calendar offline</span></div>';});}
  loadCalendar();setInterval(loadCalendar,300000);
  function toggleChore(ki,ci,el){var k=kidsData[ki];if(!k)return;var chore=k.chores[ci];if(!k.choreDone)k.choreDone={};k.choreDone[chore]=el.checked;saveKids();}
  function showKid(i){var k=kidsData[i]||{name:'---',chores:[],schedule:''};document.getElementById('kidOverlayName').textContent=k.name;var ch=(k.chores&&k.chores.length)?k.chores.map(function(c,ci){var done=k.choreDone&&k.choreDone[c];return'<label class="chore-item'+(done?' chore-done':'')+'"><input type="checkbox" class="chore-check"'+(done?' checked':'')+' onchange="toggleChore('+i+','+ci+',this)"><span>'+c+'</span></label>';}).join(''):'<div class="overlay-loading">No chores listed yet &mdash; tap Edit to add some.</div>';var sc=k.schedule?'<div class="schedule-text">'+k.schedule.split('\n').map(function(l){return l.trim()?'<p>'+l+'</p>':''}).join('')+'</div>':'<div class="overlay-loading">No schedule listed yet &mdash; tap Edit to add one.</div>';document.getElementById('kidDetail').innerHTML='<div class="kid-section"><div class="kid-section-title">Daily Chores</div>'+ch+'</div><div class="kid-section"><div class="kid-section-title">Work Schedule</div>'+sc+'</div>';document.getElementById('kidOverlay').className='overlay open';}
  function closeKid(){document.getElementById('kidOverlay').className='overlay';}
  function goFullscreen(){var el=document.documentElement;if(el.requestFullscreen)el.requestFullscreen();else if(el.webkitRequestFullscreen)el.webkitRequestFullscreen();}
  document.addEventListener('fullscreenchange',function(){document.getElementById('fsBtn').style.display=document.fullscreenElement?'none':'inline-block';});
  document.addEventListener('webkitfullscreenchange',function(){document.getElementById('fsBtn').style.display=document.webkitFullscreenElement?'none':'inline-block';});
  </script>
</body>
</html>
)rawliteral";

const char otaPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>MILTONHAUS OTA Update</title>
  <link href="https://fonts.googleapis.com/css2?family=Comfortaa:wght@400;700&display=swap" rel="stylesheet">
  <style>
    body { font-family: 'Comfortaa', sans-serif; background: #d2c6a5; color: #3b3225; padding: 40px; text-align: center; }
    h2 { color: #8b5e3c; margin-bottom: 20px; }
    input[type=file] { margin: 20px; }
    input[type=submit] { padding: 12px 24px; background: #8b5e3c; color: #fff; border: none; border-radius: 8px; cursor: pointer; font-family: inherit; font-size: 1em; }
    input[type=submit]:hover { background: #6d4a2e; }
    a { color: #2e7d5b; }
  </style>
</head>
<body>
  <h2>MILTONHAUS OTA Update</h2>
  <p>Select a compiled .bin file to upload new firmware.</p>
  <form method="POST" action="/update" enctype="multipart/form-data">
    <input type="file" name="update" accept=".bin"><br>
    <input type="submit" value="Upload Firmware">
  </form>
  <p style="margin-top:30px;"><a href="/">Back to Dashboard</a></p>
</body>
</html>
)rawliteral";

void handleRoot() {
  size_t totalLen = strlen(page);
  server.setContentLength(totalLen);
  server.send(200, "text/html", "");
  const size_t chunkSize = 1024;
  size_t sent = 0;
  while (sent < totalLen) {
    size_t len = totalLen - sent;
    if (len > chunkSize) len = chunkSize;
    server.sendContent(page + sent, len);
    sent += len;
  }
}

void handleData() {
  static char buf[4096];
  int pos = 0;
  if (xSemaphoreTake(wxMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    pos = snprintf(buf, sizeof(buf),
      "{\"tempC\":%.1f,\"tempF\":%.1f,\"dhtH\":%.1f,\"sensor\":%s,"
      "\"oTemp\":\"%s\",\"oHigh\":\"%s\",\"oLow\":\"%s\","
      "\"oHum\":\"%s\",\"oWind\":\"%s\",\"oDesc\":\"%s\","
      "\"sunrise\":\"%s\",\"sunset\":\"%s\",\"forecast\":[",
      tempC, tempF, dhtHumidity, sensorConnected ? "true" : "false",
      wx.oTemp.c_str(), wx.oHigh.c_str(), wx.oLow.c_str(),
      wx.oHum.c_str(), wx.oWind.c_str(), wx.desc.c_str(),
      wx.sunrise.c_str(), wx.sunset.c_str());
    for (int i = 0; i < FORECAST_DAYS; i++) {
      pos += snprintf(buf + pos, sizeof(buf) - pos, "%s{\"day\":\"%s\",\"desc\":\"%s\",\"hi\":\"%s\",\"lo\":\"%s\"}",
        i > 0 ? "," : "", wx.fcDay[i].c_str(), wx.fcDesc[i].c_str(), wx.fcHigh[i].c_str(), wx.fcLow[i].c_str());
    }
    snprintf(buf + pos, sizeof(buf) - pos, "]}");
    xSemaphoreGive(wxMutex);
  } else {
    snprintf(buf, sizeof(buf), "{\"error\":\"busy\"}");
  }
  server.send(200, "application/json", buf);
}

String chickenGet(const String& path) {
  HTTPClient http;
  http.begin("http://192.168.12.241" + path);
  http.setConnectTimeout(1500);
  http.setTimeout(1500);
  int code = http.GET();
  String body = "";
  if (code == 200) body = http.getString();
  http.end();
  return body;
}

void handleChickenStatus() {
  String raw = chickenGet("/status");
  int lp = raw.indexOf("leds:");
  String ledsLine = lp >= 0 ? raw.substring(lp, raw.indexOf('\n', lp)) : "";
  bool on = ledsLine.indexOf("off") < 0 && ledsLine.indexOf("on") >= 0;
  bool ok = raw.length() > 0;
  char buf[48];
  snprintf(buf, sizeof(buf), "{\"on\":%s,\"ok\":%s}", on ? "true" : "false", ok ? "true" : "false");
  server.send(200, "application/json", buf);
}

void handleChickenOn()  { chickenGet("/leds-on");  server.send(200, "text/plain", "ok"); }
void handleChickenOff() { chickenGet("/leds-off"); server.send(200, "text/plain", "ok"); }

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== MILTONHAUS Weather Station ===");

  wxMutex = xSemaphoreCreateMutex();
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    IPAddress staticIP(192, 168, 12, 240);
    IPAddress gateway(192, 168, 12, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(192, 168, 12, 1);
    WiFi.config(staticIP, gateway, subnet, dns);
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.printf("RSSI: %d dBm | TX Power: max\n", WiFi.RSSI());

    setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    tzset();
    configTime(0, 0, ntpServer);
    Serial.println("NTP time sync started (Eastern)");
    struct tm ti;
    for (int i = 0; i < 10; i++) {
      if (getLocalTime(&ti, 1000)) {
        Serial.printf("NTP synced: %02d:%02d:%02d\n", ti.tm_hour, ti.tm_min, ti.tm_sec);
        break;
      }
      delay(500);
    }

  } else {
    Serial.println("\nWiFi FAILED - will retry in loop");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/chicken-status", handleChickenStatus);
  server.on("/chicken-on", handleChickenOn);
  server.on("/chicken-off", handleChickenOff);

  server.on("/update", HTTP_GET, []() {
    size_t totalLen = strlen(otaPage);
    server.setContentLength(totalLen);
    server.send(200, "text/html", "");
    const size_t chunkSize = 1024;
    size_t sent = 0;
    while (sent < totalLen) {
      size_t len = totalLen - sent;
      if (len > chunkSize) len = chunkSize;
      server.sendContent(otaPage + sent, len);
      sent += len;
    }
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", Update.hasError() ? "Update FAILED" : "Update OK - Rebooting...");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      otaInProgress = true;
      Serial.printf("OTA update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      esp_task_wdt_reset();  // upload can run >30s; keep the dog fed
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("OTA success: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();
  Serial.println("Web server started");

  xTaskCreatePinnedToCore(weatherTask, "weather", 10240, NULL, 1, NULL, 0);
  Serial.println("Weather fetch task started on core 0");

  // Hardware Task Watchdog: if loop() ever hangs (e.g. a TLS handshake that
  // never returns), the chip resets itself instead of going unreachable until
  // a manual power-cycle. This is the real fix for the "frozen, LED on, won't
  // respond to ping" lockups. The core may already have inited the TWDT for
  // idle tasks, so retune via reconfigure if init reports it's already up.
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WDT_TIMEOUT_S * 1000,
    .idle_core_mask = 0,
    .trigger_panic = true,
  };
  if (esp_task_wdt_init(&wdtConfig) == ESP_ERR_INVALID_STATE) {
    esp_task_wdt_reconfigure(&wdtConfig);
  }
  esp_task_wdt_add(NULL);  // watch this (loop) task
  Serial.printf("Hardware watchdog armed (%ds)\n", WDT_TIMEOUT_S);
}

void loop() {
  esp_task_wdt_reset();  // pet the hardware watchdog every pass
  server.handleClient();


  if (millis() - lastSensorRead > 2000) {
    readSensors();
    lastSensorRead = millis();
  }

  bool wifiDead = (WiFi.status() != WL_CONNECTED);
  if (wifiDead && millis() - lastWifiCheck > 30000) {
    lastWifiCheck = millis();
    Serial.printf("WiFi lost (status=%d RSSI=%d) - reconnecting...\n", WiFi.status(), WiFi.RSSI());
    WiFi.disconnect(true);
    delay(500);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      esp_task_wdt_reset();  // keep feeding during the blocking reconnect
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      IPAddress staticIP(192, 168, 12, 240);
      IPAddress gateway(192, 168, 12, 1);
      IPAddress subnet(255, 255, 255, 0);
      IPAddress dns(192, 168, 12, 1);
      WiFi.config(staticIP, gateway, subnet, dns);
      Serial.print("WiFi reconnected! IP: ");
      Serial.println(WiFi.localIP());
      wifiFailCount = 0;
    } else {
      wifiFailCount++;
      Serial.printf("WiFi reconnect failed (%d)\n", wifiFailCount);
    }
  }

  // Heap watchdog: if we drop critically low, reboot cleanly before a
  // fragmented-alloc failure hard-crashes the web server.
  if (!otaInProgress && ESP.getFreeHeap() < 10000) {
    Serial.printf("WATCHDOG: heap critical (%d bytes) - rebooting\n", ESP.getFreeHeap());
    delay(100);
    ESP.restart();
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo) && timeinfo.tm_hour == 2 && timeinfo.tm_min == 0 && timeinfo.tm_sec < 3) {
    Serial.println("Scheduled daily reboot");
    delay(100);
    ESP.restart();
  }
}
