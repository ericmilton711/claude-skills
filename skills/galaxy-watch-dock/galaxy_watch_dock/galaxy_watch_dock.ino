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
#define BRIGHTNESS  150  // 0-255

// Color: Orange/Amber (R, G, B)
uint8_t baseColor[3] = {255, 80, 0};

// Breathing effect settings
#define BREATH_SPEED 10  // ms between brightness steps (lower = faster)

// Debounce settings to prevent flicker
#define DEBOUNCE_READINGS 5
#define DEBOUNCE_DELAY_MS 20

Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// State variables
int breatheBrightness = 0;
int breatheDirection = 1;  // 1 = brighter, -1 = dimmer
unsigned long lastBreathUpdate = 0;
bool watchDetected = false;

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
  // Check if watch is detected (with debouncing)
  watchDetected = readSensorDebounced();

  if (watchDetected) {
    // Breathing effect when watch is present
    if (millis() - lastBreathUpdate >= BREATH_SPEED) {
      lastBreathUpdate = millis();
      updateBreathing();
    }
  } else {
    // Turn off LEDs when watch is removed
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

  // Majority vote
  bool detected = (highCount > DEBOUNCE_READINGS / 2);

  // Debug output (comment out for production)
  static bool lastState = false;
  if (detected != lastState) {
    Serial.print("Watch ");
    Serial.println(detected ? "DETECTED - LEDs ON" : "REMOVED - LEDs OFF");
    lastState = detected;
  }

  return detected;
}

void updateBreathing() {
  // Update brightness level
  breatheBrightness += breatheDirection * 2;

  // Reverse direction at limits
  if (breatheBrightness >= 255) {
    breatheBrightness = 255;
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
