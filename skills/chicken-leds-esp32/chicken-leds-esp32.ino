#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Update.h>
#include <time.h>

const char* SSID     = "DIEMILTONHAUS";
const char* PASSWORD = "wisdom22!!";

const IPAddress STATIC_IP(192, 168, 12, 241);
const IPAddress GATEWAY(192, 168, 12, 1);
const IPAddress SUBNET(255, 255, 255, 0);
const IPAddress DNS1(192, 168, 12, 136);

const int LED_PIN = 16;
const char* NTP_SERVER = "pool.ntp.org";
const char* TIMEZONE   = "EST5EDT,M3.2.0,M11.1.0";

// DS3231 RTC address
const uint8_t DS3231_ADDR = 0x68;

WebServer server(80);
bool ledsOn = false;
bool manualOverride = false;
bool lastScheduleState = false;
bool ntpSynced = false;
bool rtcAvailable = false;

// Blink state
volatile bool blinkActive = false;
TaskHandle_t blinkTaskHandle = NULL;
int blinkDuration = 120;

static char statusBuf[256];

uint8_t bcd2dec(uint8_t bcd) { return (bcd >> 4) * 10 + (bcd & 0x0F); }

void setLeds(bool on) {
    ledsOn = on;
    digitalWrite(LED_PIN, on ? HIGH : LOW);
}

bool getRtcTime(struct tm* t) {
    Wire.beginTransmission(DS3231_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(DS3231_ADDR, (uint8_t)7);
    if (Wire.available() < 7) return false;
    t->tm_sec  = bcd2dec(Wire.read() & 0x7F);
    t->tm_min  = bcd2dec(Wire.read());
    t->tm_hour = bcd2dec(Wire.read() & 0x3F);
    Wire.read(); // day of week
    t->tm_mday = bcd2dec(Wire.read());
    t->tm_mon  = bcd2dec(Wire.read() & 0x1F) - 1;
    t->tm_year = bcd2dec(Wire.read()) + 100;
    return true;
}

// Returns "ntp", "rtc", or "none"
const char* getTime(struct tm* t) {
    if (getLocalTime(t, 100)) {
        ntpSynced = true;
        return "ntp";
    }
    if (rtcAvailable && getRtcTime(t)) return "rtc";
    return "none";
}

void applySchedule() {
    struct tm t;
    const char* src = getTime(&t);
    if (strcmp(src, "none") == 0) return;
    int h = t.tm_hour;
    int m = t.tm_min;
    // ON 4am-9am, ON 6pm-1:30am
    bool shouldBeOn = (h >= 4 && h < 9) || (h >= 18) || (h == 0) || (h == 1 && m < 30);
    if (shouldBeOn != lastScheduleState) {
        lastScheduleState = shouldBeOn;
        manualOverride = false;
    }
    if (!manualOverride && shouldBeOn != ledsOn) setLeds(shouldBeOn);
}

void handleOn() {
    manualOverride = true;
    setLeds(true);
    server.send(200, "text/plain", "LEDs ON\n");
}

void handleOff() {
    manualOverride = true;
    setLeds(false);
    server.send(200, "text/plain", "LEDs OFF\n");
}

void handleStatus() {
    struct tm t;
    const char* clockSrc = getTime(&t);
    bool synced = strcmp(clockSrc, "none") != 0;
    snprintf(statusBuf, sizeof(statusBuf),
        "leds:     %s\nclock:    %s\ntime:     %02d:%02d:%02d\nip:       %s\noverride: %s\n",
        ledsOn ? "on" : "off",
        clockSrc,
        synced ? t.tm_hour : 0,
        synced ? t.tm_min  : 0,
        synced ? t.tm_sec  : 0,
        WiFi.localIP().toString().c_str(),
        manualOverride ? "yes" : "no"
    );
    server.send(200, "text/plain", statusBuf);
}

// Blink task runs on core 0
void blinkTask(void* param) {
    int durationSec = *((int*)param);
    unsigned long endMs = millis() + (unsigned long)durationSec * 1000;
    bool savedState = ledsOn;
    while (blinkActive && millis() < endMs) {
        setLeds(true);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (!blinkActive) break;
        setLeds(false);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    blinkActive = false;
    applySchedule();
    blinkTaskHandle = NULL;
    vTaskDelete(NULL);
}

void handleBlink() {
    if (blinkActive) {
        server.send(200, "text/plain", "Blink already running\n");
        return;
    }
    blinkDuration = 120;
    if (server.hasArg("sec")) {
        int s = server.arg("sec").toInt();
        if (s >= 1 && s <= 600) blinkDuration = s;
    }
    blinkActive = true;
    xTaskCreatePinnedToCore(blinkTask, "blink", 2048, &blinkDuration, 1, &blinkTaskHandle, 0);
    char buf[64];
    snprintf(buf, sizeof(buf), "Blinking for %d seconds\n", blinkDuration);
    server.send(200, "text/plain", buf);
}

void handleBlinkStop() {
    if (!blinkActive) {
        server.send(200, "text/plain", "No blink running\n");
        return;
    }
    blinkActive = false;
    server.send(200, "text/plain", "Blink stopped\n");
}

// OTA update page
const char otaPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Chicken LEDs OTA</title>
<style>body{font-family:sans-serif;background:#222;color:#eee;text-align:center;padding:40px;}
input[type=file]{margin:20px;}input[type=submit]{padding:10px 20px;font-size:16px;}</style>
</head><body>
<h2>Chicken LEDs — OTA Update</h2>
<form method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="update" accept=".bin"><br>
<input type="submit" value="Upload Firmware">
</form></body></html>
)rawliteral";

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    WiFi.config(STATIC_IP, GATEWAY, SUBNET, DNS1);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 40) delay(500);
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Wire.begin();
    // Check if DS3231 is present
    Wire.beginTransmission(DS3231_ADDR);
    rtcAvailable = (Wire.endTransmission() == 0);

    connectWiFi();

    configTzTime(TIMEZONE, NTP_SERVER);

    // Wait up to 10s for NTP sync
    struct tm t;
    for (int i = 0; i < 20; i++) {
        if (getLocalTime(&t, 100)) { ntpSynced = true; break; }
        delay(500);
    }

    applySchedule();

    server.on("/leds-on",  handleOn);
    server.on("/leds-off", handleOff);
    server.on("/status",   handleStatus);
    server.on("/blink",    handleBlink);
    server.on("/blink-stop", handleBlinkStop);

    server.on("/update", HTTP_GET, []() {
        server.send(200, "text/html", otaPage);
    });
    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", Update.hasError() ? "Update FAILED" : "Update OK - Rebooting...");
        delay(1000);
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
        } else if (upload.status == UPLOAD_FILE_END) {
            if (!Update.end(true)) Update.printError(Serial);
        }
    });

    server.begin();
}

unsigned long lastCheck = 0;

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
        if (WiFi.status() == WL_CONNECTED) {
            configTzTime(TIMEZONE, NTP_SERVER);
        }
    }
    server.handleClient();

    if (millis() - lastCheck >= 30000) {
        lastCheck = millis();
        if (!blinkActive) applySchedule();
    }
}
