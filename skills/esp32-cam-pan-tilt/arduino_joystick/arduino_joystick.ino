/*
 * Arduino Joystick Controller for ESP32-CAM Pan-Tilt
 *
 * Reads joystick and sends serial commands to ESP32
 *
 * Wiring:
 *   Joystick VRx -> A0
 *   Joystick VRy -> A1
 *   Joystick SW  -> D2 (optional, for center button)
 *   Arduino D11  -> ESP32 GPIO13
 *   Arduino GND  -> ESP32 GND
 */

#include <SoftwareSerial.h>

#define JOY_X A0
#define JOY_Y A1
#define JOY_BTN 2  // Optional center button
#define SOFT_TX 11 // Use pin 11 for sending to ESP32

// RX pin 10 is unused but required by SoftwareSerial
SoftwareSerial espSerial(10, SOFT_TX);

// Dead zone to prevent drift (joystick center is ~512)
#define DEAD_ZONE_LOW 450
#define DEAD_ZONE_HIGH 574

// How often to send commands (ms)
#define SEND_INTERVAL 100

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);   // For debugging via USB
  espSerial.begin(9600);  // Communication with ESP32 (9600 is reliable for SoftwareSerial)
  pinMode(JOY_BTN, INPUT_PULLUP);
  Serial.println("Joystick ready");
}

void loop() {
  // Check center button
  if (digitalRead(JOY_BTN) == LOW) {
    espSerial.println("C");  // Center command
    Serial.println("Sent: C (center)");
    delay(300);  // Debounce
    return;
  }

  // Rate limit sending
  if (millis() - lastSend < SEND_INTERVAL) {
    return;
  }
  lastSend = millis();

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  // Check X axis (pan)
  if (x < DEAD_ZONE_LOW) {
    espSerial.println("L");
    Serial.println("Sent: L (left)");
  } else if (x > DEAD_ZONE_HIGH) {
    espSerial.println("R");
    Serial.println("Sent: R (right)");
  }

  // Check Y axis (tilt)
  if (y < DEAD_ZONE_LOW) {
    espSerial.println("U");
    Serial.println("Sent: U (up)");
  } else if (y > DEAD_ZONE_HIGH) {
    espSerial.println("D");
    Serial.println("Sent: D (down)");
  }
}
