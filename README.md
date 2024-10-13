## About
addrledctl - **Add**ressable **LED** **C**on**t**ro**l**, is a simple arduino
firmware that allows you to control addressable RGB LEDs with the press of just
one button.


## Features
- Static lighting only
- Save state to EEPROM and restore on boot
- Only one button required for control


## Controls
Long press the button no enter the configuration mode. You'll see the current
selected color (R,G,B) on the first 4 LEDs. LEDs after the first 4 will show
the final result of RGB. Hold the button to start the iteration through the
selected color, and release when the desired color is achieved. Single pressing
the button in configuration mode will switch the current selected color. System
will exit from configuration and save the selected values to EEPROM after 10
seconds of button inactivity.


## Assembly
As this project meant for PC builds, the stabilized 5v are expected to come
directly from PCs PSU (SATA or Molex) to 5v and GND pins of Arduino board. Just
be sure to unplug the LED strip or place a diode on board 5v pin to make sure
that board will not power the LEDs by any accident (when flashing, and it's
powered by USB for example).

This is very basic, but I'll still want to point out that ground should be
shared between the Arduino board and LED strip to send the signal.

All pins are defined in human readable format with preprocessor `#define`s in
main sketch file.

When connecting the ARGB data pin, it's recommended to use the ~320 Ohm
resistor to limit the signal amplitude.


## Build
Project is configured to be used with `arduinocli`.

- To build from source:
```sh
arduino-cli compile
```

- Upload to board with:
```sh
arduino upload -p <PORT>
```
