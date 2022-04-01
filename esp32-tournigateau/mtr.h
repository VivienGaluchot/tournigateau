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

// true when the dirver is enabled
extern bool isDriverEnabled;

// ----------------------------------
// services
// ----------------------------------

void initialize();

void doCycle(uint32_t timeInMs);

void rampSetup(uint32_t timeInMs, float fromRmp, float toRmp, float rmpPerSec);

}  // namespace mtr

#endif