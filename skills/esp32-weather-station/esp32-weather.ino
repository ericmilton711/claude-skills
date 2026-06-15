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
      font-family: 'Comfortaa', sans-serif; background: #d2c6a5; color: #3b3225;
      height: 100dvh; display: flex; flex-direction: column; overflow: hidden;
    }
    /* ---- Top bar ---- */
    .topbar { flex: 0 0 auto; background: linear-gradient(90deg, #8b5e3c, #6d4a2e);
      padding: clamp(6px,1.3vh,12px) clamp(12px,2vw,20px); display: flex; justify-content: space-between; align-items: center; }
    .tb-title { font-weight: 700; font-size: clamp(0.95rem,2.4vh,1.25rem); letter-spacing: 1px; color: #fff; }
    .tb-clock { color: #f0e6d2; text-align: right; font-size: clamp(0.85rem,2vh,1.1rem); line-height: 1.15; }
    .tb-clock .t { font-weight: 700; }
    .tb-clock .d { font-size: 0.72em; color: #d8c9a8; display: block; }
    .fs-btn { background: rgba(255,255,255,0.15); border: none; color: #f0e6d2; border-radius: 8px; padding: 4px 8px; font-size: 1.2rem; cursor: pointer; margin-left: 12px; line-height: 1; }
    .fs-btn:active { background: rgba(255,255,255,0.3); }
    :fullscreen .fs-btn, :-webkit-full-screen .fs-btn { display: none; }
    /* ---- Layout: fills the viewport, never scrolls ---- */
    .wrap { flex: 1 1 auto; min-height: 0; padding: clamp(8px,1.6vh,18px) clamp(10px,2vw,20px); display: flex; }
    .grid { flex: 1; min-height: 0; width: 100%; max-width: 1200px; margin: 0 auto;
      display: grid; gap: clamp(8px,1.5vh,16px);
      grid-template-columns: 2fr 1fr;
      grid-template-rows: 1.3fr 1fr 0.72fr 0.9fr;
      grid-template-areas: "main gauge" "fc fc" "stats stats" "scores scores"; }
    .weather-main { grid-area: main; }
    .gauge-card { grid-area: gauge; }
    .forecast-card { grid-area: fc; }
    .stats { grid-area: stats; }
    .card { background: #e8dcc8; border: 1px solid #c4b494; border-radius: 16px;
      padding: clamp(10px,1.8vh,18px) clamp(12px,1.6vw,18px); min-height: 0; overflow: hidden; }
    .section { font-size: clamp(0.62rem,1.4vh,0.78rem); color: #8b5e3c; text-transform: uppercase; letter-spacing: 2px; font-weight: 700; }
    /* ---- Weather main card ---- */
    .weather-main { display: flex; flex-direction: column; }
    .wm-head { display: flex; justify-content: space-between; align-items: center; gap: 10px; margin-bottom: clamp(4px,1vh,12px); }
    .hourly-btn { background: linear-gradient(135deg, #ff9d2e, #e8590c); color: #fff; border: none; border-radius: 30px;
      padding: clamp(8px,1.4vh,16px) clamp(16px,2.4vw,40px); font-family: inherit; font-size: clamp(0.95rem,2.1vh,1.4rem);
      font-weight: 700; cursor: pointer; box-shadow: 0 4px 12px rgba(180,80,10,0.4); white-space: nowrap; }
    .hourly-btn:active { transform: scale(0.96); }
    .wm-top { flex: 1; display: flex; flex-direction: column; align-items: center; justify-content: center; text-align: center; }
    .wm-cond-row { display: flex; align-items: center; justify-content: center; gap: 12px; }
    .wm-icon { font-size: clamp(1.6rem, min(5vh,7vw), 2.8rem); line-height: 1; }
    .wm-cond { font-size: clamp(1.1rem, min(3.4vh,4.6vw), 1.9rem); font-weight: 700; }
    .wm-temp { font-size: clamp(2.4rem, min(13vh,17vw), 6.2rem); font-weight: 700; line-height: 1; color: #3b3225; margin: clamp(2px,0.6vh,8px) 0; }
    .wm-temp .u { font-size: 0.3em; color: #7a6f5f; vertical-align: super; }
    .wm-hl { font-size: clamp(0.85rem, min(2.4vh,3.2vw), 1.25rem); color: #7a6f5f; }
    /* ---- 6-day forecast ---- */
    .forecast-card { display: flex; flex-direction: column; }
    .forecast-card .section { margin-bottom: clamp(4px,1vh,10px); }
    .fc-strip { flex: 1; display: flex; gap: clamp(4px,0.8vw,8px); }
    .fc-item { flex: 1; min-width: 0; display: flex; flex-direction: column; align-items: center; justify-content: center;
      background: #ddcdb2; border-radius: 12px; padding: clamp(4px,1vh,12px) 2px; }
    .fc-day { font-size: clamp(0.62rem, min(1.7vh,2.4vw), 0.9rem); color: #7a6f5f; font-weight: 700; margin-bottom: clamp(2px,0.6vh,8px); }
    .fc-icon { font-size: clamp(1.1rem, min(3vh,4.2vw), 1.9rem); margin-bottom: clamp(2px,0.6vh,8px); }
    .fc-hi { font-weight: 700; color: #e81e00; font-size: clamp(0.78rem, min(2vh,2.8vw), 1.15rem); }
    .fc-lo { font-size: clamp(0.68rem, min(1.7vh,2.4vw), 0.95rem); color: #7a6f5f; }
    /* ---- Indoor gauge ---- */
    .gauge-card { display: flex; flex-direction: column; align-items: center; justify-content: center; gap: clamp(4px,1vh,10px); }
    .gauge-title { align-self: flex-start; font-size: clamp(0.62rem,1.4vh,0.78rem); color: #8b5e3c; text-transform: uppercase; letter-spacing: 2px; font-weight: 700; }
    .gauge { position: relative; width: var(--g); height: var(--g); --g: clamp(96px, min(30vh,40vw), 200px); }
    .gauge-ring {
      width: 100%; height: 100%; border-radius: 50%;
      background: conic-gradient(from 225deg,
        #e8a000 0deg, #e81e00 var(--deg, 0deg),
        #cdbf9f var(--deg, 0deg), #cdbf9f 270deg,
        transparent 270deg);
      -webkit-mask: radial-gradient(transparent 62%, #000 63%);
      mask: radial-gradient(transparent 62%, #000 63%); }
    .gauge-center { position: absolute; inset: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; }
    .gauge-temp { font-size: clamp(1.4rem, min(7vh,9vw), 2.8rem); font-weight: 700; color: #3b3225; line-height: 1.05; }
    .gauge-temp .u { font-size: 0.4em; color: #7a6f5f; vertical-align: super; }
    .gauge-sub { font-size: clamp(0.7rem, min(1.8vh,2.4vw), 0.95rem); color: #7a6f5f; }
    .gauge-hum { font-size: clamp(0.72rem, min(1.9vh,2.6vw), 1rem); color: #00b35a; }
    .status { margin-top: clamp(2px,0.8vh,8px); }
    .pill { display: inline-block; padding: clamp(3px,0.7vh,6px) clamp(10px,1.4vw,16px); border-radius: 20px; font-size: clamp(0.66rem,1.5vh,0.85rem); color: #fff; background: #a0522d; }
    .pill.ok { background: #2e7d5b; }
    /* ---- Stats strip ---- */
    .stats { display: grid; grid-template-columns: repeat(4, 1fr); gap: clamp(6px,1vw,12px); align-items: center; }
    .stat { text-align: center; }
    .stat .slabel { font-size: clamp(0.6rem, min(1.5vh,2.2vw), 0.78rem); color: #7a6f5f; text-transform: uppercase; letter-spacing: 1px; }
    .stat .sval { font-size: clamp(1rem, min(3.4vh,4.4vw), 1.8rem); font-weight: 700; margin: clamp(1px,0.4vh,4px) 0; line-height: 1.1; }
    .stat .sval.time { font-size: clamp(0.85rem, min(2.6vh,3.2vw), 1.35rem); }
    .stat .sunit { font-size: clamp(0.5rem,1.1vh,0.62rem); color: #7a6f5f; }
    .hum { color: #00b35a; } .wind { color: #0090cc; } .sun { color: #e8a000; }
    /* ---- World Cup scores strip ---- */
    .scores-strip { grid-area: scores; display: flex; flex-direction: column; }
    .scores-hdr { display: flex; justify-content: space-between; align-items: center; margin-bottom: clamp(3px,0.6vh,7px); }
    .scores-btn { background: linear-gradient(135deg, #2e7d5b, #1a5540); color: #fff; border: none; border-radius: 30px; padding: clamp(5px,1vh,10px) clamp(12px,1.8vw,24px); font-family: inherit; font-size: clamp(0.82rem,1.7vh,1.1rem); font-weight: 700; cursor: pointer; box-shadow: 0 3px 8px rgba(30,80,50,0.4); white-space: nowrap; }
    .scores-btn:active { transform: scale(0.96); }
    .matches-row { flex: 1; display: flex; gap: clamp(8px,1.2vw,16px); align-items: center; overflow: hidden; }
    .match-chip { background: #ddcdb2; border-radius: 14px; padding: clamp(6px,1.2vh,14px) clamp(12px,1.8vw,22px); display: flex; align-items: center; gap: 8px; white-space: nowrap; font-size: clamp(1rem,2.2vh,1.4rem); flex-shrink: 0; }
    .match-score { font-weight: 700; color: #3b3225; }
    .match-teams { color: #5a5040; }
    .match-ft { font-size: 0.82em; color: #8b5e3c; }
    .match-live { font-size: 0.82em; color: #e81e00; font-weight: 700; }
    .match-time { font-size: 0.82em; color: #7a6f5f; }
    .no-matches { color: #7a6f5f; font-style: italic; font-size: clamp(0.9rem,2vh,1.2rem); }
    /* ---- Scores overlay ---- */
    .scores-overlay { display: none; position: fixed; inset: 0; background: #d2c6a5; z-index: 100; overflow-y: auto; padding: 18px; }
    .scores-overlay.open { display: block; }
    .sc-ov-hdr { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; max-width: 1200px; margin-left: auto; margin-right: auto; }
    .sc-ov-hdr h2 { color: #8b5e3c; font-size: clamp(1rem,2.4vh,1.3rem); }
    .sc-ov-body { max-width: 1200px; margin: 0 auto; }
    .sc-day-group { margin-bottom: 14px; }
    .sc-day-label { font-size: 0.75rem; color: #8b5e3c; text-transform: uppercase; letter-spacing: 2px; font-weight: 700; margin-bottom: 6px; }
    .sc-match { background: #e8dcc8; border: 1px solid #c4b494; border-radius: 12px; padding: 10px 16px; margin-bottom: 6px; display: grid; grid-template-columns: 1fr auto 1fr; align-items: center; gap: 12px; }
    .sc-team { font-size: 1em; }
    .sc-team.home { text-align: right; }
    .sc-team.away { text-align: left; }
    .sc-mid { text-align: center; min-width: 80px; }
    .sc-score { font-size: 1.3em; font-weight: 700; color: #3b3225; display: block; }
    .sc-score.live { color: #e81e00; }
    .sc-st { font-size: 0.75em; color: #8b5e3c; display: block; }
    .sc-st.live { color: #e81e00; }
    .sc-kick { font-size: 0.88em; color: #7a6f5f; }
    /* ---- Footer ---- */
    .footer { flex: 0 0 auto; text-align: center; padding: clamp(3px,0.8vh,8px) 0; color: #9a8d7a; font-size: clamp(0.6rem,1.3vh,0.78rem); }
    .footer a { color: #8b5e3c; }
    /* ---- Narrow phones: give the gauge a bit more room ---- */
    @media (max-width: 460px) {
      .grid { grid-template-columns: 3fr 2fr; gap: 8px; }
      .tb-title { letter-spacing: 0; }
    }
    /* ---- Hourly overlay (its own scroll) ---- */
    .overlay { display: none; position: fixed; inset: 0; background: #d2c6a5; z-index: 100; overflow-y: auto; padding: 18px; }
    .overlay.open { display: block; }
    .overlay-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; max-width: 1200px; margin-left: auto; margin-right: auto; }
    .overlay-header h2 { color: #8b5e3c; font-size: clamp(1rem,2.4vh,1.3rem); }
    .close-btn { background: linear-gradient(135deg, #ff9d2e, #e8590c); color: #fff; border: none; border-radius: 30px; padding: clamp(10px,1.6vh,18px) clamp(24px,3vw,48px); font-family: inherit; font-size: clamp(1rem,2.2vh,1.5rem); font-weight: 700; cursor: pointer; box-shadow: 0 4px 12px rgba(180,80,10,0.4); }
    .close-btn:active { transform: scale(0.96); }
    .overlay-body { max-width: 1200px; margin: 0 auto; }
    .hourly-row { background: #e8dcc8; border: 1px solid #c4b494; border-radius: 12px; padding: 11px 16px; margin-bottom: 7px; display: flex; justify-content: space-between; align-items: center; font-size: 1em; }
    .hourly-row .hr-time { font-weight: 700; min-width: 80px; }
    .hourly-row .hr-desc { color: #6b6050; font-size: 0.88em; flex: 1; text-align: center; }
    .hourly-row .hr-temp { color: #e81e00; font-weight: 700; }
    .hourly-row .hr-wind { color: #0090cc; font-size: 0.88em; min-width: 70px; text-align: right; }
    .overlay-loading { text-align: center; color: #7a6f5f; padding: 40px; }
  </style>
</head>
<body>
  <div class="topbar">
    <div class="tb-title">&#127968; MILTONHAUS Weather</div>
    <div style="display:flex;align-items:center;">
      <div class="tb-clock"><span class="t" id="clock">--:--</span><span class="d" id="date">Loading...</span></div>
      <button class="fs-btn" id="fsBtn" onclick="goFullscreen()" title="Fullscreen">&#x26F6;</button>
    </div>
  </div>
  <div class="wrap">
    <div class="grid">
      <div class="card weather-main">
        <div class="wm-head">
          <div class="section">Willow Street, PA</div>
          <button class="hourly-btn" onclick="showHourly()">&#9201; Hourly &raquo;</button>
        </div>
        <div class="wm-top">
          <div class="wm-cond-row"><span class="wm-icon" id="oIcon">&#127780;&#65039;</span><span class="wm-cond" id="oCond">Loading...</span></div>
          <div class="wm-temp"><span id="oTemp">--</span><span class="u">&deg;F</span></div>
          <div class="wm-hl" id="oHL">H: -- / L: --</div>
        </div>
      </div>
      <div class="card gauge-card">
        <div class="gauge-title">Indoor</div>
        <div class="gauge">
          <div class="gauge-ring" id="gaugeRing"></div>
          <div class="gauge-center">
            <div class="gauge-temp"><span id="tempF">--</span><span class="u">&deg;F</span></div>
            <div class="gauge-sub"><span id="tempC">--</span>&deg;C</div>
            <div class="gauge-hum"><span id="dhtH">--</span>% humidity</div>
          </div>
        </div>
        <div class="status"><span class="pill" id="status">Loading...</span></div>
      </div>
      <div class="card forecast-card">
        <div class="section">6-Day Forecast</div>
        <div class="fc-strip" id="forecast"></div>
      </div>
      <div class="card stats">
        <div class="stat"><div class="slabel">Humidity</div><div class="sval hum"><span id="oHum">--</span></div><div class="sunit">%</div></div>
        <div class="stat"><div class="slabel">Wind</div><div class="sval wind"><span id="oWind">--</span></div><div class="sunit">mph</div></div>
        <div class="stat"><div class="slabel">Sunrise</div><div class="sval sun time" id="sunrise">--:--</div></div>
        <div class="stat"><div class="slabel">Sunset</div><div class="sval sun time" id="sunset">--:--</div></div>
      </div>
      <div class="card scores-strip">
        <div class="scores-hdr">
          <div class="section">&#9917; World Cup 2026</div>
          <button class="scores-btn" onclick="showScores()">All Matches &raquo;</button>
        </div>
        <div class="matches-row" id="todayMatches"><span class="no-matches">Loading scores...</span></div>
      </div>
    </div>
  </div>
  <div class="footer">NWS Weather every 10 min &bull; <a href="/update">OTA Update</a></div>
  <div class="scores-overlay" id="scoresOverlay">
    <div class="sc-ov-hdr">
      <h2>&#9917; FIFA World Cup 2026</h2>
      <button class="close-btn" onclick="closeScores()">Back</button>
    </div>
    <div class="sc-ov-body" id="scoresList"><div class="overlay-loading">Loading...</div></div>
  </div>
  <div class="overlay" id="hourlyOverlay">
    <div class="overlay-header">
      <h2>Hourly Forecast &mdash; Next 24 Hours</h2>
      <button class="close-btn" onclick="closeHourly()">Back</button>
    </div>
    <div class="overlay-body" id="hourlyList"><div class="overlay-loading">Loading...</div></div>
  </div>
  <script>
    void(function(){var c=document.getElementById('clock'),d=document.getElementById('date');setInterval(function(){var n=new Date();c.textContent=n.toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:true,timeZone:'America/New_York'});d.textContent=n.toLocaleDateString('en-US',{weekday:'short',month:'short',day:'numeric',timeZone:'America/New_York'});},1000)}());
    void(function(){var u=function(){fetch('/data').then(function(r){return r.json()}).then(function(d){var s=document.getElementById('status');if(d.sensor){s.textContent='Sensor Online';s.className='pill ok';document.getElementById('tempF').textContent=d.tempF.toFixed(1);document.getElementById('tempC').textContent=d.tempC.toFixed(1);document.getElementById('dhtH').textContent=d.dhtH.toFixed(0);var fr=Math.max(0,Math.min(1,(d.tempF-40)/50));document.getElementById('gaugeRing').style.setProperty('--deg',(fr*270)+'deg')}else{s.textContent='Sensor Not Connected';s.className='pill';document.getElementById('tempF').textContent='--';document.getElementById('tempC').textContent='--';document.getElementById('dhtH').textContent='--';document.getElementById('gaugeRing').style.setProperty('--deg','0deg')}document.getElementById('oTemp').textContent=d.oTemp;document.getElementById('oHL').textContent='H: '+d.oHigh+'° / L: '+d.oLow+'°';document.getElementById('oHum').textContent=d.oHum;document.getElementById('oWind').textContent=d.oWind;var parts=d.oDesc.split(' ');document.getElementById('oIcon').innerHTML=parts.shift();document.getElementById('oCond').textContent=parts.join(' ');document.getElementById('sunrise').textContent=d.sunrise;document.getElementById('sunset').textContent=d.sunset;var fc=document.getElementById('forecast');var h='';for(var i=0;i<d.forecast.length;i++){var f=d.forecast[i];var ic=f.desc.split(' ')[0];h+='<div class="fc-item"><div class="fc-day">'+f.day+'</div><div class="fc-icon">'+ic+'</div><div class="fc-hi">'+f.hi+'°</div><div class="fc-lo">'+f.lo+'°</div></div>'}fc.innerHTML=h}).catch(function(){document.getElementById('status').textContent='Connection Lost';document.getElementById('status').className='pill'})};u();setInterval(u,5000)}());
  function descEmoji(d){d=d.toLowerCase();if(d.indexOf('thunder')>=0)return'⚡';if(d.indexOf('snow')>=0||d.indexOf('blizzard')>=0)return'\u{1F328}️';if(d.indexOf('rain')>=0||d.indexOf('shower')>=0||d.indexOf('drizzle')>=0)return'\u{1F327}️';if(d.indexOf('fog')>=0||d.indexOf('mist')>=0)return'\u{1F32B}️';if(d.indexOf('cloud')>=0||d.indexOf('overcast')>=0)return'⛅';if(d.indexOf('sunny')>=0||d.indexOf('clear')>=0)return'☀️';return'\u{1F324}️';}
  function showHourly(){document.getElementById('hourlyOverlay').className='overlay open';document.getElementById('hourlyList').innerHTML='<div class="overlay-loading">Loading...</div>';fetch('https://api.weather.gov/gridpoints/CTP/128,27/forecast/hourly',{headers:{'Accept':'application/geo+json'}}).then(function(r){return r.json()}).then(function(d){var p=d.properties.periods;var h='';var count=Math.min(p.length,24);for(var i=0;i<count;i++){var t=new Date(p[i].startTime);var hr=t.getHours();var ampm=hr>=12?'PM':'AM';if(hr===0)hr=12;else if(hr>12)hr-=12;var timeStr=hr+':00 '+ampm;var e=descEmoji(p[i].shortForecast);h+='<div class="hourly-row"><div class="hr-time">'+timeStr+'</div><div class="hr-desc">'+e+' '+p[i].shortForecast+'</div><div class="hr-temp">'+p[i].temperature+'°</div><div class="hr-wind">'+p[i].windSpeed+'</div></div>'}document.getElementById('hourlyList').innerHTML=h}).catch(function(){document.getElementById('hourlyList').innerHTML='<div class="overlay-loading">Failed to load hourly forecast</div>'});}
  function closeHourly(){document.getElementById('hourlyOverlay').className='overlay';}
  // ---- World Cup Scores ----
  var WC_URL='https://site.api.espn.com/apis/site/v2/sports/soccer/fifa.world/scoreboard';
  function parseMTC(e){var comp=e.competitions[0];var home=comp.competitors.find(function(c){return c.homeAway==='home';});var away=comp.competitors.find(function(c){return c.homeAway==='away';});return{home:home.team.abbreviation,away:away.team.abbreviation,homeFull:home.team.displayName,awayFull:away.team.displayName,homeScore:home.score||'',awayScore:away.score||'',state:e.status.type.state,detail:e.status.type.shortDetail||e.status.type.description,date:e.date};}
  function fetchTodayScores(){fetch(WC_URL).then(function(r){return r.json();}).then(function(d){var events=d.events||[];var strip=document.getElementById('todayMatches');if(!events.length){strip.innerHTML='<span class="no-matches">No matches today</span>';return;}strip.innerHTML=events.map(function(e){var m=parseMTC(e);var scoreStr,stHTML;if(m.state==='post'){scoreStr=m.homeScore+'-'+m.awayScore;stHTML='<span class="match-ft">FT</span>';}else if(m.state==='in'){scoreStr=m.homeScore+'-'+m.awayScore;stHTML='<span class="match-live">'+m.detail+'</span>';}else{scoreStr='vs';stHTML='<span class="match-time">'+new Date(m.date).toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:true,timeZone:'America/New_York'})+'</span>';}return'<div class="match-chip"><span class="match-teams">'+m.home+'</span><span class="match-score">'+scoreStr+'</span><span class="match-teams">'+m.away+'</span>'+stHTML+'</div>';}).join('');}).catch(function(){document.getElementById('todayMatches').innerHTML='<span class="no-matches">Scores unavailable</span>';});}
  function showScores(){document.getElementById('scoresOverlay').className='scores-overlay open';document.getElementById('scoresList').innerHTML='<div class="overlay-loading">Loading...</div>';var now=new Date();var s=new Date(now);s.setDate(now.getDate()-14);var en=new Date(now);en.setDate(now.getDate()+21);function fd(d){return d.toISOString().slice(0,10).replace(/-/g,'');}fetch(WC_URL+'?dates='+fd(s)+'-'+fd(en)+'&limit=200').then(function(r){return r.json();}).then(function(d){var events=d.events||[];if(!events.length){document.getElementById('scoresList').innerHTML='<div class="overlay-loading">No match data</div>';return;}var groups={};events.forEach(function(e){var day=e.date.slice(0,10);if(!groups[day])groups[day]=[];groups[day].push(e);});var days=Object.keys(groups).sort();var html=days.map(function(day){var label=new Date(day+'T12:00:00').toLocaleDateString('en-US',{weekday:'long',month:'long',day:'numeric'});var rows=groups[day].map(function(e){var m=parseMTC(e);var midHTML;if(m.state==='post'){midHTML='<span class="sc-score">'+m.homeScore+' - '+m.awayScore+'</span><span class="sc-st">Final</span>';}else if(m.state==='in'){midHTML='<span class="sc-score live">'+m.homeScore+' - '+m.awayScore+'</span><span class="sc-st live">'+m.detail+'</span>';}else{midHTML='<span class="sc-kick">'+new Date(m.date).toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:true,timeZone:'America/New_York'})+'</span>';}return'<div class="sc-match"><div class="sc-team home">'+m.homeFull+'</div><div class="sc-mid">'+midHTML+'</div><div class="sc-team away">'+m.awayFull+'</div></div>';}).join('');return'<div class="sc-day-group"><div class="sc-day-label">'+label+'</div>'+rows+'</div>';}).join('');document.getElementById('scoresList').innerHTML=html;}).catch(function(){document.getElementById('scoresList').innerHTML='<div class="overlay-loading">Failed to load match data</div>';});}
  function closeScores(){document.getElementById('scoresOverlay').className='scores-overlay';}
  fetchTodayScores();setInterval(fetchTodayScores,60000);
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
