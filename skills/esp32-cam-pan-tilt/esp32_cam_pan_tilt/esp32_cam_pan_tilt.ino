/*
 * ESP32-CAM Pan-Tilt Web Controller
 *
 * Hardware: ESP32-CAM with OV2640 + 2x SG90 Servos
 * Board: "AI Thinker ESP32-CAM"
 *
 * Features:
 * - Live video streaming via web browser
 * - Pan/Tilt servo control via web interface
 * - WiFi Access Point or Station mode
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include <ESP32Servo.h>

// ===========================================
// WiFi Configuration
// ===========================================
// Option 1: Connect to existing WiFi
const char* ssid = "MILTONHAUS";
const char* password = "wisdom22!!";

// Option 2: Create Access Point (set useAP = true)
bool useAP = false;
const char* apSSID = "ESP32-CAM-PanTilt";
const char* apPassword = "12345678";

// ===========================================
// Servo Configuration
// ===========================================
#define SERVO_PAN_PIN   14  // GPIO14 for Pan (horizontal)
#define SERVO_TILT_PIN  15  // GPIO15 for Tilt (vertical)

// ===========================================
// Joystick Serial Configuration
// ===========================================
#define JOYSTICK_RX_PIN 4  // GPIO4 for receiving from Arduino (flash LED pin)

Servo panServo;
Servo tiltServo;

int panAngle = 90;   // Start centered
int tiltAngle = 90;  // Start centered

// ===========================================
// Camera Pin Definitions (AI Thinker ESP32-CAM)
// ===========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

// ===========================================
// HTML Web Interface
// ===========================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Pan-Tilt</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial;
      text-align: center;
      margin: 0;
      padding: 20px;
      background: #1a1a2e;
      color: #eee;
    }
    h1 { color: #00d4ff; margin-bottom: 10px; }
    .stream-container {
      margin: 10px auto;
      max-width: 640px;
    }
    img {
      width: 100%;
      border-radius: 10px;
      border: 2px solid #00d4ff;
    }
    .controls {
      display: grid;
      grid-template-columns: repeat(3, 70px);
      gap: 10px;
      justify-content: center;
      margin: 20px auto;
    }
    .btn {
      width: 70px;
      height: 70px;
      font-size: 24px;
      border: none;
      border-radius: 10px;
      background: #16213e;
      color: #00d4ff;
      cursor: pointer;
      touch-action: manipulation;
    }
    .btn:active { background: #00d4ff; color: #1a1a2e; }
    .btn:disabled { opacity: 0.3; }
    .center-btn { background: #e94560; }
    .slider-container {
      margin: 20px auto;
      max-width: 300px;
    }
    .slider-label {
      display: flex;
      justify-content: space-between;
      margin-bottom: 5px;
    }
    input[type="range"] {
      width: 100%;
      height: 20px;
      -webkit-appearance: none;
      background: #16213e;
      border-radius: 10px;
      outline: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 30px;
      height: 30px;
      background: #00d4ff;
      border-radius: 50%;
      cursor: pointer;
    }
    .info { color: #888; font-size: 12px; margin-top: 20px; }
  </style>
</head>
<body>
  <h1>ESP32-CAM Pan-Tilt</h1>
  <div class="stream-container">
    <img id="stream" src="">
  </div>

  <div class="controls">
    <button class="btn" disabled></button>
    <button class="btn" ontouchstart="move('up')" onmousedown="move('up')">&#9650;</button>
    <button class="btn" disabled></button>
    <button class="btn" ontouchstart="move('left')" onmousedown="move('left')">&#9664;</button>
    <button class="btn center-btn" onclick="center()">&#9679;</button>
    <button class="btn" ontouchstart="move('right')" onmousedown="move('right')">&#9654;</button>
    <button class="btn" disabled></button>
    <button class="btn" ontouchstart="move('down')" onmousedown="move('down')">&#9660;</button>
    <button class="btn" disabled></button>
  </div>

  <div class="slider-container">
    <div class="slider-label"><span>Pan</span><span id="panVal">90</span></div>
    <input type="range" id="pan" min="0" max="180" value="90"
           oninput="setServo('pan', this.value)">
  </div>

  <div class="slider-container">
    <div class="slider-label"><span>Tilt</span><span id="tiltVal">90</span></div>
    <input type="range" id="tilt" min="0" max="180" value="90"
           oninput="setServo('tilt', this.value)">
  </div>

  <p class="info">Step: 10&deg; | Range: 0-180&deg;</p>

  <script>
    window.onload = function() {
      document.getElementById('stream').src = window.location.href.slice(0,-1) + ":81/stream";
    }

    function move(dir) {
      fetch('/move?dir=' + dir);
      setTimeout(updateSliders, 100);
    }

    function center() {
      fetch('/center');
      document.getElementById('pan').value = 90;
      document.getElementById('tilt').value = 90;
      document.getElementById('panVal').innerText = 90;
      document.getElementById('tiltVal').innerText = 90;
    }

    function setServo(servo, val) {
      fetch('/set?' + servo + '=' + val);
      document.getElementById(servo + 'Val').innerText = val;
    }

    function updateSliders() {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('pan').value = data.pan;
          document.getElementById('tilt').value = data.tilt;
          document.getElementById('panVal').innerText = data.pan;
          document.getElementById('tiltVal').innerText = data.tilt;
        });
    }
  </script>
</body>
</html>
)rawliteral";

// ===========================================
// Camera Initialization
// ===========================================
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Lower resolution for smoother streaming
  config.frame_size = FRAMESIZE_QVGA;  // 320x240
  config.jpeg_quality = 30;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }
  Serial.println("Camera initialized");
}

// ===========================================
// HTTP Handlers
// ===========================================
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, index_html, strlen(index_html));
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];

  res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      size_t hlen = snprintf(part_buf, 64,
        "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
      res = httpd_resp_send_chunk(req, part_buf, hlen);
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
      }
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, "\r\n", 2);
      }
      esp_camera_fb_return(fb);
    }
    if (res != ESP_OK) break;
  }
  return res;
}

static esp_err_t move_handler(httpd_req_t *req) {
  char buf[32];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char dir[10];
    if (httpd_query_key_value(buf, "dir", dir, sizeof(dir)) == ESP_OK) {
      if (strcmp(dir, "up") == 0) {
        tiltAngle = constrain(tiltAngle - 10, 0, 180);
        tiltServo.write(tiltAngle);
      } else if (strcmp(dir, "down") == 0) {
        tiltAngle = constrain(tiltAngle + 10, 0, 180);
        tiltServo.write(tiltAngle);
      } else if (strcmp(dir, "left") == 0) {
        panAngle = constrain(panAngle + 10, 0, 180);
        panServo.write(panAngle);
      } else if (strcmp(dir, "right") == 0) {
        panAngle = constrain(panAngle - 10, 0, 180);
        panServo.write(panAngle);
      }
      Serial.printf("Move: %s | Pan: %d, Tilt: %d\n", dir, panAngle, tiltAngle);
    }
  }
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "OK", 2);
}

static esp_err_t center_handler(httpd_req_t *req) {
  panAngle = 90;
  tiltAngle = 90;
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);
  Serial.println("Centered servos");
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "OK", 2);
}

static esp_err_t set_handler(httpd_req_t *req) {
  char buf[32];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char val[8];
    if (httpd_query_key_value(buf, "pan", val, sizeof(val)) == ESP_OK) {
      panAngle = constrain(atoi(val), 0, 180);
      panServo.write(panAngle);
    }
    if (httpd_query_key_value(buf, "tilt", val, sizeof(val)) == ESP_OK) {
      tiltAngle = constrain(atoi(val), 0, 180);
      tiltServo.write(tiltAngle);
    }
  }
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "OK", 2);
}

static esp_err_t status_handler(httpd_req_t *req) {
  char json[50];
  snprintf(json, sizeof(json), "{\"pan\":%d,\"tilt\":%d}", panAngle, tiltAngle);
  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, json, strlen(json));
}

void startWebServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler };
  httpd_uri_t move_uri = { .uri = "/move", .method = HTTP_GET, .handler = move_handler };
  httpd_uri_t center_uri = { .uri = "/center", .method = HTTP_GET, .handler = center_handler };
  httpd_uri_t set_uri = { .uri = "/set", .method = HTTP_GET, .handler = set_handler };
  httpd_uri_t status_uri = { .uri = "/status", .method = HTTP_GET, .handler = status_handler };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &move_uri);
    httpd_register_uri_handler(camera_httpd, &center_uri);
    httpd_register_uri_handler(camera_httpd, &set_uri);
    httpd_register_uri_handler(camera_httpd, &status_uri);
  }

  config.server_port = 81;
  config.ctrl_port = 32769;
  httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler };
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

// ===========================================
// Setup
// ===========================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\nESP32-CAM Pan-Tilt Starting...");

  // Initialize Serial2 for joystick input on GPIO13
  Serial2.begin(9600, SERIAL_8N1, JOYSTICK_RX_PIN, -1);  // RX only, 9600 baud
  Serial.println("Joystick serial initialized on GPIO13");

  // Initialize servos
  ESP32PWM::allocateTimer(1);  // Timer 0 used by camera
  ESP32PWM::allocateTimer(2);
  panServo.setPeriodHertz(50);
  tiltServo.setPeriodHertz(50);
  panServo.attach(SERVO_PAN_PIN, 500, 2400);
  tiltServo.attach(SERVO_TILT_PIN, 500, 2400);
  panServo.write(panAngle);
  tiltServo.write(tiltAngle);
  Serial.println("Servos initialized");

  // Initialize camera
  initCamera();

  // Connect to WiFi or start AP
  if (useAP) {
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP Started: ");
    Serial.println(apSSID);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nWiFi failed, starting AP mode");
      WiFi.softAP(apSSID, apPassword);
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());
    }
  }

  // Start web server
  startWebServer();
  Serial.println("Web server started");
  Serial.println("Open browser to the IP address above");
}

// ===========================================
// Loop - Handle serial commands from Arduino joystick
// ===========================================
void loop() {
  // Check for serial commands from Arduino on Serial2 (GPIO13)
  // Commands: U=up, D=down, L=left, R=right, C=center
  if (Serial2.available()) {
    char cmd = Serial2.read();

    switch (cmd) {
      case 'U':  // Tilt up
        tiltAngle = constrain(tiltAngle - 10, 0, 180);
        tiltServo.write(tiltAngle);
        Serial.printf("Joystick: Up | Tilt: %d\n", tiltAngle);
        break;
      case 'D':  // Tilt down
        tiltAngle = constrain(tiltAngle + 10, 0, 180);
        tiltServo.write(tiltAngle);
        Serial.printf("Joystick: Down | Tilt: %d\n", tiltAngle);
        break;
      case 'L':  // Pan left
        panAngle = constrain(panAngle + 10, 0, 180);
        panServo.write(panAngle);
        Serial.printf("Joystick: Left | Pan: %d\n", panAngle);
        break;
      case 'R':  // Pan right
        panAngle = constrain(panAngle - 10, 0, 180);
        panServo.write(panAngle);
        Serial.printf("Joystick: Right | Pan: %d\n", panAngle);
        break;
      case 'C':  // Center
        panAngle = 90;
        tiltAngle = 90;
        panServo.write(panAngle);
        tiltServo.write(tiltAngle);
        Serial.println("Joystick: Centered");
        break;
    }

    // Clear any remaining characters (like newline)
    while (Serial2.available()) {
      Serial2.read();
    }
  }

  delay(10);
}
