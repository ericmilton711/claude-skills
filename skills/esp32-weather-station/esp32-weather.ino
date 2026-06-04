#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Update.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <time.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define FORECAST_DAYS 6

const char* ssid = "DIEMILTONHAUS";
const char* password = "wisdom22!!";
const char* ntpServer = "pool.ntp.org";

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

float tempC = 0, tempF = 0, dhtHumidity = 0;
bool sensorConnected = false;
unsigned long lastSensorRead = 0;
unsigned long lastWeatherFetch = 0;
unsigned long lastWifiCheck = 0;
int wifiFailCount = 0;

String outsideTemp = "--";
String outsideHigh = "--";
String outsideLow = "--";
String outsideHumidity = "--";
String windSpeed = "--";
String weatherDesc = "Loading...";
String sunrise = "--:--";
String sunset = "--:--";
String forecastDay[FORECAST_DAYS];
String forecastHigh[FORECAST_DAYS];
String forecastLow[FORECAST_DAYS];
String forecastDesc[FORECAST_DAYS];

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
  int code = http.GET();
  bool ok = false;
  if (code == 200) {
    String payload = http.getString();
    // Filtered parse: keep only the few fields we use so the JsonDocument
    // stays tiny (NWS payloads are 50-100KB; full parse exhausts heap).
    DeserializationError err = filter
      ? deserializeJson(doc, payload, DeserializationOption::Filter(*filter))
      : deserializeJson(doc, payload);
    if (!err) ok = true;
    else Serial.printf("NWS JSON error: %s\n", err.c_str());
  } else {
    Serial.printf("NWS fetch failed (%s): %d\n", url, code);
  }
  http.end();
  return ok;
}

void fetchCurrentObs() {
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
  outsideTemp = String(tempFVal, 1);

  float rh = props["relativeHumidity"]["value"].as<float>();
  outsideHumidity = String((int)rh);

  float windKmh = props["windSpeed"]["value"].as<float>();
  float windMph = windKmh * 0.621371;
  windSpeed = String(windMph, 1);

  const char* desc = props["textDescription"];
  if (desc) {
    weatherDesc = nwsDescToEmoji(String(desc)) + " " + String(desc);
  }

  Serial.println("NWS current observations updated");
}

void fetchForecast() {
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
      outsideHigh = String(temp);
      todayHighSet = true;
      continue;
    }
    if (!todayLowSet && !daytime) {
      outsideLow = String(temp);
      todayLowSet = true;
      if (!todayHighSet) {
        todayHighSet = true;
        outsideHigh = "--";
      }
      continue;
    }

    if (daytime && dayIdx < FORECAST_DAYS) {
      forecastDay[dayIdx] = String(name).substring(0, 3);
      forecastHigh[dayIdx] = String(temp);
      forecastDesc[dayIdx] = nwsDescToEmoji(String(shortFc)) + " " + String(shortFc);
    }
    if (!daytime && dayIdx < FORECAST_DAYS) {
      forecastLow[dayIdx] = String(temp);
      dayIdx++;
    }
  }

  Serial.println("NWS forecast updated");
}

