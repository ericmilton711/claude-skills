#ifndef CONFIG_H
#define CONFIG_H

// LED Strip Configuration
#define LED_PIN         5       // GPIO pin connected to the LED strip data line
#define NUM_LEDS        60      // Number of LEDs in your strip
#define LED_TYPE        WS2812B // LED strip type (WS2812B, WS2811, SK6812, etc.)
#define COLOR_ORDER     GRB     // Color order (GRB is common for WS2812B)

// Brightness (0-255) - lower = longer battery life
#define DEFAULT_BRIGHTNESS  80

// Animation speeds (lower = faster)
#define ANIMATION_SPEED     50

// PIR Motion Sensor (HC-SR501) - battery friendly
#define PIR_PIN             GPIO_NUM_13  // Must be RTC GPIO for deep sleep wake
#define LED_ON_TIME         15000        // LEDs stay on for 15 seconds after motion

// Deep Sleep - for battery operation
#define DEEP_SLEEP_ENABLED  true

// WiFi Configuration (disabled for battery savings)
#define WIFI_ENABLED    false
#define WIFI_SSID       "YourWiFiSSID"
#define WIFI_PASSWORD   "YourWiFiPassword"

#endif
