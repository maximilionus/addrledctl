#include <EEPROM.h>
#include <FastLED.h>

#define NAME "addrledctl"
#define VERSION "1.0.0"
#define SERIAL_ON true // Logging status to serial.
#define SERIAL_BAUD 9600
#define EEPROM_ON true // Use EEPROM to store the user settings

#define BUTTON_PIN 10
#define ARGB_DATA_PIN 3
#define LEDS_NUM 16

enum class ControllerMode {
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
    ButtonMode _previousMode;
    unsigned long _lastPressTime;

    void _setMode(ButtonMode mode) {
        this->_previousMode = this->_mode;
        this->_mode = mode;

        if (mode == ButtonMode::PressDown
            || mode == ButtonMode::PressUp)
        {
            this->_updateLastPress();
            #if SERIAL_ON
            Serial.println(F("[btn] Upd press time"));
            #endif
        }

        #if SERIAL_ON
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

    ButtonMode getPreviousMode() {
        return this->_previousMode;
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
                this->_updateLastPress();
            } else if (
                currentMode != ButtonMode::Idle
                || currentMode == ButtonMode::Hold
            )
            {
                this->_setMode(ButtonMode::Idle);
            }
       }
    }
};

class Controller {
private:
    ControllerMode _mode;
    ControllerMode _previousMode;
    Button *_pbutton;
    CRGB _leds[LEDS_NUM];
    CRGB _globalColor;
    int16_t _ledIterator = 0;
    int16_t _ledIteratorDirection = 1;

    void _setMode(ControllerMode mode) {
        this->_previousMode = this->_mode;
        this->_mode = mode;

        #if SERIAL_ON
        Serial.print(F("[ctlr] Mode: "));
        Serial.println((uint16_t) mode);
        #endif
    }

    void _bumpLEDIterator() {
        this->_ledIterator += this->_ledIteratorDirection;

        if (this->_ledIterator > 255) {
            this->_ledIterator = 255;
            this->_invertLEDIteratorDirection();
        } else if (this->_ledIterator < 0) {
            this->_ledIterator = 0;
            this->_invertLEDIteratorDirection();
        }

        #if SERIAL_ON
        Serial.print(F("[ctlr] LED value: "));
        Serial.println(this->_ledIterator);
        #endif

        // Way to improve color input accuracy
        delay(15);
    }

    void _clearLEDIterator() {
        this->_ledIterator = 0;
    }

    void _invertLEDIteratorDirection() {
        this->_ledIteratorDirection *= -1;
    }

public:
    void setup(Button *button) {
        this->_pbutton = button;
        this->_setMode(ControllerMode::Idle);
        this->_globalColor.setRGB(0,0,0);

        #if EEPROM_ON
        EEPROM.get(0, this->_globalColor);
        #if SERIAL_ON
        Serial.println(F("[eemem] Values read"));
        #endif
        #endif

        pinMode(ARGB_DATA_PIN, OUTPUT);
        FastLED.addLeds<NEOPIXEL, ARGB_DATA_PIN>(this->_leds, LEDS_NUM);
        fill_solid(this->_leds, LEDS_NUM, this->_globalColor);
        FastLED.show();
    }

    ControllerMode getMode() {
        return this->_mode;
    }

    ControllerMode getPreviousMode() {
        return this->_previousMode;
    }

    void tick() {
        ControllerMode currentControllerMode = this->getMode();
        ButtonMode currentButtonMode = this->_pbutton->getMode();

        switch (currentButtonMode) {
        case ButtonMode::PressUp:
            if (currentControllerMode == ControllerMode::Idle
                || currentControllerMode == ControllerMode::LEDConfigure_B) // Cycle through R,G,B
            {
                this->_setMode(ControllerMode::LEDConfigure_R);
                this->_clearLEDIterator();
                fill_solid(this->_leds, 1, CRGB::Red);
                FastLED.show();
            } else if (currentControllerMode == ControllerMode::LEDConfigure_R) {
                this->_setMode(ControllerMode::LEDConfigure_G);
                this->_clearLEDIterator();
                fill_solid(this->_leds, 1, CRGB::Green);
                FastLED.show();
            } else if (currentControllerMode == ControllerMode::LEDConfigure_G) {
                this->_setMode(ControllerMode::LEDConfigure_B);
                this->_clearLEDIterator();
                fill_solid(this->_leds, 1, CRGB::Blue);
                FastLED.show();
            }
            break;

        case ButtonMode::Hold:
            switch (currentControllerMode) {
            case ControllerMode::LEDConfigure_R:
                this->_globalColor.r = this->_ledIterator;
                this->_bumpLEDIterator();
                fill_solid(this->_leds+1, LEDS_NUM-1, this->_globalColor);
                FastLED.show();
                break;
            case ControllerMode::LEDConfigure_G:
                this->_globalColor.g = this->_ledIterator;
                this->_bumpLEDIterator();
                fill_solid(this->_leds+1, LEDS_NUM-1, this->_globalColor);
                FastLED.show();
                break;
            case ControllerMode::LEDConfigure_B:
                this->_globalColor.b = this->_ledIterator;
                this->_bumpLEDIterator();
                fill_solid(this->_leds+1, LEDS_NUM-1, this->_globalColor);
                FastLED.show();
                break;
            }
            break;

        case ButtonMode::Idle:
            if (currentControllerMode != ControllerMode::Idle) {
                if (this->_pbutton->getTimeFromLastPress() >= 10000) {
                    this->_setMode(ControllerMode::Idle);
                    fill_solid(this->_leds, LEDS_NUM, this->_globalColor);
                    FastLED.show();

                    #if EEPROM_ON
                    EEPROM.put(0, this->_globalColor);

                    #if SERIAL_ON
                    Serial.println(F("[eemem] Values updated"));
                    #endif
                    #endif
                } else if (this->_pbutton->getPreviousMode() == ButtonMode::Hold)
                {
                    this->_invertLEDIteratorDirection();
                }
            }
            break;
        }
    }
};

Controller controller;
Button button;

void setup() {
    #if SERIAL_ON
    Serial.begin(SERIAL_BAUD);
    Serial.println(F(NAME));
    Serial.print(F("ver: ")); Serial.println(F(VERSION));
    Serial.print(F("rev: ")); Serial.println(F(__DATE__ " " __TIME__));
    #endif

    button.setup();
    controller.setup(&button);

    #if SERIAL_ON
    Serial.println(F("Setup done"));
    #endif
}

void loop() {
    button.tick();
    controller.tick();
}
