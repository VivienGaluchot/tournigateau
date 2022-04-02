#include "mtr.h"

#include <Arduino.h>

#include "pins.h"
#include "toolbox.h"

namespace mtr {

// ----------------------------------
// const - types
// ----------------------------------

// Turn ratio
// Signal -> Driver -> Motor -> Gear
const float gearRatio = 90.0 / 20.0;
const float mtrRatio = 360 / 1.8;
const float driverRatio = 16;
const int stepPerTurn = driverRatio * mtrRatio * gearRatio;

const int mtrPwmResolution = 10;
const uint32_t rampMinPeriodInMs = 50;

// ----------------------------------
// local variable
// ----------------------------------

static float rampTargetRmp = 0;
static float rampRmpPerSec = 1;
static bool isRampEnabled = false;
static uint32_t rampLastCycleInMs = 0;

// ----------------------------------
// public variable
// ----------------------------------

float currentRpm = 0;
bool isDriverEnabled = false;

// ----------------------------------
// local services
// ----------------------------------

static float rpmToStepPerSec(float rmp) {
    return rmp * (stepPerTurn / 60);
}

static void mtrEnable(bool isEnabled) {
    digitalWrite(mtrEnPin, isEnabled ? LOW : HIGH);
    isDriverEnabled = isEnabled;
}

static void setFixedRpm(float rpm) {
    mtrEnable(rpm != 0 && !isnan(rpm));
    float freq = rpmToStepPerSec(rpm);
    if (freq >= 0) {
        ledcWriteTone(mtrPwmChannel, freq);
        digitalWrite(mtrDirPin, HIGH);
    } else {
        ledcWriteTone(mtrPwmChannel, -1 * freq);
        digitalWrite(mtrDirPin, LOW);
    }
    currentRpm = rpm;
}

// ----------------------------------
// public services
// ----------------------------------

void initialize() {
    pinMode(mtrDirPin, OUTPUT);
    pinMode(mtrStepPin, OUTPUT);
    pinMode(mtrClkPin, OUTPUT);
    pinMode(mtrPdnPin, OUTPUT);
    pinMode(mtrNcPin, OUTPUT);
    pinMode(mtrMs2Pin, OUTPUT);
    pinMode(mtrMs1Pin, OUTPUT);
    pinMode(mtrEnPin, OUTPUT);

    // disable driver
    digitalWrite(mtrEnPin, HIGH);
    digitalWrite(mtrDirPin, LOW);

    // 1/16 microstepping
    // driverRatio shall be set accordingly
    digitalWrite(mtrMs1Pin, HIGH);
    digitalWrite(mtrMs2Pin, HIGH);

    // default pins
    digitalWrite(mtrClkPin, LOW);
    digitalWrite(mtrPdnPin, LOW);
    digitalWrite(mtrNcPin, LOW);

    // configure PWM pin
    ledcSetup(mtrPwmChannel, 1, mtrPwmResolution);
    ledcWrite(mtrPwmChannel, 0);
    ledcAttachPin(mtrStepPin, mtrPwmChannel);
}

float toRmpLog = 0;

void doCycle(uint32_t timeInMs) {
    // update Ramp
    if (isRampEnabled) {
        uint32_t deltaTime = timeInMs - rampLastCycleInMs;
        if (deltaTime >= rampMinPeriodInMs) {
            float maxDeltaRmp = (deltaTime * rampRmpPerSec) / 1000.0;
            float rpm = toolbox::clamp(rampTargetRmp, currentRpm - maxDeltaRmp, currentRpm + maxDeltaRmp);
            setFixedRpm(rpm);
            rampLastCycleInMs = timeInMs;
            if (rpm == rampTargetRmp) {
                isRampEnabled = false;
            }
        }
    }
}

void rampSetup(uint32_t timeInMs, float toRmp, float rmpPerSec) {
    rampTargetRmp = toRmp;
    rampRmpPerSec = rmpPerSec;
    isRampEnabled = true;
}

}  // namespace mtr