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
const uint32_t mtrRampMinPeriodInMs = 50;

// ----------------------------------
// local variable
// ----------------------------------

static uint32_t mtrRampStartTimeInMs = 0;
static uint32_t mtrRampEndTimeInMs = 0;
static float mtrRampStartRpm = 0;
static float mtrRampEndRmp = 0;
static bool mtrIsRampEnabled = false;
static uint32_t mtrRampLastCycleInMs = 0;

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

static void doRampCycle(uint32_t timeInMs) {
    if (mtrIsRampEnabled && ((timeInMs - mtrRampLastCycleInMs) >= mtrRampMinPeriodInMs)) {
        float rpm = toolbox::squish(timeInMs, mtrRampStartTimeInMs, mtrRampEndTimeInMs, mtrRampStartRpm, mtrRampEndRmp);
        if (timeInMs >= mtrRampEndTimeInMs) {
            mtrIsRampEnabled = false;
        }
        setFixedRpm(rpm);
        mtrRampLastCycleInMs = timeInMs;
    }
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

void doCycle(uint32_t timeInMs) {
    doRampCycle(timeInMs);
}

void rampSetup(uint32_t timeInMs, float fromRmp, float toRmp, float rmpPerSec) {
    mtrRampStartRpm = fromRmp;
    mtrRampEndRmp = toRmp;

    mtrRampStartTimeInMs = timeInMs;
    mtrRampEndTimeInMs = timeInMs + (1000.0 * abs(toRmp - fromRmp)) / rmpPerSec;

    mtrIsRampEnabled = true;

    // Serial.print("ramp: ");
    // Serial.print(fromRmp);
    // Serial.print(" rmp (");
    // Serial.print(mtrRpmToStepPerSec(fromRmp));
    // Serial.print(" Hz) -> ");
    // Serial.print(toRmp);
    // Serial.print(" rmp (");
    // Serial.print(mtrRpmToStepPerSec(toRmp));
    // Serial.print(" Hz) in ");
    // Serial.print(mtrRampEndTimeInMs - mtrRampStartTimeInMs);
    // Serial.println(" ms");
}

}  // namespace mtr