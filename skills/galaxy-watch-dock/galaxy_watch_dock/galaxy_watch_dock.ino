/*
 * Galaxy Watch Charging Dock LED Controller
 *
 * Hardware:
 *   - Arduino Nano (ATmega328P)
 *   - WS2812B 7-LED Ring
 *   - INA219 Current Sensor Module
 *
 * Wiring:
 *   Nano 5V  -> LED Ring VCC
 *   Nano 5V  -> INA219 VCC
 *   Nano GND -> LED Ring GND
 *   Nano GND -> INA219 GND
 *   Nano D6  -> LED Ring DIN
 *   Nano A4  -> INA219 SDA
 *   Nano A5  -> INA219 SCL
 *
 *   INA219 VIN+ -> USB 5V (from power source)
 *   INA219 VIN- -> Samsung Charger VCC
 *   Samsung Charger GND -> USB GND (from power source)
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     6      // Data pin connected to LED ring
#define NUM_LEDS    7      // 7-bit LED ring
#define BRIGHTNESS  150    // 0-255, adjust to taste

// Current thresholds in mA with hysteresis for stable detection
// Watch charging typically draws 200-500mA
#define CURRENT_ON_THRESHOLD_MA   50.0   // Turn ON when current exceeds this
#define CURRENT_OFF_THRESHOLD_MA  30.0   // Turn OFF when current drops below this

// Number of readings to average for stable detection
#define CURRENT_SAMPLES  5

Adafruit_INA219 ina219;
Adafruit_NeoPixel ring(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Colors (R, G, B)
uint8_t baseColor[3] = {255, 80, 0};  // Orange/Amber - warm glow

// Breathing state
int breatheBrightness = 0;
int breatheDirection = 1;  // 1 = getting brighter, -1 = getting dimmer

// Timing
unsigned long lastBreathUpdate = 0;
unsigned long lastCurrentCheck = 0;
const int BREATH_SPEED = 10;        // ms between brightness steps (lower = faster)
const int CURRENT_CHECK_INTERVAL = 500;  // ms between current readings

bool isCharging = false;

void setup() {
  Serial.begin(115200);

  // Initialize INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    // Continue anyway - LEDs will stay off
  }

  // Initialize LED ring
  ring.begin();
  ring.setBrightness(BRIGHTNESS);
  ring.clear();
  ring.show();

  Serial.println("Galaxy Watch Dock Ready");
}

void loop() {
  unsigned long currentTime = millis();

  // Check current periodically
  if (currentTime - lastCurrentCheck >= CURRENT_CHECK_INTERVAL) {
    lastCurrentCheck = currentTime;
    checkCharging();
  }

  // Update LEDs
  if (isCharging) {
    // Breathing effect while charging
    if (currentTime - lastBreathUpdate >= BREATH_SPEED) {
      lastBreathUpdate = currentTime;
      updateBreathing();
    }
  } else {
    // Turn off LEDs when not charging
    if (breatheBrightness > 0) {
      ring.clear();
      ring.show();
      breatheBrightness = 0;
      breatheDirection = 1;
    }
  }
}

void checkCharging() {
  // Average multiple readings for stability
  float totalCurrent = 0;
  for (int i = 0; i < CURRENT_SAMPLES; i++) {
    totalCurrent += ina219.getCurrent_mA();
    delay(10);
  }
  float current_mA = totalCurrent / CURRENT_SAMPLES;

  // Debug output
  Serial.print("Current (avg): ");
  Serial.print(current_mA);
  Serial.println(" mA");

  // Hysteresis: different thresholds for turning on vs off
  // Prevents flickering when current is near the threshold
  bool wasCharging = isCharging;

  if (!isCharging && current_mA > CURRENT_ON_THRESHOLD_MA) {
    // Was off, current exceeded ON threshold -> turn on
    isCharging = true;
  } else if (isCharging && current_mA < CURRENT_OFF_THRESHOLD_MA) {
    // Was on, current dropped below OFF threshold -> turn off
    isCharging = false;
  }
  // Otherwise, keep current state (hysteresis zone between 30-50mA)

  if (isCharging && !wasCharging) {
    Serial.println("Charging started - LEDs ON");
  } else if (!isCharging && wasCharging) {
    Serial.println("Charging stopped - LEDs OFF");
  }
}

void updateBreathing() {
  // Update brightness
  breatheBrightness += breatheDirection * 2;

  // Reverse direction at limits
  if (breatheBrightness >= 255) {
    breatheBrightness = 255;
    breatheDirection = -1;
  } else if (breatheBrightness <= 0) {
    breatheBrightness = 0;
    breatheDirection = 1;
  }

  // Apply to LEDs
  float factor = breatheBrightness / 255.0;
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, ring.Color(
      baseColor[0] * factor,
      baseColor[1] * factor,
      baseColor[2] * factor
    ));
  }
  ring.show();
}
