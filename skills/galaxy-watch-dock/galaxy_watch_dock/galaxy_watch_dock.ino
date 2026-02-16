/*
 * Galaxy Watch Charging Dock LED Controller
 *
 * Detection: TTP-223 Capacitive Touch Sensor (proximity mode)
 * Effect: Orange/Amber breathing glow
 */

#include <Adafruit_NeoPixel.h>

// Pin definitions
#define LED_PIN     6    // WS2812B data pin
#define SENSOR_PIN  2    // TTP-223 signal pin

// LED configuration
#define NUM_LEDS    7
#define BRIGHTNESS  200  // Increased for more visible breathing

// Color: Orange/Amber (R, G, B)
uint8_t baseColor[3] = {255, 80, 0};

// Breathing effect - SLOWER for more visible effect
#define BREATH_SPEED 25      // ms between steps (higher = slower, more visible)
#define BREATH_MIN   20      // minimum brightness (not fully off)
#define BREATH_MAX   255     // maximum brightness

// Sensitivity - require MORE consecutive readings to trigger
#define READINGS_REQUIRED 50  // must see this many HIGH readings to trigger ON
#define READINGS_TO_CLEAR 60  // must see this many LOW readings to trigger OFF (~3 sec)

Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// State variables
int breatheBrightness = BREATH_MIN;
int breatheDirection = 1;
unsigned long lastBreathUpdate = 0;
unsigned long lastSensorRead = 0;

bool watchDetected = false;
int highCount = 0;   // consecutive HIGH readings
int lowCount = 0;    // consecutive LOW readings

void setup() {
  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);

  ring.begin();
  ring.setBrightness(BRIGHTNESS);
  ring.clear();
  ring.show();

  Serial.println("Galaxy Watch Dock Ready");
  Serial.println("Using TTP-223 proximity detection");
}

void loop() {
  unsigned long now = millis();

  // Read sensor every 50ms (non-blocking)
  if (now - lastSensorRead >= 50) {
    lastSensorRead = now;
    updateSensorState();
  }

  // Update breathing animation
  if (watchDetected) {
    if (now - lastBreathUpdate >= BREATH_SPEED) {
      lastBreathUpdate = now;
      updateBreathing();
    }
  } else {
    // Turn off when not detected
    if (breatheBrightness > BREATH_MIN) {
      ring.clear();
      ring.show();
      breatheBrightness = BREATH_MIN;
      breatheDirection = 1;
    }
  }
}

void updateSensorState() {
  bool currentState = digitalRead(SENSOR_PIN) == HIGH;

  if (currentState) {
    highCount++;
    lowCount = 0;

    // Require multiple HIGH readings to turn ON
    if (!watchDetected && highCount >= READINGS_REQUIRED) {
      watchDetected = true;
      Serial.println("Watch DETECTED - LEDs ON");
    }
  } else {
    lowCount++;
    highCount = 0;

    // Require multiple LOW readings to turn OFF
    if (watchDetected && lowCount >= READINGS_TO_CLEAR) {
      watchDetected = false;
      Serial.println("Watch REMOVED - LEDs OFF");
    }
  }
}

void updateBreathing() {
  // Update brightness
  breatheBrightness += breatheDirection * 3;  // step size

  // Reverse at limits
  if (breatheBrightness >= BREATH_MAX) {
    breatheBrightness = BREATH_MAX;
    breatheDirection = -1;
  } else if (breatheBrightness <= BREATH_MIN) {
    breatheBrightness = BREATH_MIN;
    breatheDirection = 1;
  }

  // Apply color with breathing
  float factor = (float)breatheBrightness / 255.0;
  uint8_t r = baseColor[0] * factor;
  uint8_t g = baseColor[1] * factor;
  uint8_t b = baseColor[2] * factor;

  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, ring.Color(r, g, b));
  }
  ring.show();
}
