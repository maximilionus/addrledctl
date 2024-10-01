#include <FastLED.h>

#define VERSION "1.0.0"
#define BUTTON_PIN 10
#define ARGB_DATA_PIN 5
#define LEDS_NUM 1

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
    // Test the RGB controls
    leds[0] = CRGB::Red;
    FastLED.show();
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    leds[0] = CRGB::Black;
    FastLED.show();
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
}
