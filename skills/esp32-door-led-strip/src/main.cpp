#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

CRGB leds[NUM_LEDS];

// Track when motion was detected
unsigned long motionStartTime = 0;

// Function declarations
void solidColor(CRGB color);
void rainbow();
void turnOnEffect();
void turnOffEffect();
void goToSleep();

void setup() {
    Serial.begin(115200);

    // Check wake reason
    esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();

    if (wakeReason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Woke up from motion!");
    } else {
        Serial.println("ESP32 Door LED Strip - Battery Mode");
        Serial.println("PIR sensor on GPIO 13");
    }

    // Initialize LED strip
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    // Initialize PIR pin
    pinMode(PIR_PIN, INPUT);

    // Run turn-on effect
    turnOnEffect();
    motionStartTime = millis();

    Serial.println("LEDs on - will sleep after timeout");
}

void loop() {
    // Check if PIR still detects motion - reset timer
    if (digitalRead(PIR_PIN) == HIGH) {
        motionStartTime = millis();
    }

    // Run animation while active
    rainbow();

    // Check for timeout
    if (millis() - motionStartTime > LED_ON_TIME) {
        Serial.println("No motion - going to sleep");
        turnOffEffect();
        goToSleep();
    }
}

void goToSleep() {
    // Turn off LEDs completely
    FastLED.clear();
    FastLED.show();

    delay(100);

    #if DEEP_SLEEP_ENABLED
    // Configure wake on PIR pin going HIGH
    esp_sleep_enable_ext0_wakeup(PIR_PIN, 1);

    Serial.println("Entering deep sleep...");
    Serial.flush();

    // Enter deep sleep - ~10µA power consumption
    esp_deep_sleep_start();
    #endif
}

void solidColor(CRGB color) {
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

void rainbow() {
    static uint8_t hue = 0;
    fill_rainbow(leds, NUM_LEDS, hue, 7);
    FastLED.show();
    hue++;
    delay(ANIMATION_SPEED);
}

void turnOnEffect() {
    FastLED.clear();
    int center = NUM_LEDS / 2;

    for (int i = 0; i <= center; i++) {
        leds[i] = CRGB::Blue;
        leds[NUM_LEDS - 1 - i] = CRGB::Blue;
        FastLED.show();
        delay(8);
    }
}

void turnOffEffect() {
    for (int b = DEFAULT_BRIGHTNESS; b >= 0; b -= 10) {
        FastLED.setBrightness(b);
        FastLED.show();
        delay(15);
    }
    FastLED.clear();
    FastLED.show();
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}
