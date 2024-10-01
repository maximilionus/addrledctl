#include <FastLED.h>

#define VERSION "1.0.0"
#define BUTTON_PIN 10
#define ARGB_DATA_PIN 3
#define LEDS_NUM 16

CRGB leds[LEDS_NUM];

void setup() {
    Serial.begin(9600);
    Serial.print(F("simple-argb-ctl, ver: "));
    Serial.print(F(VERSION));
    Serial.print(F(", rev: "));
    Serial.println(__DATE__ " " __TIME__);

    pinMode(LED_BUILTIN, OUTPUT);  // DEBUG
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(ARGB_DATA_PIN, OUTPUT);

    FastLED.addLeds<NEOPIXEL, ARGB_DATA_PIN>(leds, LEDS_NUM);
}

void loop() {
    const bool button_state = digitalRead(BUTTON_PIN);
    // Test the RGB controls
    if (button_state == LOW) {
        for (uint16_t i = 0; i < LEDS_NUM; i++) {
            leds[i] = CRGB::Red;
        }

        FastLED.show();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(2000);

        for (uint16_t i = 0; i < LEDS_NUM; i++) {
            leds[i] = CRGB::Green;
        }

        FastLED.show();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(2000);

        for (uint16_t i = 0; i < LEDS_NUM; i++) {
            leds[i] = CRGB::Blue;
            FastLED.show();
            delay(2000);
        }

        FastLED.show();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(2000);
    }
}
