#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <Update.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <time.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11

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
String fcDay[7], fcDesc[7], fcHi[7], fcLo[7];

SemaphoreHandle_t piMutex;
String piLedState = "--";
String piIrrigationState = "--";
String piCpuTemp = "--";
String piUptime = "--";
bool piConnected = false;
unsigned long lastSuccessfulBleMillis = 0;
int bleMissCount = 0;
volatile bool otaInProgress = false;

static BLEUUID piServiceUUID("12345678-1234-5678-1234-56789abcdef0");
static BLEUUID piCmdUUID("12345678-1234-5678-1234-56789abcdef1");
static BLEUUID piRespUUID("12345678-1234-5678-1234-56789abcdef2");

String weatherCodeToDesc(int code) {
  if (code == 0) return "Clear";
  if (code <= 3) return "Partly Cloudy";
  if (code <= 49) return "Foggy";
  if (code <= 59) return "Drizzle";
  if (code <= 69) return "Rain";
  if (code <= 79) return "Snow";
  if (code <= 82) return "Rain Showers";
  if (code <= 86) return "Snow Showers";
  if (code >= 95) return "Thunderstorm";
  return "Unknown";
}

String weatherCodeToEmoji(int code) {
  if (code == 0) return "&#9728;&#65039;";
  if (code <= 3) return "&#9925;";
  if (code <= 49) return "&#127787;&#65039;";
  if (code <= 59) return "&#127782;&#65039;";
  if (code <= 69) return "&#127783;&#65039;";
  if (code <= 79) return "&#127784;&#65039;";
  if (code <= 82) return "&#127783;&#65039;";
  if (code <= 86) return "&#127784;&#65039;";
  if (code >= 95) return "&#9889;";
  return "?";
}

