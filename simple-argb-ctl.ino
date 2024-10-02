#include <FastLED.h>

#define VERSION "1.0.0"
#define BUTTON_PIN 10
#define ARGB_DATA_PIN 3
#define LEDS_NUM 16

CRGB leds[LEDS_NUM];
unsigned long btnLastPressTime = 0;

enum class ButtonStatus {
    Released,
    Press,
    Hold
};

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
    FastLED.clear();
    FastLED.show();

    Serial.println(F("Setup done"));
}

void loop() {
    // Process button input
    // TODO: Probably needs to be reworked into a split fnc
    const bool buttonRead = digitalRead(BUTTON_PIN);
    ButtonStatus buttonStatus = ButtonStatus::Released;

    if (buttonRead == LOW) {
        const unsigned long btnTimeFromLastPress = millis() - btnLastPressTime;
        if (btnTimeFromLastPress <= 10) {
            buttonStatus = ButtonStatus::Hold;
        } else if (btnTimeFromLastPress <= 1000) {
            buttonStatus = ButtonStatus::Press;
        }
    }

    // Test the RGB controls
    for (uint8_t i = 0; i < LEDS_NUM; i++) {
        leds[i] = CRGB::Red;
    }

    FastLED.show();
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);

    for (uint8_t i = 0; i < LEDS_NUM; i++) {
        leds[i] = CRGB::Green;
    }

    FastLED.show();
    delay(2000);

    for (uint8_t i = 0; i < LEDS_NUM; i++) {
        leds[i] = CRGB::Blue;
        FastLED.show();
        delay(100);
    }
}
