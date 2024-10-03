#include <FastLED.h>

#define NAME "argbctl"
#define VERSION "1.0.0"
#define DEBUG // Logging status to serial.
              // Comment out to disable.

#define BUTTON_PIN 10
#define ARGB_DATA_PIN 3
#define LEDS_NUM 16

CRGB leds[LEDS_NUM];
bool buttonIsActive = false;
unsigned long buttonLastPressTime = 0;

enum class ButtonStatus {
    Released,
    Press,
    Hold
} buttonStatus = ButtonStatus::Released;

void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    Serial.println(F(NAME));
    Serial.print(F("ver: ")); Serial.println(F(VERSION));
    Serial.print(F("rev: ")); Serial.println(F(__DATE__ " " __TIME__));
    #endif

    pinMode(LED_BUILTIN, OUTPUT);  // DEBUG
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(ARGB_DATA_PIN, OUTPUT);

    FastLED.addLeds<NEOPIXEL, ARGB_DATA_PIN>(leds, LEDS_NUM);
    FastLED.clear();
    FastLED.show();

    #ifdef DEBUG
    Serial.println(F("Setup done"));
    #endif
}

void loop() {
    updateButton();
}

void updateButton() {
    const bool buttonRead = digitalRead(BUTTON_PIN);

    if (buttonRead == LOW) {
        // Button considered pressed on LOW because it's connected to ground
        // with internal pullup.
        if (!buttonIsActive) {
            buttonIsActive = true;
            buttonStatus = ButtonStatus::Press;
            buttonLastPressTime = millis();

            #ifdef DEBUG
            Serial.println(F("[btn] Press"));
            #endif
        } else {
            if (millis() - buttonLastPressTime > 250 && buttonStatus != ButtonStatus::Hold) {
                buttonStatus = ButtonStatus::Hold;
                #ifdef DEBUG
                Serial.println(F("[btn] Hold"));
                #endif
            }
        }
   } else {
        if (buttonIsActive) {
            buttonIsActive = false;
            buttonStatus = ButtonStatus::Released;
            #ifdef DEBUG
            Serial.println(F("[btn] Released"));
            #endif
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
