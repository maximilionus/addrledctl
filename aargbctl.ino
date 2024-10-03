#include <FastLED.h>

#define NAME "aargbctl"
#define VERSION "1.0.0"
#define DEBUG // Logging status to serial.
              // Comment out to disable.

#define BUTTON_PIN 10
#define ARGB_DATA_PIN 3
#define LEDS_NUM 16

enum class SystemMode {
    Idle,
    LEDConfigure_R,
    LEDConfigure_G,
    LEDConfigure_B
};

enum class ButtonMode {
    Idle,
    PressDown,
    PressUp,
    Hold
};

class Button {
private:
    ButtonMode _mode;
    unsigned long _lastPressTime;

    void _setMode(ButtonMode mode) {
        this->_mode = mode;

        if (mode == ButtonMode::PressDown \
            || mode == ButtonMode::PressUp)
        {
            this->_updateLastPress();
            #ifdef DEBUG
            Serial.println(F("[btn] Upd press time"));
            #endif
        }

        #ifdef DEBUG
        Serial.print(F("[btn] Mode: "));
        Serial.println((uint16_t) mode);
        #endif
    }

    void _updateLastPress() {
        this->_lastPressTime = millis();
    }

public:
    void setup() {
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        this->_setMode(ButtonMode::Idle);
    }

    bool isPressed() {
        // Button considered pressed on LOW because it's connected to ground
        // with internal pullup.
        return !digitalRead(BUTTON_PIN);
    }

    ButtonMode getMode() {
        return this->_mode;
    }

    unsigned long getTimeFromLastPress() {
        return millis() - this->_lastPressTime;
    }

    void tick() {
        const ButtonMode currentMode = this->getMode();

        if (this->isPressed()) {
            if (currentMode == ButtonMode::Idle) {
                this->_setMode(ButtonMode::PressDown);
            } else if (currentMode == ButtonMode::PressDown) {
                if (this->getTimeFromLastPress() > 250 && currentMode != ButtonMode::Hold) {
                    this->_setMode(ButtonMode::Hold);
                }
            }
       } else {
            if (currentMode == ButtonMode::PressDown) {
                this->_setMode(ButtonMode::PressUp);
            } else if (currentMode != ButtonMode::Idle) {
                this->_setMode(ButtonMode::Idle);
            }
       }
    }
};

SystemMode systemMode;
Button button;
CRGB leds[LEDS_NUM];
uint8_t rgbIter = 0;

void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    Serial.println(F(NAME));
    Serial.print(F("ver: ")); Serial.println(F(VERSION));
    Serial.print(F("rev: ")); Serial.println(F(__DATE__ " " __TIME__));
    #endif

    pinMode(LED_BUILTIN, OUTPUT);  // DEBUG
    pinMode(ARGB_DATA_PIN, OUTPUT);

    button.setup();

    FastLED.addLeds<NEOPIXEL, ARGB_DATA_PIN>(leds, LEDS_NUM);
    FastLED.clear();
    FastLED.show();

    systemMode = SystemMode::Idle;

    #ifdef DEBUG
    Serial.println(F("Setup done"));
    #endif
}

void loop() {
    button.tick();
    //updateMode();
}

void updateMode() {
    if (button.getMode() == ButtonMode::PressUp) {
        if (systemMode == SystemMode::Idle \
            || systemMode == SystemMode::LEDConfigure_B) // Cycle through R,G,B
        {
            systemMode = SystemMode::LEDConfigure_R;
            fill_solid(leds, 4, CRGB::Blue);
            FastLED.show();

            #ifdef DEBUG
            Serial.println(F("[SysM] Conf LED R"));
            #endif
        } else if (systemMode == SystemMode::LEDConfigure_R) {
            systemMode = SystemMode::LEDConfigure_G;
            fill_solid(leds, 4, CRGB::Blue);
            FastLED.show();

            #ifdef DEBUG
            Serial.println(F("[SysM] Conf LED G"));
            #endif
        } else if (systemMode == SystemMode::LEDConfigure_G) {
            systemMode = SystemMode::LEDConfigure_B;
            fill_solid(leds, 4, CRGB::Blue);
            FastLED.show();

            #ifdef DEBUG
            Serial.println(F("[SysM] Conf LED B"));
            #endif
        }

        // Hacky way to prevent ghost input on single press
        //delay(250);
    } else if (button.getMode() == ButtonMode::Hold) {
        if (systemMode == SystemMode::LEDConfigure_R) {

        }
    } else if (button.getMode() == ButtonMode::Idle) {
        if (button.getTimeFromLastPress() >= 5000 \
            && systemMode != SystemMode::Idle)
        {
            systemMode = SystemMode::Idle;
            #ifdef DEBUG
            Serial.println(F("[SysM] Idle"));
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
