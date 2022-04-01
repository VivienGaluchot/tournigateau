#ifndef PINS_H
#define PINS_H

// ----------------------------------
// const - types
// ----------------------------------

// general io

const int btnPin = 23;
const int ledPin = 22;
const int potPin = 36;

// stepper

// Driver   Gpio
// -------------
// Dir      2
// Step     0
// Clk      4
// Pdn      16
// Nc       17
// Ms2      5
// Ms1      18
// En       19

const int mtrDirPin = 2;
const int mtrStepPin = 0;
const int mtrClkPin = 4;
const int mtrPdnPin = 16;
const int mtrNcPin = 17;
const int mtrMs2Pin = 5;
const int mtrMs1Pin = 18;
const int mtrEnPin = 19;

// pwm channels

const int mtrPwmChannel = 0;

#endif