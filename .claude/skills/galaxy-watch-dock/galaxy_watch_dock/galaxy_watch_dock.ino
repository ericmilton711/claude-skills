/*
 * Galaxy Watch Charging Dock LED Controller
 *
 * Detection: TTP-223 Capacitive Touch Sensor (proximity mode)
 * Effect: Orange/Amber breathing glow
 *
 * Hardware:
 *   - Arduino Nano
 *   - WS2812B 7-LED Ring
 *   - TTP-223 Capacitive Touch Module
 *
 * Wiring:
 *   Arduino Nano       WS2812B LED Ring
 *   -----------        ----------------
 *   5V  --------------> VCC
 *   GND --------------> GND
 *   D6  --------------> DIN
 *
 *   Arduino Nano       TTP-223 Module
 *   -----------        --------------
 *   5V  --------------> VCC
 *   GND --------------> GND
 *   D2  --------------> SIG (or OUT/IO)
 *
 * TTP-223 Setup for Proximity Detection:
 *   - The module can detect through thin non-metallic materials
 *   - Mount it facing upward, underneath where the watch sits
 *   - May need to adjust sensitivity or add a thin spacer
 *   - Some modules have a small jumper (A/B) to toggle mode
 */

#include <Adafruit_NeoPixel.h>

// Pin definitions
#define LED_PIN     6    // WS2812B data pin
#define SENSOR_PIN  2    // TTP-223 signal pin

// LED configuration
#define NUM_LEDS    7    // 7-LED ring
#define BRIGHTNESS  50  // 0-255

// Color: Orange/Amber (R, G, B)
uint8_t baseColor[3] = {255, 80, 0};

// Breathing effect settings
#define BREATH_SPEED 10  // ms between brightness steps (lower = faster)

// Debounce settings to prevent flicker
#define DEBOUNCE_READINGS 10
#define DEBOUNCE_DELAY_MS 20
#define REQUIRED_HIGH_COUNT 8      // Require 8 of 10 readings (80%)

// Sustained detection - must stay triggered for this long
#define SUSTAINED_MS 100           // 300ms sustained before activating

// Cooldown after watch removed before re-detecting
#define COOLDOWN_MS 500

Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// State variables
int breatheBrightness = 0;
int breatheDirection = 1;  // 1 = brighter, -1 = dimmer
unsigned long lastBreathUpdate = 0;
bool watchDetected = false;

// Sustained detection tracking
bool sensorTriggered = false;
unsigned long triggerStartTime = 0;
unsigned long lastDetectedTime = 0;
bool watchConfirmed = false;  // Only true after sustained detection

void setup() {
  Serial.begin(115200);

  // Initialize sensor pin
  pinMode(SENSOR_PIN, INPUT);

  // Initialize LED ring
  ring.begin();
  ring.setBrightness(BRIGHTNESS);
  ring.clear();
  ring.show();

  Serial.println("Galaxy Watch Dock Ready");
  Serial.println("Using TTP-223 proximity detection");
}

void loop() {
  // Check if sensor is currently triggered (with strict debouncing)
  sensorTriggered = readSensorDebounced();

  unsigned long now = millis();

  // Sustained detection logic
  if (sensorTriggered) {
    if (triggerStartTime == 0) {
      // Just started triggering
      triggerStartTime = now;
    } else if (!watchConfirmed && (now - triggerStartTime >= SUSTAINED_MS)) {
      // Sustained long enough - confirm watch is present
      watchConfirmed = true;
      Serial.println("Watch CONFIRMED after sustained detection");
    }
    lastDetectedTime = now;
  } else {
    // Sensor not triggered
    triggerStartTime = 0;

    // Check if watch was removed (with cooldown)
    if (watchConfirmed && (now - lastDetectedTime >= COOLDOWN_MS)) {
      watchConfirmed = false;
      Serial.println("Watch REMOVED after cooldown");
    }
  }

  if (watchConfirmed) {
    // Breathing effect when watch is confirmed present
    if (now - lastBreathUpdate >= BREATH_SPEED) {
      lastBreathUpdate = now;
      updateBreathing();
    }
  } else {
    // Turn off LEDs when watch is not confirmed
    if (breatheBrightness > 0) {
      ring.clear();
      ring.show();
      breatheBrightness = 0;
      breatheDirection = 1;
    }
  }
}

bool readSensorDebounced() {
  // Read multiple samples for stability
  int highCount = 0;

  for (int i = 0; i < DEBOUNCE_READINGS; i++) {
    if (digitalRead(SENSOR_PIN) == HIGH) {
      highCount++;
    }
    delay(DEBOUNCE_DELAY_MS);
  }

  // Strict threshold - require REQUIRED_HIGH_COUNT of DEBOUNCE_READINGS
  bool detected = (highCount >= REQUIRED_HIGH_COUNT);

  // Debug output
  static bool lastState = false;
  static int lastCount = 0;
  if (detected != lastState || highCount != lastCount) {
    Serial.print("Sensor: ");
    Serial.print(highCount);
    Serial.print("/");
    Serial.print(DEBOUNCE_READINGS);
    Serial.print(" readings HIGH (need ");
    Serial.print(REQUIRED_HIGH_COUNT);
    Serial.println(")");
    lastState = detected;
    lastCount = highCount;
  }

  return detected;
}

void updateBreathing() {
  // Update brightness level
  breatheBrightness += breatheDirection * 2;

  // Reverse direction at limits (capped by BRIGHTNESS)
  if (breatheBrightness >= BRIGHTNESS) {
    breatheBrightness = BRIGHTNESS;
    breatheDirection = -1;
  } else if (breatheBrightness <= 0) {
    breatheBrightness = 0;
    breatheDirection = 1;
  }

  // Calculate dimmed color
  float factor = breatheBrightness / 255.0;
  uint8_t r = baseColor[0] * factor;
  uint8_t g = baseColor[1] * factor;
  uint8_t b = baseColor[2] * factor;

  // Apply to all LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, ring.Color(r, g, b));
  }
  ring.show();
}
