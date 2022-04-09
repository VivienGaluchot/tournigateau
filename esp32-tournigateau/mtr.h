#ifndef MTR_H
#define MTR_H

#include <stdint.h>

namespace mtr {

// ----------------------------------
// const - types
// ----------------------------------

// ----------------------------------
// variables
// ----------------------------------

// current motor rmp
extern float currentRpm;

extern int32_t rotateCurrentStep;

// true when the dirver is enabled
extern bool isDriverEnabled;

// true when a rotate command is running
extern bool isRotating;

// ----------------------------------
// services
// ----------------------------------

void initialize();

void doCycle(uint32_t timeInMs);

void rampSetup(uint32_t timeInMs, float toRmp, float rmpPerSec);

void resetAbsReference();

void startRotateToAbs(float angleInTurn);

void stop();

}  // namespace mtr

#endif