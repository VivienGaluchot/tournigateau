#include "autom.h"

#include <Arduino.h>

#include "io.h"
#include "mtr.h"

namespace autom {

// ----------------------------------
// const - types
// ----------------------------------

static const char* state_names[STATE_COUNT] = {
    "NONE",
    "INIT",
    "IDLE",
    "CONTINUOUS",
    "AUTO_360"};

// ----------------------------------
// local variable
// ----------------------------------

static state_t prevState = NONE;
static state_t state = INIT;

static uint32_t auto360EntryTimeInMs = 0;

// ----------------------------------
// local services
// ----------------------------------

// init

static state_t enterInit(uint32_t timeInMs) {
    io::setLedBlink(300);
}

static state_t doInit(uint32_t timeInMs) {
    if (timeInMs > 100 && io::potRateCentered == 0) {
        return IDLE;
    }
    return INIT;
}

// idle

static state_t enterIdle(uint32_t timeInMs) {
    io::setLed(false);
}

static state_t doIdle(uint32_t timeInMs) {
    if (io::potRateCentered != 0) {
        return CONTINUOUS;
    }
    if (io::btnHasSwitched && io::btnIsPressed) {
        return AUTO_360;
    }
    return IDLE;
}

// continuous

static state_t doContinuous(uint32_t timeInMs) {
    if (io::potRateCentered == 0 && !mtr::isDriverEnabled) {
        return IDLE;
    }
    if (io::potHasChanged) {
        float rpm = io::potRateCentered * 20;
        mtr::rampSetup(timeInMs, rpm, 20);
        io::setLed(io::potRateCentered != 0);
    }
    return CONTINUOUS;
}

// auto 360

static state_t enterAuto360(uint32_t timeInMs) {
    io::setLedBlink(100);
    auto360EntryTimeInMs = timeInMs;
}

static state_t doAuto360(uint32_t timeInMs) {
    if (timeInMs - 3000 > auto360EntryTimeInMs) {
        return INIT;
    }
    // TODO
    return AUTO_360;
}

// ----------------------------------
// public services
// ----------------------------------

void doCycle(uint32_t timeInMs) {
    if (prevState != state) {
        Serial.print(">>> ENTER ");
        Serial.println(state_names[state]);
        if (state == INIT) {
            enterInit(timeInMs);
        } else if (state == IDLE) {
            enterIdle(timeInMs);
        } else if (state == AUTO_360) {
            enterAuto360(timeInMs);
        }
        prevState = state;
    }

    state_t next;
    if (state == INIT) {
        next = doInit(timeInMs);
    } else if (state == IDLE) {
        next = doIdle(timeInMs);
    } else if (state == CONTINUOUS) {
        next = doContinuous(timeInMs);
    } else if (state == AUTO_360) {
        next = doAuto360(timeInMs);
    }
    if (next != state) {
        Serial.print("<<< EXIT  ");
        Serial.println(state_names[state]);
        state = next;
    }
}

}  // namespace autom