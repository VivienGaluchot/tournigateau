#ifndef AUTOM_H
#define AUTOM_H

// ----------------------------------
// const - types
// ----------------------------------

typedef enum {
    // wait for knob to be reseted
    INIT = 0,
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
// services
// ----------------------------------

void doState();

#endif