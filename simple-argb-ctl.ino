#include <FastLED.h>

#define VERSION "1.0.0"
#define BUTTON_PIN 10
#define ARGB_DATA_PIN 3
#define LEDS_NUM 16

CRGB leds[LEDS_NUM];
bool btnIsActive = false;
unsigned long btnLastPressTime = 0;

enum class ButtonStatus {
    Released,
    Press,
    Hold
} buttonStatus = ButtonStatus::Released;

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
    updateButton();
}

void updateButton() {
    const bool buttonRead = digitalRead(BUTTON_PIN);

    if (buttonRead == LOW) {
        // Button considered pressed on LOW because it's connected to ground
        // with internal pullup.
        if (!btnIsActive) {
            btnIsActive = true;
            buttonStatus = ButtonStatus::Press;
            btnLastPressTime = millis();
            Serial.println(F("Button press"));
        } else {
            if (millis() - btnLastPressTime > 250 && buttonStatus != ButtonStatus::Hold) {
                buttonStatus = ButtonStatus::Hold;
                Serial.println(F("Button hold"));
            }
        }
   } else {
        if (btnIsActive) {
            btnIsActive = false;
            buttonStatus = ButtonStatus::Released;
            Serial.println(F("Button released"));
        }
   }
}

// Test the RGB controls
void testRGB() {
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
