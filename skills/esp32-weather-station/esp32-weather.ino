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
#define FORECAST_DAYS 7

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

bool nwsFetch(const char* url, JsonDocument& doc) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("User-Agent", "(miltonhaus-weather, ericmilton711@gmail.com)");
  http.addHeader("Accept", "application/geo+json");
  http.setConnectTimeout(10000);
  http.setTimeout(10000);
  int code = http.GET();
  bool ok = false;
  if (code == 200) {
    String payload = http.getString();
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) ok = true;
    else Serial.printf("NWS JSON error: %s\n", err.c_str());
  } else {
    Serial.printf("NWS fetch failed (%s): %d\n", url, code);
  }
  http.end();
  return ok;
}

void fetchCurrentObs() {
  JsonDocument doc;
  if (!nwsFetch("https://api.weather.gov/stations/KLNS/observations/latest", doc)) return;

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
  JsonDocument doc;
  if (!nwsFetch("https://api.weather.gov/gridpoints/CTP/128,27/forecast", doc)) return;

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
      padding: 20px 16px;
    }
    .container { max-width: 100%; margin: 0 auto; }
    @media (min-width: 768px) { .container { max-width: 70%; } }
    h1 { color: #8b5e3c; text-align: center; font-size: 1.3em; margin-bottom: 4px; }
    .clock { text-align: center; margin-bottom: 8px; }
    .clock .time { font-size: 2.2em; font-weight: bold; color: #3b3225; }
    .clock .date { font-size: 1em; color: #7a6f5f; margin-top: 2px; }
    .section { color: #8b5e3c; font-size: 0.75em; text-transform: uppercase; letter-spacing: 2px; margin: 14px 0 6px; }
    .row { display: flex; gap: 8px; margin-bottom: 8px; }
    .card {
      flex: 1;
      background: #e8dcc8;
      border-radius: 12px;
      padding: 12px;
      text-align: center;
      border: 1px solid #c4b494;
    }
    .card .label { font-size: 0.75em; color: #7a6f5f; text-transform: uppercase; letter-spacing: 1px; }
    .card .value { font-size: 1.8em; font-weight: 700; margin: 3px 0; -webkit-text-stroke: 1px currentColor; }
    .card .unit { font-size: 0.6em; color: #7a6f5f; }
    .card .sub { font-size: 0.75em; color: #6b6050; margin-top: 3px; }
    .big .value { font-size: 2.4em; }
    .temp { color: #e81e00; }
    .hum { color: #00b35a; }
    .wind { color: #0090cc; }
    .sun { color: #e8a000; }
    .conditions { color: #5a4e3c; }
    .forecast-row {
      background: #e8dcc8;
      border-radius: 12px;
      border: 1px solid #c4b494;
      padding: 12px 18px;
      margin-bottom: 6px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 1em;
    }
    .forecast-row .day { font-weight: bold; min-width: 44px; }
    .forecast-row .desc { color: #6b6050; font-size: 0.85em; flex: 1; text-align: center; }
    .forecast-row .hi { color: #e81e00; font-weight: 700; -webkit-text-stroke: 1px currentColor; }
    .forecast-row .lo { color: #7a6f5f; margin-left: 8px; }
    .status-pill { text-align: center; margin-bottom: 8px; }
    .pill {
      display: inline-block;
      padding: 5px 16px;
      border-radius: 20px;
      font-size: 0.85em;
      color: #fff;
      background: #a0522d;
    }
    .pill.ok { background: #2e7d5b; }
    .footer { text-align: center; margin-top: 20px; color: #9a8d7a; font-size: 0.75em; }
    .grid { display: block; }
    .clickable { cursor: pointer; }
    .clickable:active { transform: scale(0.97); }
    .overlay {
      display: none;
      position: fixed;
      top: 0; left: 0; right: 0; bottom: 0;
      background: #d2c6a5;
      z-index: 100;
      overflow-y: auto;
      padding: 20px 16px;
    }
    .overlay.open { display: block; }
    .overlay-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 12px;
    }
    .overlay-header h2 { color: #8b5e3c; font-size: 1.1em; margin: 0; }
    .close-btn {
      background: #8b5e3c;
      color: #fff;
      border: none;
      border-radius: 8px;
      padding: 8px 16px;
      font-family: inherit;
      font-size: 0.9em;
      cursor: pointer;
    }
    .hourly-row {
      background: #e8dcc8;
      border-radius: 12px;
      border: 1px solid #c4b494;
      padding: 10px 14px;
      margin-bottom: 6px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.95em;
    }
    .hourly-row .hr-time { font-weight: bold; min-width: 70px; }
    .hourly-row .hr-desc { color: #6b6050; font-size: 0.85em; flex: 1; text-align: center; }
    .hourly-row .hr-temp { color: #e81e00; font-weight: 700; -webkit-text-stroke: 1px currentColor; }
    .hourly-row .hr-wind { color: #0090cc; font-size: 0.85em; min-width: 60px; text-align: right; }
    .overlay-loading { text-align: center; color: #7a6f5f; padding: 40px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>MILTONHAUS Weather</h1>
    <div class="clock">
      <div class="time" id="clock">--:--</div>
      <div class="date" id="date">Loading...</div>
    </div>
    <div class="grid">
      <div>
        <div class="section">Willow Street, PA</div>
        <div class="row">
          <div class="card big">
            <div class="label">Temperature</div>
            <div class="value temp" id="oTemp">--</div>
            <div class="unit">&deg;F</div>
            <div class="sub" id="oHL">H: -- / L: --</div>
          </div>
          <div class="card clickable" id="condCard" onclick="showHourly()">
            <div class="label">Conditions</div>
            <div class="value conditions" id="oDesc" style="font-size:1.1em;">...</div>
            <div class="sub" style="color:#8b5e3c;">Tap for hourly</div>
          </div>
        </div>
        <div class="row">
          <div class="card">
            <div class="label">Humidity</div>
            <div class="value hum" id="oHum">--</div>
            <div class="unit">%</div>
          </div>
          <div class="card">
            <div class="label">Wind</div>
            <div class="value wind" id="oWind">--</div>
            <div class="unit">mph</div>
          </div>
        </div>
        <div class="row">
          <div class="card">
            <div class="label">Sunrise</div>
            <div class="value sun" style="font-size:1.3em;" id="sunrise">--:--</div>
          </div>
          <div class="card">
            <div class="label">Sunset</div>
            <div class="value sun" style="font-size:1.3em;" id="sunset">--:--</div>
          </div>
        </div>
        <div class="section">Indoor Sensor</div>
        <div class="status-pill"><div class="pill" id="status">Loading...</div></div>
        <div class="row">
          <div class="card">
            <div class="label">Temp</div>
            <div class="value temp" id="tempF">--</div>
            <div class="unit">&deg;F</div>
          </div>
          <div class="card">
            <div class="label">Temp</div>
            <div class="value temp" id="tempC">--</div>
            <div class="unit">&deg;C</div>
          </div>
          <div class="card">
            <div class="label">Humidity</div>
            <div class="value hum" id="dhtH">--</div>
            <div class="unit">%</div>
          </div>
        </div>
      </div>
      <div>
        <div class="section">7-Day Forecast</div>
        <div id="forecast"></div>
      </div>
    </div>
    <div class="footer">NWS Weather every 10 min &bull; <a href="/update" style="color:#8b5e3c;">OTA Update</a></div>
  </div>
  <div class="overlay" id="hourlyOverlay">
    <div class="container">
      <div class="overlay-header">
        <h2>Hourly Forecast</h2>
        <button class="close-btn" onclick="closeHourly()">Back</button>
      </div>
      <div id="hourlyList"><div class="overlay-loading">Loading...</div></div>
    </div>
  </div>
  <script>
    void(function(){var c=document.getElementById('clock'),d=document.getElementById('date');setInterval(function(){var n=new Date();c.textContent=n.toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:true,timeZone:'America/New_York'});d.textContent=n.toLocaleDateString('en-US',{weekday:'long',month:'long',day:'numeric',year:'numeric',timeZone:'America/New_York'});},1000)}());
    void(function(){var u=function(){fetch('/data').then(function(r){return r.json()}).then(function(d){var s=document.getElementById('status');if(d.sensor){s.textContent='Sensor Online';s.className='pill ok';document.getElementById('tempF').textContent=d.tempF.toFixed(1);document.getElementById('tempC').textContent=d.tempC.toFixed(1);document.getElementById('dhtH').textContent=d.dhtH.toFixed(1)}else{s.textContent='Sensor Not Connected';s.className='pill';document.getElementById('tempF').textContent='--';document.getElementById('tempC').textContent='--';document.getElementById('dhtH').textContent='--'}document.getElementById('oTemp').textContent=d.oTemp;document.getElementById('oHL').textContent='H: '+d.oHigh+'° / L: '+d.oLow+'°';document.getElementById('oHum').textContent=d.oHum;document.getElementById('oWind').textContent=d.oWind;document.getElementById('oDesc').innerHTML=d.oDesc;document.getElementById('sunrise').textContent=d.sunrise;document.getElementById('sunset').textContent=d.sunset;var fc=document.getElementById('forecast');var h='';for(var i=0;i<d.forecast.length;i++){var f=d.forecast[i];h+='<div class="forecast-row"><div class="day">'+f.day+'</div><div class="desc">'+f.desc+'</div><div><span class="hi">'+f.hi+'</span>° <span class="lo">'+f.lo+'</span>°</div></div>'}fc.innerHTML=h}).catch(function(){document.getElementById('status').textContent='Connection Lost';document.getElementById('status').className='pill'})};u();setInterval(u,5000)}());
  function descEmoji(d){d=d.toLowerCase();if(d.indexOf('thunder')>=0)return'⚡';if(d.indexOf('snow')>=0||d.indexOf('blizzard')>=0)return'\u{1F328}️';if(d.indexOf('rain')>=0||d.indexOf('shower')>=0||d.indexOf('drizzle')>=0)return'\u{1F327}️';if(d.indexOf('fog')>=0||d.indexOf('mist')>=0)return'\u{1F32B}️';if(d.indexOf('cloud')>=0||d.indexOf('overcast')>=0)return'⛅';if(d.indexOf('sunny')>=0||d.indexOf('clear')>=0)return'☀️';return'\u{1F324}️';}
  function showHourly(){document.getElementById('hourlyOverlay').className='overlay open';document.getElementById('hourlyList').innerHTML='<div class="overlay-loading">Loading...</div>';fetch('https://api.weather.gov/gridpoints/CTP/128,27/forecast/hourly',{headers:{'Accept':'application/geo+json'}}).then(function(r){return r.json()}).then(function(d){var p=d.properties.periods;var h='';var count=Math.min(p.length,24);for(var i=0;i<count;i++){var t=new Date(p[i].startTime);var hr=t.getHours();var ampm=hr>=12?'PM':'AM';if(hr===0)hr=12;else if(hr>12)hr-=12;var timeStr=hr+':00 '+ampm;var e=descEmoji(p[i].shortForecast);h+='<div class="hourly-row"><div class="hr-time">'+timeStr+'</div><div class="hr-desc">'+e+' '+p[i].shortForecast+'</div><div class="hr-temp">'+p[i].temperature+'&deg;</div><div class="hr-wind">'+p[i].windSpeed+'</div></div>'}document.getElementById('hourlyList').innerHTML=h}).catch(function(){document.getElementById('hourlyList').innerHTML='<div class="overlay-loading">Failed to load hourly forecast</div>'});}
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

  if (ESP.getFreeHeap() < 8192) {
    Serial.printf("WATCHDOG: heap low (%d bytes)\n", ESP.getFreeHeap());
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo) && timeinfo.tm_hour == 2 && timeinfo.tm_min == 0 && timeinfo.tm_sec < 3) {
    Serial.println("Scheduled daily reboot");
    delay(100);
    ESP.restart();
  }
}
