#include "io.h"

#include <Arduino.h>

#include "pins.h"
#include "toolbox.h"

namespace io {

// ----------------------------------
// const - types
// ----------------------------------

static const uint32_t bntDebounceDelay = 50;
static const uint32_t potDebounceDelay = 50;

// ----------------------------------
// local variable
// ----------------------------------

// button

static uint32_t bntLastDebounceTime = 0;
static bool bntLastReading = false;

// potentiometer

static const uint16_t potEps = 10;
static float potAvg = 0;
static uint16_t potLastDebounceTime = 0;

// led

static bool isBlinkEnabled = false;
static uint32_t blinkPeriodInMs = 0;
static uint32_t lastBlinkTimeInMs = 0;

// ----------------------------------
// public variable
// ----------------------------------

bool btnIsPressed = false;
bool btnHasSwitched = false;
uint16_t potValue = 0;
float potRate = 0;
float potRateCentered = 0;
bool potHasChanged = 0;

// ----------------------------------
// local services
// ----------------------------------

static void btnRead(uint32_t timeInMs) {
    bool reading = digitalRead(btnPin) == LOW;
    if (reading != bntLastReading) {
        bntLastDebounceTime = timeInMs;
    }
    bntLastReading = reading;
    btnHasSwitched = false;
    if ((timeInMs - bntLastDebounceTime) > bntDebounceDelay) {
        if (reading != btnIsPressed) {
            btnHasSwitched = true;
        }
        btnIsPressed = reading;
    }
}

static void potRead(uint32_t timeInMs) {
    potHasChanged = false;
    uint16_t raw = analogRead(potPin);
    potAvg = potAvg * 0.9 + raw * 0.1;
    bool hasDiff = abs(potAvg - potValue) >= potEps;
    if (hasDiff) {
        if ((timeInMs - potLastDebounceTime) > potDebounceDelay) {
            potValue = round(potAvg);
            potHasChanged = true;
            potRate = toolbox::clamp(potValue / 4096.0, 0, 1);

            float pos = toolbox::squish(potRate, 0.40, .95, 0, 1);
            float neg = toolbox::squish(potRate, 0.30, .05, 0, -1);
            potRateCentered = pos + neg;
        } else {
            // wait to check if the diff is maintained
        }
    } else {
        potLastDebounceTime = timeInMs;
    }
}

// ----------------------------------
// public services
// ----------------------------------

void initialize() {
    pinMode(btnPin, INPUT_PULLUP);
    pinMode(potPin, ANALOG);
    pinMode(ledPin, OUTPUT);
}

void doCycle(uint32_t timeInMs) {
    btnRead(timeInMs);
    potRead(timeInMs);

    if (btnHasSwitched) {
        if (btnIsPressed) {
            Serial.println("btn on");
        } else {
            Serial.println("btn off");
        }
    }

    if (isBlinkEnabled) {
        if ((timeInMs - lastBlinkTimeInMs) > blinkPeriodInMs) {
            digitalWrite(ledPin, !digitalRead(ledPin));
            lastBlinkTimeInMs = timeInMs;
        }
    }
}

void setLed(bool isOn) {
    digitalWrite(ledPin, isOn);
    isBlinkEnabled = false;
}

void setLedBlink(uint32_t blinkPeriodInMs) {
    isBlinkEnabled = true;
    io::blinkPeriodInMs = blinkPeriodInMs;
}

}  // namespace io