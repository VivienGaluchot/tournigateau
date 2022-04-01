#ifndef AUTOM_H
#define AUTOM_H

#include <stdint.h>

namespace autom {

// ----------------------------------
// const - types
// ----------------------------------

typedef enum {
    NONE = 0,
    // wait for knob to be reseted
    INIT,
    // no rotation
    IDLE,
    // motor speed is driven by the knob
    CONTINUOUS,
    // motor speed is fixed for 360 photos
    AUTO_360,
    // number of states
    STATE_COUNT
} state_t;

// ----------------------------------
// variables
// ----------------------------------

// ----------------------------------
// services
// ----------------------------------

void doCycle(uint32_t timeInMs);

}  // namespace autom

#endif