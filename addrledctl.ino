#include <EEPROM.h>
#include <FastLED.h>

#define NAME "addrledctl"
#define VERSION "1.1.0"
#define SERIAL_ON true
#define SERIAL_BAUD 9600
#define EEPROM_FORCE_REWRITE false
#define EEPROM_MAGIC 69

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

struct Configuration {
    uint8_t magic;
    CRGB color;
    bool isOn;
};

class Button {
private:
    ButtonMode _mode;
    unsigned long _lastPressTime;

    void _setMode(ButtonMode mode) {
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
            } else if (currentMode != ButtonMode::Idle
                       || currentMode == ButtonMode::Hold)
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
    Configuration _configuration;
    Button *_pbutton;
    CRGB _leds[LEDS_NUM];
    uint16_t _ledIterator;

    void _setMode(ControllerMode mode) {
        this->_previousMode = this->_mode;
        this->_mode = mode;

        #if SERIAL_ON
        Serial.print(F("[ctrl] Mode: "));
        Serial.println((uint16_t) mode);
        #endif
    }

    void _bumpLEDIterator() {
        unsigned long btnHoldTime = this->_pbutton->getTimeFromLastPress();
        // Progressive acceleration precision
        int step = map(
            constrain(btnHoldTime, 0 , 3000),
            0, 4000,
            1, 5
        );

        // Bump the iterator ensuring it fits under 256 (255 colors) limit
        this->_ledIterator = (this->_ledIterator + step) % 256;

        // Progressive delay decrease on button hold
        delay(map(
            constrain(btnHoldTime, 0, 3000),
            0, 4000,
            100, 50
        ));
    }

    void _clearLEDIterator() {
        this->_ledIterator = 0;
    }

    void _initConfiguration() {
        this->_configuration.magic = EEPROM_MAGIC;
        this->_configuration.isOn = true;
        this->_configuration.color.setRGB(128, 128, 128);
    }

    void _readEEPROM() {
        EEPROM.get(0, this->_configuration);
        uint8_t memMagic = this->_configuration.magic;
        Serial.println(F("[eemem] Config read"));

        if (memMagic != EEPROM_MAGIC || EEPROM_FORCE_REWRITE) {
            this->_initConfiguration();
            this->_writeEEPROM();

            #if SERIAL_ON
            #if EEPROM_FORCE_REWRITE
            Serial.println(F("[eemem] Forced overwrite"));
            #else
            Serial.println(F("[eemem] Corruption fixed"));
            Serial.print(F("\tMagic: ")); Serial.println(memMagic);
            #endif
            #endif
        }
    };

    void _writeEEPROM() {
        EEPROM.put(0, this->_configuration);
        #if SERIAL_ON
        Serial.println(F("[eemem] Values set"));
        #endif
    };

public:
    void setup(Button *button) {
        this->_pbutton = button;
        this->_setMode(ControllerMode::Idle);
        this->_initConfiguration();
        this->_readEEPROM();

        pinMode(ARGB_DATA_PIN, OUTPUT);
        FastLED.addLeds<NEOPIXEL, ARGB_DATA_PIN>(this->_leds, LEDS_NUM);
        fill_solid(this->_leds, LEDS_NUM, this->_configuration.color);
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
                this->_configuration.color.r = this->_ledIterator;
                this->_bumpLEDIterator();
                fill_solid(this->_leds+1, LEDS_NUM-1, this->_configuration.color);
                FastLED.show();
                break;
            case ControllerMode::LEDConfigure_G:
                this->_configuration.color.g = this->_ledIterator;
                this->_bumpLEDIterator();
                fill_solid(this->_leds+1, LEDS_NUM-1, this->_configuration.color);
                FastLED.show();
                break;
            case ControllerMode::LEDConfigure_B:
                this->_configuration.color.b = this->_ledIterator;
                this->_bumpLEDIterator();
                fill_solid(this->_leds+1, LEDS_NUM-1, this->_configuration.color);
                FastLED.show();
                break;
            }
            break;

        case ButtonMode::Idle:
            if (this->_pbutton->getTimeFromLastPress() >= 10000
                && currentControllerMode != ControllerMode::Idle)
            {
                this->_setMode(ControllerMode::Idle);
                fill_solid(this->_leds, LEDS_NUM, this->_configuration.color);
                FastLED.show();

                this->_writeEEPROM();
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