String getDayName(const char* dateStr) {
  struct tm tm = {};
  sscanf(dateStr, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
  tm.tm_year -= 1900;
  tm.tm_mon -= 1;
  mktime(&tm);
  const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return String(days[tm.tm_wday]);
}

String extractTime(const String& isoTime) {
  int tPos = isoTime.indexOf('T');
  if (tPos < 0) return isoTime;
  int h = isoTime.substring(tPos + 1, tPos + 3).toInt();
  String m = isoTime.substring(tPos + 4, tPos + 6);
  String ampm = (h >= 12) ? "PM" : "AM";
  if (h == 0) h = 12;
  else if (h > 12) h -= 12;
  return String(h) + ":" + m + " " + ampm;
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  http.begin("https://api.open-meteo.com/v1/forecast?latitude=39.98&longitude=-76.28&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m&daily=temperature_2m_max,temperature_2m_min,weather_code,sunrise,sunset&temperature_unit=fahrenheit&wind_speed_unit=mph&timezone=America/New_York&forecast_days=8");
  int code = http.GET();

  if (code == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      outsideTemp = String(doc["current"]["temperature_2m"].as<float>(), 1);
      outsideHumidity = String(doc["current"]["relative_humidity_2m"].as<int>());
      windSpeed = String(doc["current"]["wind_speed_10m"].as<float>(), 1);
      int wc = doc["current"]["weather_code"].as<int>();
      weatherDesc = weatherCodeToEmoji(wc) + " " + weatherCodeToDesc(wc);

      outsideHigh = String(doc["daily"]["temperature_2m_max"][0].as<float>(), 0);
      outsideLow = String(doc["daily"]["temperature_2m_min"][0].as<float>(), 0);

      sunrise = extractTime(doc["daily"]["sunrise"][0].as<String>());
      sunset = extractTime(doc["daily"]["sunset"][0].as<String>());

      for (int i = 0; i < 7; i++) {
        const char* dd = doc["daily"]["time"][i + 1];
        fcDay[i] = getDayName(dd);
        fcHi[i] = String(doc["daily"]["temperature_2m_max"][i + 1].as<float>(), 0);
        fcLo[i] = String(doc["daily"]["temperature_2m_min"][i + 1].as<float>(), 0);
        int fwc = doc["daily"]["weather_code"][i + 1].as<int>();
        fcDesc[i] = weatherCodeToEmoji(fwc) + " " + weatherCodeToDesc(fwc);
      }

      Serial.println("Weather updated successfully");
    }
  } else {
    Serial.printf("Weather fetch failed: %d\n", code);
  }
  http.end();
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

String extractField(const String& text, const String& key) {
  int idx = text.indexOf(key);
  if (idx < 0) return "--";
  int start = idx + key.length();
  int end = text.indexOf('\n', start);
  if (end < 0) end = text.length();
  String val = text.substring(start, end);
  val.trim();
  return val;
}

volatile bool bleResetRequested = false;

void bleTask(void* param) {
  delay(15000);
  BLEAddress piAddress("b8:27:eb:f6:24:e9");
  Serial.println("BLE: direct connect to b8:27:eb:f6:24:e9");
  int backoffMs = 30000;

  while (true) {
    if (otaInProgress) {
      delay(1000);
      continue;
    }

    if (bleResetRequested) {
      bleResetRequested = false;
      backoffMs = 30000;
      bleMissCount = 0;
      Serial.println("BLE: manual reset, retrying now");
    }

    if (WiFi.status() != WL_CONNECTED || WiFi.RSSI() == 0 || WiFi.RSSI() < -80) {
      Serial.printf("BLE: skipping, WiFi weak (status=%d RSSI=%d)\n", WiFi.status(), WiFi.RSSI());
      delay(30000);
      continue;
    }

    Serial.printf("BLE: connecting (heap: %d, WiFi: %d, RSSI: %d)\n", ESP.getFreeHeap(), WiFi.status(), WiFi.RSSI());
    BLEClient* client = BLEDevice::createClient();
    bool ok = false;

    try {
      ok = client->connect(piAddress, BLE_ADDR_TYPE_PUBLIC, 5000);
    } catch (...) {
      Serial.println("BLE: connect exception");
      ok = false;
    }

    if (ok) {
      BLERemoteService* svc = client->getService(piServiceUUID);
      if (svc) {
        BLERemoteCharacteristic* cmdChar = svc->getCharacteristic(piCmdUUID);
        BLERemoteCharacteristic* respChar = svc->getCharacteristic(piRespUUID);
        if (cmdChar && respChar) {
          cmdChar->writeValue(String("status"), true);

          String resp;
          for (int attempt = 0; attempt < 3; attempt++) {
            delay(500);
            resp = respChar->readValue();
            if (resp.length() > 0) break;
            Serial.printf("BLE: empty read, retry %d/3\n", attempt + 1);
          }

          if (resp.length() > 0) {
            String response = resp;
            if (xSemaphoreTake(piMutex, pdMS_TO_TICKS(200))) {
              piLedState = extractField(response, "LEDs: ");
              piIrrigationState = extractField(response, "Irrigation: ");
              piUptime = extractField(response, "Uptime: ");
              piCpuTemp = extractField(response, "CPU Temp: ");
              piConnected = true;
              lastSuccessfulBleMillis = millis();
              bleMissCount = 0;
              xSemaphoreGive(piMutex);
            }
            Serial.println("Pi status updated via BLE");
          } else {
            Serial.println("BLE: connected but got empty response");
          }
        }
      }
      client->disconnect();
      delete client;
      backoffMs = 30000;
      if (bleMissCount > 0) bleMissCount--;
    } else {
      bleMissCount++;
      if (bleMissCount >= 3 && xSemaphoreTake(piMutex, pdMS_TO_TICKS(200))) {
        piConnected = false;
        xSemaphoreGive(piMutex);
      }
      client->disconnect();
      delete client;
      Serial.printf("BLE connect failed (miss %d, retry in %ds)\n", bleMissCount, backoffMs / 1000);
      if (backoffMs < 60000) backoffMs += 10000;
    }

    delay(backoffMs);
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
      background: #1a1a2e;
      color: #eee;
      height: 100vh;
      overflow: hidden;
      padding: 8px 10px;
    }
    .grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 6px;
      max-width: 900px;
      margin: 0 auto;
      height: 100%;
      align-content: start;
    }
    .span2 { grid-column: span 2; }
    .header { text-align: center; padding: 2px 0; }
    .header h1 { color: #e94560; font-size: 1.1em; margin: 0; display: inline; }
    .header .time { font-size: 1.4em; font-weight: bold; color: #fff; }
    .header .date { font-size: 0.75em; color: #888; }
    .section { color: #e94560; font-size: 0.65em; text-transform: uppercase; letter-spacing: 2px; padding: 4px 0 0; }
    .card {
      background: #16213e;
      border-radius: 8px;
      padding: 6px 8px;
      text-align: center;
      border: 1px solid #0f3460;
    }
    .card .label { font-size: 0.6em; color: #888; text-transform: uppercase; letter-spacing: 1px; }
    .card .value { font-size: 1.4em; font-weight: bold; margin: 1px 0; }
    .card .unit { font-size: 0.55em; color: #888; }
    .card .sub { font-size: 0.65em; color: #aaa; }
    .big .value { font-size: 1.8em; }
    .temp { color: #e94560; }
    .hum { color: #4ecca3; }
    .wind { color: #a8d8ea; }
    .sun { color: #f9d923; }
    .conditions { color: #ccc; }
    .fc {
      background: #16213e;
      border-radius: 8px;
      border: 1px solid #0f3460;
      padding: 5px 10px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.85em;
    }
    .fc .day { font-weight: bold; }
    .fc .desc { color: #aaa; font-size: 0.85em; flex: 1; text-align: center; }
    .fc .hi { color: #e94560; font-weight: bold; }
    .fc .lo { color: #888; margin-left: 6px; }
    #fcList { display: flex; flex-direction: column; gap: 4px; }
    .pill {
      display: inline-block;
      padding: 3px 12px;
      border-radius: 14px;
      font-size: 0.7em;
      color: #0f3460;
      background: #e94560;
    }
    .pill.ok { background: #4ecca3; }
    .pi-on { color: #4ecca3; }
    .pi-off { color: #888; }
    .footer { text-align: center; color: #555; font-size: 0.6em; padding: 2px 0; }
  </style>
</head>
<body>
  <div class="grid">
    <div class="span2 header">
      <h1>MILTONHAUS</h1>
      <div class="time" id="clock">--:--</div>
      <div class="date" id="date">Loading...</div>
    </div>

    <div class="card big">
      <div class="label">Temperature</div>
      <div class="value temp" id="oTemp">--</div>
      <div class="unit">&deg;F</div>
      <div class="sub" id="oHL">H: -- / L: --</div>
    </div>
    <div class="card">
      <div class="label">Conditions</div>
      <div class="value conditions" id="oDesc" style="font-size:0.9em;">...</div>
      <div style="font-size:0.65em; color:#888; margin-top:2px;">
        <span class="hum" id="oHum">--</span>% hum &bull;
        <span class="wind" id="oWind">--</span> mph
      </div>
    </div>

    <div class="card">
      <div class="label">Sunrise</div>
      <div class="value sun" style="font-size:1.1em;" id="sunrise">--:--</div>
    </div>
    <div class="card">
      <div class="label">Sunset</div>
      <div class="value sun" style="font-size:1.1em;" id="sunset">--:--</div>
    </div>

    <div id="fcList" class="span2"></div>

    <div class="span2 section">Homestead Pi <span class="pill" id="piStatus">Scanning...</span></div>
    <div class="card">
      <div class="label">LEDs</div>
      <div class="value pi-off" id="piLed" style="font-size:1.1em;">--</div>
    </div>
    <div class="card">
      <div class="label">Irrigation</div>
      <div class="value pi-off" id="piWater" style="font-size:1.1em;">--</div>
    </div>
    <div class="card">
      <div class="label">CPU Temp</div>
      <div class="value temp" id="piTemp" style="font-size:1.1em;">--</div>
    </div>
    <div class="card">
      <div class="label">Uptime</div>
      <div class="value" id="piUp" style="font-size:0.75em; color:#a8d8ea;">--</div>
    </div>

    <div class="span2 section">Indoor <span class="pill" id="status">Loading...</span></div>
    <div class="card">
      <div class="label">Temp</div>
      <div class="value temp" id="tempF" style="font-size:1.1em;">--</div>
      <div class="unit">&deg;F / <span id="tempC">--</span>&deg;C</div>
    </div>
    <div class="card">
      <div class="label">Humidity</div>
      <div class="value hum" id="dhtH" style="font-size:1.1em;">--</div>
      <div class="unit">%</div>
    </div>

    <div class="span2 footer"><a href="/update" style="color:#e94560;">OTA Update</a></div>
  </div>
  <script>
    void(function(){var c=document.getElementById('clock'),d=document.getElementById('date');setInterval(function(){var n=new Date();c.textContent=n.toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:true,timeZone:'America/New_York'});d.textContent=n.toLocaleDateString('en-US',{weekday:'long',month:'long',day:'numeric',year:'numeric',timeZone:'America/New_York'});},1000)}());
    void(function(){var u=function(){fetch('/data').then(function(r){return r.json()}).then(function(d){var s=document.getElementById('status');if(d.sensor){s.textContent='Online';s.className='pill ok';document.getElementById('tempF').textContent=d.tempF.toFixed(1);document.getElementById('tempC').textContent=d.tempC.toFixed(1);document.getElementById('dhtH').textContent=d.dhtH.toFixed(1)}else{s.textContent='No Sensor';s.className='pill';document.getElementById('tempF').textContent='--';document.getElementById('tempC').textContent='--';document.getElementById('dhtH').textContent='--'}document.getElementById('oTemp').textContent=d.oTemp;document.getElementById('oHL').textContent='H: '+d.oHigh+'° / L: '+d.oLow+'°';document.getElementById('oHum').textContent=d.oHum;document.getElementById('oWind').textContent=d.oWind;document.getElementById('oDesc').innerHTML=d.oDesc;document.getElementById('sunrise').textContent=d.sunrise;document.getElementById('sunset').textContent=d.sunset;var fl=document.getElementById('fcList');fl.innerHTML='';if(d.fc){d.fc.forEach(function(f){var r=document.createElement('div');r.className='fc';r.innerHTML='<div class="day">'+f.d+'</div><div class="desc">'+f.c+'</div><div><span class="hi">'+f.h+'</span>&deg; <span class="lo">'+f.l+'</span>&deg;</div>';fl.appendChild(r)})}var ps=document.getElementById('piStatus');var hasPiData=d.piLed&&d.piLed!=='--';if(d.piConn){ps.textContent='Connected';ps.className='pill ok'}else if(hasPiData){ps.textContent='Last Data';ps.className='pill ok'}else{ps.textContent='Not in Range';ps.className='pill'}if(hasPiData||d.piConn){document.getElementById('piLed').textContent=d.piLed;document.getElementById('piLed').className='value '+(d.piLed==='ON'?'pi-on':'pi-off');document.getElementById('piWater').textContent=d.piWater;document.getElementById('piWater').className='value '+(d.piWater==='ON'?'pi-on':'pi-off');document.getElementById('piTemp').textContent=d.piTemp;document.getElementById('piUp').textContent=d.piUp}else{document.getElementById('piLed').textContent='--';document.getElementById('piLed').className='value pi-off';document.getElementById('piWater').textContent='--';document.getElementById('piWater').className='value pi-off';document.getElementById('piTemp').textContent='--';document.getElementById('piUp').textContent='--'}}).catch(function(){document.getElementById('status').textContent='Lost';document.getElementById('status').className='pill'})};u();setInterval(u,5000)}());
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
    body { font-family: 'Comfortaa', sans-serif; background: #1a1a2e; color: #eee; padding: 40px; text-align: center; }
    h2 { color: #e94560; margin-bottom: 20px; }
    input[type=file] { margin: 20px; }
    input[type=submit] { padding: 12px 24px; background: #e94560; color: #fff; border: none; border-radius: 8px; cursor: pointer; font-family: inherit; font-size: 1em; }
    input[type=submit]:hover { background: #c73650; }
    a { color: #4ecca3; }
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
  String pLed, pWater, pTemp, pUp;
  bool pConn;
  if (xSemaphoreTake(piMutex, pdMS_TO_TICKS(100))) {
    pLed = piLedState;
    pWater = piIrrigationState;
    pTemp = piCpuTemp;
    pUp = piUptime;
    pConn = (lastSuccessfulBleMillis > 0 && (millis() - lastSuccessfulBleMillis < 90000));
    xSemaphoreGive(piMutex);
  } else {
    pLed = "--"; pWater = "--"; pTemp = "--"; pUp = "--";
    pConn = false;
  }

  String json = "{\"tempC\":" + String(tempC, 1) +
                ",\"tempF\":" + String(tempF, 1) +
                ",\"dhtH\":" + String(dhtHumidity, 1) +
                ",\"sensor\":" + (sensorConnected ? "true" : "false") +
                ",\"oTemp\":\"" + outsideTemp + "\"" +
                ",\"oHigh\":\"" + outsideHigh + "\"" +
                ",\"oLow\":\"" + outsideLow + "\"" +
                ",\"oHum\":\"" + outsideHumidity + "\"" +
                ",\"oWind\":\"" + windSpeed + "\"" +
                ",\"oDesc\":\"" + weatherDesc + "\"" +
                ",\"sunrise\":\"" + sunrise + "\"" +
                ",\"sunset\":\"" + sunset + "\"" +
                ",\"fc\":[";
  for (int i = 0; i < 7; i++) {
    if (i > 0) json += ",";
    json += "{\"d\":\"" + fcDay[i] + "\",\"c\":\"" + fcDesc[i] + "\",\"h\":\"" + fcHi[i] + "\",\"l\":\"" + fcLo[i] + "\"}";
  }
  json += "]";
  json += ",\"piConn\":" + String(pConn ? "true" : "false") +
                ",\"piLed\":\"" + pLed + "\"" +
                ",\"piWater\":\"" + pWater + "\"" +
                ",\"piTemp\":\"" + pTemp + "\"" +
                ",\"piUp\":\"" + pUp + "\"" +
                ",\"bleMiss\":" + String(bleMissCount) + "}";
  server.send(200, "application/json", json);
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

    configTzTime("EST5EDT,M3.2.0,M11.1.0", ntpServer);
    Serial.println("NTP time sync started (Eastern)");

    fetchWeather();
  } else {
    Serial.println("\nWiFi FAILED - will retry in loop");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.on("/ble-reset", []() {
    bleResetRequested = true;
    server.send(200, "text/plain", "BLE reset requested — will retry on next cycle");
  });

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
      BLEDevice::deinit(false);
      Serial.printf("OTA update: %s (BLE paused)\n", upload.filename.c_str());
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

  piMutex = xSemaphoreCreateMutex();

  BLEDevice::init("miltonhaus-weather");
  Serial.println("BLE initialized");

  xTaskCreatePinnedToCore(bleTask, "bleTask", 16384, NULL, 1, NULL, 0);
  Serial.println("BLE task started on core 0");
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

  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiCheck > 30000) {
    lastWifiCheck = millis();
    wifiFailCount++;
    Serial.printf("WiFi lost (status=%d RSSI=%d fail=%d) - reconnecting...\n", WiFi.status(), WiFi.RSSI(), wifiFailCount);
    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(ssid, password);
    if (wifiFailCount >= 10) {
      Serial.println("WATCHDOG: WiFi failed 10x, rebooting");
      delay(100);
      ESP.restart();
    }
  }

  if (WiFi.status() == WL_CONNECTED && wifiFailCount > 0) {
    IPAddress staticIP(192, 168, 12, 240);
    IPAddress gateway(192, 168, 12, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(192, 168, 12, 1);
    WiFi.config(staticIP, gateway, subnet, dns);
    Serial.print("WiFi reconnected! IP: ");
    Serial.println(WiFi.localIP());
    wifiFailCount = 0;
  }

  // Heap watchdog: reboot if free heap drops below 8KB
  if (ESP.getFreeHeap() < 8192) {
    Serial.println("WATCHDOG: heap critical, rebooting");
    delay(100);
    ESP.restart();
  }

  // Daily reboot at 3:00 AM to clear heap fragmentation
  struct tm timeinfo;
  if (getLocalTime(&timeinfo) && timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec < 3) {
    Serial.println("Scheduled daily reboot");
    delay(100);
    ESP.restart();
  }
}