void fetchSunriseSunset() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, "https://api.sunrise-sunset.org/json?lat=39.98&lng=-76.28&formatted=0");
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  int code = http.GET();
  if (code == 200) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
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

      sunrise = utcIsoToLocal(sr);
      sunset = utcIsoToLocal(ss);
      Serial.printf("Sunrise: %s  Sunset: %s\n", sunrise.c_str(), sunset.c_str());
    }
  } else {
    Serial.printf("Sunrise API failed: %d\n", code);
  }
  http.end();
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  fetchCurrentObs();
  fetchForecast();
  fetchSunriseSunset();
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
    body {
      font-family: 'Comfortaa', sans-serif;
      background: #d2c6a5;
      color: #3b3225;
      min-height: 100vh;
    }
    .topbar {
      background: linear-gradient(90deg, #8b5e3c, #6d4a2e);
      padding: 10px 18px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .tb-title { font-weight: 700; font-size: 1.15em; letter-spacing: 1px; color: #fff; }
    .tb-clock { font-size: 1em; color: #f0e6d2; text-align: right; }
    .tb-clock .t { font-weight: 700; }
    .tb-clock .d { font-size: 0.72em; color: #d8c9a8; display: block; }
    .wrap { padding: 18px; max-width: 1200px; margin: 0 auto; }
    .grid { display: grid; grid-template-columns: 1fr; gap: 16px; }
    @media (min-width: 760px) {
      .grid { grid-template-columns: 2fr 1fr; }
      .weather-main { grid-column: 1; }
      .gauge-card { grid-column: 2; }
      .forecast-card { grid-column: 1 / -1; }
      .stats { grid-column: 1 / -1; }
    }
    .card {
      background: #e8dcc8;
      border: 1px solid #c4b494;
      border-radius: 16px;
      padding: 18px;
    }
    .section { font-size: 0.78em; color: #8b5e3c; text-transform: uppercase; letter-spacing: 2px; margin-bottom: 12px; font-weight: 700; }
    /* Weather main card */
    .wm-head { display: flex; justify-content: space-between; align-items: center; margin-bottom: 14px; }
    .hourly-btn { background: linear-gradient(135deg, #ff9d2e, #e8590c); color: #fff; border: none; border-radius: 30px; padding: 18px 42px; font-family: inherit; font-size: 1.5em; font-weight: 700; cursor: pointer; box-shadow: 0 4px 12px rgba(180,80,10,0.4); }
    .hourly-btn:active { transform: scale(0.96); }
    .wm-top { text-align: center; padding: 6px 0; }
    .wm-cond-row { display: flex; align-items: center; justify-content: center; gap: 12px; margin-bottom: 6px; }
    .wm-icon { font-size: 2.6em; line-height: 1; }
    .wm-cond { font-size: 1.8em; font-weight: 700; }
    .wm-temp { font-size: 6em; font-weight: 700; line-height: 1; color: #3b3225; margin: 6px 0; }
    .wm-temp .u { font-size: 0.3em; color: #7a6f5f; vertical-align: super; }
    .wm-hl { font-size: 1.15em; color: #7a6f5f; margin-top: 4px; }
    /* 6-day forecast */
    .fc-strip { display: flex; gap: 8px; }
    .fc-item { flex: 1; text-align: center; background: #ddcdb2; border-radius: 12px; padding: 12px 6px; }
    .fc-day { font-size: 0.85em; color: #7a6f5f; margin-bottom: 8px; font-weight: 700; }
    .fc-icon { font-size: 1.9em; margin-bottom: 8px; }
    .fc-hi { font-weight: 700; color: #e81e00; font-size: 1.1em; }
    .fc-lo { font-size: 0.9em; color: #7a6f5f; }
    /* Indoor gauge */
    .gauge-card { display: flex; flex-direction: column; align-items: center; justify-content: center; }
    .gauge-title { align-self: flex-start; font-size: 0.78em; color: #8b5e3c; text-transform: uppercase; letter-spacing: 2px; margin-bottom: 10px; font-weight: 700; }
    .gauge { position: relative; width: 210px; height: 210px; }
    .gauge-ring {
      width: 100%; height: 100%; border-radius: 50%;
      background: conic-gradient(from 225deg,
        #e8a000 0deg, #e81e00 var(--deg, 0deg),
        #cdbf9f var(--deg, 0deg), #cdbf9f 270deg,
        transparent 270deg);
      -webkit-mask: radial-gradient(transparent 62%, #000 63%);
      mask: radial-gradient(transparent 62%, #000 63%);
      transition: --deg 0.6s ease;
    }
    .gauge-center { position: absolute; inset: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; }
    .gauge-temp { font-size: 2.8em; font-weight: 700; color: #3b3225; }
    .gauge-temp .u { font-size: 0.4em; color: #7a6f5f; vertical-align: super; }
    .gauge-sub { font-size: 0.85em; color: #7a6f5f; margin-top: 2px; }
    .gauge-hum { font-size: 0.95em; color: #00b35a; margin-top: 6px; }
    .status { margin-top: 14px; }
    .pill { display: inline-block; padding: 5px 16px; border-radius: 20px; font-size: 0.82em; color: #fff; background: #a0522d; }
    .pill.ok { background: #2e7d5b; }
    /* Stats strip */
    .stats { display: grid; grid-template-columns: repeat(4, 1fr); gap: 12px; }
    @media (max-width: 480px) { .stats { grid-template-columns: repeat(2, 1fr); } }
    .stat { text-align: center; }
    .stat .slabel { font-size: 0.75em; color: #7a6f5f; text-transform: uppercase; letter-spacing: 1px; }
    .stat .sval { font-size: 1.7em; font-weight: 700; margin: 4px 0; }
    .stat .sunit { font-size: 0.6em; color: #7a6f5f; }
    .hum { color: #00b35a; }
    .wind { color: #0090cc; }
    .sun { color: #e8a000; }
    .footer { text-align: center; margin-top: 20px; color: #9a8d7a; font-size: 0.78em; }
    .footer a { color: #8b5e3c; }
    /* Phone layout */
    @media (max-width: 600px) {
      .wrap { padding: 12px; }
      .grid { gap: 12px; }
      .card { padding: 14px; }
      .tb-title { font-size: 1em; }
      .wm-head { flex-direction: column; gap: 10px; align-items: stretch; }
      .hourly-btn { width: 100%; font-size: 1.2em; padding: 14px; }
      .wm-icon { font-size: 2em; }
      .wm-cond { font-size: 1.4em; }
      .wm-temp { font-size: 4em; }
      .wm-hl { font-size: 1em; }
      .fc-strip { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; }
      .fc-icon { font-size: 1.6em; }
      .gauge { width: 180px; height: 180px; }
      .gauge-temp { font-size: 2.3em; }
      .stats { grid-template-columns: repeat(2, 1fr); }
      .close-btn { font-size: 1.2em; padding: 14px 30px; }
      .overlay-header h2 { font-size: 1em; }
    }
    /* Hourly overlay */
    .overlay {
      display: none; position: fixed; inset: 0;
      background: #d2c6a5; z-index: 100; overflow-y: auto; padding: 18px;
    }
    .overlay.open { display: block; }
    .overlay-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; max-width: 1200px; margin-left: auto; margin-right: auto; }
    .overlay-header h2 { color: #8b5e3c; font-size: 1.2em; }
    .close-btn { background: linear-gradient(135deg, #ff9d2e, #e8590c); color: #fff; border: none; border-radius: 30px; padding: 18px 48px; font-family: inherit; font-size: 1.5em; font-weight: 700; cursor: pointer; box-shadow: 0 4px 12px rgba(180,80,10,0.4); }
    .close-btn:active { transform: scale(0.96); }
    .overlay-body { max-width: 1200px; margin: 0 auto; }
    .hourly-row {
      background: #e8dcc8; border: 1px solid #c4b494; border-radius: 12px;
      padding: 11px 16px; margin-bottom: 7px;
      display: flex; justify-content: space-between; align-items: center; font-size: 1em;
    }
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
    <div class="tb-clock"><span class="t" id="clock">--:--</span><span class="d" id="date">Loading...</span></div>
  </div>
  <div class="wrap">
    <div class="grid">
      <div class="card weather-main">
        <div class="wm-head">
          <div class="section" style="margin:0;">Willow Street, PA</div>
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
        <div class="stat">
          <div class="slabel">Humidity</div>
          <div class="sval hum"><span id="oHum">--</span></div>
          <div class="sunit">%</div>
        </div>
        <div class="stat">
          <div class="slabel">Wind</div>
          <div class="sval wind"><span id="oWind">--</span></div>
          <div class="sunit">mph</div>
        </div>
        <div class="stat">
          <div class="slabel">Sunrise</div>
          <div class="sval sun" style="font-size:1.2em;" id="sunrise">--:--</div>
        </div>
        <div class="stat">
          <div class="slabel">Sunset</div>
          <div class="sval sun" style="font-size:1.2em;" id="sunset">--:--</div>
        </div>
      </div>
    </div>
    <div class="footer">NWS Weather every 10 min &bull; <a href="/update">OTA Update</a></div>
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
  int pos = snprintf(buf, sizeof(buf),
    "{\"tempC\":%.1f,\"tempF\":%.1f,\"dhtH\":%.1f,\"sensor\":%s,"
    "\"oTemp\":\"%s\",\"oHigh\":\"%s\",\"oLow\":\"%s\","
    "\"oHum\":\"%s\",\"oWind\":\"%s\",\"oDesc\":\"%s\","
    "\"sunrise\":\"%s\",\"sunset\":\"%s\",\"forecast\":[",
    tempC, tempF, dhtHumidity, sensorConnected ? "true" : "false",
    outsideTemp.c_str(), outsideHigh.c_str(), outsideLow.c_str(),
    outsideHumidity.c_str(), windSpeed.c_str(), weatherDesc.c_str(),
    sunrise.c_str(), sunset.c_str());
  for (int i = 0; i < FORECAST_DAYS; i++) {
    pos += snprintf(buf + pos, sizeof(buf) - pos, "%s{\"day\":\"%s\",\"desc\":\"%s\",\"hi\":\"%s\",\"lo\":\"%s\"}",
      i > 0 ? "," : "", forecastDay[i].c_str(), forecastDesc[i].c_str(), forecastHigh[i].c_str(), forecastLow[i].c_str());
  }
  snprintf(buf + pos, sizeof(buf) - pos, "]}");
  server.send(200, "application/json", buf);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== MILTONHAUS Weather Station ===");

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

    fetchWeather();
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

}

void loop() {
  server.handleClient();


  if (millis() - lastSensorRead > 2000) {
    readSensors();
    lastSensorRead = millis();
  }

  if (millis() - lastWeatherFetch > 600000) {
    fetchWeather();
    lastWeatherFetch = millis();
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
      fetchWeather();
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
