#ifndef IO_H
#define IO_H

#include <stdint.h>

namespace io {

// ----------------------------------
// const - types
// ----------------------------------

// ----------------------------------
// variables
// ----------------------------------

// state of the button
extern bool btnIsPressed;

// true when state of the button has changed in the last read
extern bool btnHasSwitched;

// value read from the potentiometer in [0,4096]
extern uint16_t potValue;

// value read from the potentiometer in [0;1]
extern float potRate;

// value read from the potentiometer in [-1;1] with a dead zone in center
extern float potRateCentered;

// true when the last value from the potentiometer has changed in the last read
extern bool potHasChanged;

// ----------------------------------
// services
// ----------------------------------

void initialize();

void doCycle(uint32_t timeInMs);

void setLed(bool isOn);

void setLedBlink(uint32_t blinkPeriodInMs);

}  // namespace io

#endif