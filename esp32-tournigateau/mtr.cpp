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
const uint32_t rotateStepPeriodInMs = 2;

typedef enum DriveMode {
    OFF,
    IMPULSE,
    PWM
};

// ----------------------------------
// local variable
// ----------------------------------

static DriveMode currentMode = OFF;

static float rampTargetRmp = 0;
static float rampRmpPerSec = 1;
static bool isRampEnabled = false;
static uint32_t rampLastTimeInMs = 0;

static int32_t rotateTargetStepCount = 0;
static uint32_t rotateLastTimeInMs = 0;

// ----------------------------------
// public variable
// ----------------------------------

float currentRpm = 0;
int32_t rotateCurrentStep;
bool isDriverEnabled = false;
bool isRotating = false;

// ----------------------------------
// local services
// ----------------------------------

static float rpmToStepPerSec(float rmp) {
    return rmp * (stepPerTurn / 60);
}

static int32_t angleToStepCount(float angleInTurn) {
    return lround(angleInTurn * stepPerTurn);
}

static void setDriveMode(DriveMode mode) {
    isDriverEnabled = mode != OFF;
    digitalWrite(mtrEnPin, isDriverEnabled ? LOW : HIGH);
    if (currentMode != mode) {
        if (mode == PWM) {
            ledcAttachPin(mtrStepPin, mtrPwmChannel);
        } else if (currentMode == PWM) {
            ledcDetachPin(mtrStepPin);
            digitalWrite(mtrStepPin, LOW);
        }
        currentMode = mode;
    }
}

static void setFixedRpm(float rpm) {
    setDriveMode((rpm != 0 && !isnan(rpm)) ? PWM : OFF);
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

static void sendImpulse(bool isClockwise) {
    if (currentMode == IMPULSE) {
        digitalWrite(mtrDirPin, isClockwise);
        digitalWrite(mtrStepPin, HIGH);
        digitalWrite(mtrStepPin, LOW);
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
    digitalWrite(mtrStepPin, LOW);
    digitalWrite(mtrClkPin, LOW);
    digitalWrite(mtrPdnPin, LOW);
    digitalWrite(mtrNcPin, LOW);

    // configure PWM pin
    ledcSetup(mtrPwmChannel, 1, mtrPwmResolution);
    ledcWrite(mtrPwmChannel, 0);
}

void doCycle(uint32_t timeInMs) {
    if (isRampEnabled) {
        // update ramp
        uint32_t deltaTime = timeInMs - rampLastTimeInMs;
        if (deltaTime >= rampMinPeriodInMs) {
            float maxDeltaRmp = (deltaTime * rampRmpPerSec) / 1000.0;
            float rpm = toolbox::clamp(rampTargetRmp, currentRpm - maxDeltaRmp, currentRpm + maxDeltaRmp);
            setFixedRpm(rpm);
            if (rpm == rampTargetRmp) {
                isRampEnabled = false;
            }
            rampLastTimeInMs = timeInMs;
        }
    } else if (isRotating) {
        // update rotate
        uint32_t deltaTime = timeInMs - rotateLastTimeInMs;
        if (deltaTime >= rotateStepPeriodInMs) {
            int32_t deltaStep = rotateTargetStepCount - rotateCurrentStep;
            if (deltaStep > 0) {
                sendImpulse(true);
                rotateCurrentStep++;
            } else if (deltaStep < 0) {
                sendImpulse(false);
                rotateCurrentStep--;
            } else if (deltaStep == 0) {
                isRotating = false;
                // setDriveMode(OFF);
                Serial.println("rotate done");
            }
            rotateLastTimeInMs = timeInMs;
        }
    }
}

void rampSetup(uint32_t timeInMs, float toRmp, float rmpPerSec) {
    rampTargetRmp = toRmp;
    rampRmpPerSec = rmpPerSec;
    isRampEnabled = true;
}

void resetAbsReference() {
    rotateCurrentStep = 0;
}

void startRotateToAbs(float angleInTurn) {
    rotateTargetStepCount = angleToStepCount(angleInTurn);
    isRotating = true;
    setDriveMode(IMPULSE);
    Serial.print("rotate from ");
    Serial.print(rotateCurrentStep);
    Serial.print(" to ");
    Serial.println(rotateTargetStepCount);
}

void stop() {
    isRotating = false;
    isRampEnabled = false;
    setFixedRpm(0);
    setDriveMode(OFF);
}

}  // namespace mtr