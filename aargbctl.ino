#include <FastLED.h>

#define NAME "aargbctl"
#define VERSION "1.0.0"
#define DEBUG // Logging status to serial.
              // Comment out to disable.

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

class Controller {
private:
    ControllerMode _mode;
    ControllerMode _previousMode;
    Button *_pbutton;
    CRGB _leds[LEDS_NUM];

    void _setMode(ControllerMode mode) {
        this->_previousMode = this->_mode;
        this->_mode = mode;

        #ifdef DEBUG
        Serial.print(F("[ctlr] Mode: "));
        Serial.println((uint16_t) mode);
        #endif
    }

public:
    void setup(Button *button) {
        this->_pbutton = button;
        this->_setMode(ControllerMode::Idle);

        pinMode(ARGB_DATA_PIN, OUTPUT);
        FastLED.addLeds<NEOPIXEL, ARGB_DATA_PIN>(this->_leds, LEDS_NUM);
        FastLED.clear();
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
            if (currentControllerMode == ControllerMode::Idle \
                || currentControllerMode == ControllerMode::LEDConfigure_B) // Cycle through R,G,B
            {
                this->_setMode(ControllerMode::LEDConfigure_R);
                fill_solid(this->_leds, 4, CRGB::Blue);
                FastLED.show();
            } else if (currentControllerMode == ControllerMode::LEDConfigure_R) {
                this->_setMode(ControllerMode::LEDConfigure_G);
                fill_solid(this->_leds, 4, CRGB::Blue);
                FastLED.show();
            } else if (currentControllerMode == ControllerMode::LEDConfigure_G) {
                this->_setMode(ControllerMode::LEDConfigure_B);
                fill_solid(this->_leds, 4, CRGB::Blue);
                FastLED.show();
            }
            break;

        case ButtonMode::Hold:
            // TODO: RGB values configuration planned here
            break;

        case ButtonMode::Idle:
            if (this->_pbutton->getTimeFromLastPress() >= 5000 \
                && currentControllerMode != ControllerMode::Idle)
            {
                this->_setMode(ControllerMode::Idle);
            }
            break;
        }
    }
};

Controller controller;
Button button;

void setup() {
    #ifdef DEBUG
    Serial.begin(9600);
    Serial.println(F(NAME));
    Serial.print(F("ver: ")); Serial.println(F(VERSION));
    Serial.print(F("rev: ")); Serial.println(F(__DATE__ " " __TIME__));
    #endif

    button.setup();
    controller.setup(&button);

    #ifdef DEBUG
    Serial.println(F("Setup done"));
    #endif
}

void loop() {
    button.tick();
    controller.tick();
}
