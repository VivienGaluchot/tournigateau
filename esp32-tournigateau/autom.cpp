#include "autom.h"

#include <Arduino.h>


// ----------------------------------
// const - types
// ----------------------------------

static const char* state_names[STATE_COUNT] = {
    "INIT",
    "IDLE",
    "CONTINUOUS",
    "AUTO_360"
};

// ----------------------------------
// local variable
// ----------------------------------

static state_t state = INIT;


// ----------------------------------
// local services
// ----------------------------------

static state_t doInit() {
    return IDLE;
}

static state_t doIdle() {
    return IDLE;
}

static state_t doContinuous() {
    return CONTINUOUS;
}

static state_t doAuto360() {
    return AUTO_360;
}


// ----------------------------------
// public services
// ----------------------------------

void doState() {
    state_t next;
    if (state == INIT) {
        next = doInit();
    } else if (state == IDLE) {
        next = doIdle();
    }  else if (state == CONTINUOUS) {
        next = doContinuous();
    }  else if (state == AUTO_360) {
        next = doAuto360();
    }
    if (next != state) {
        Serial.print(">>> ");
        Serial.println(state_names[next]);
        state =  next;
    }
}