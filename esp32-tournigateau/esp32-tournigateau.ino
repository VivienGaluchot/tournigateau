#include "autom.h"
#include "io.h"
#include "mtr.h"
#include "pins.h"
#include "toolbox.h"

// ----------------------------------
// const - types
// ----------------------------------

const uint32_t logPeriodInMs = 1000;

const char version[] = "0.0.0";

// ----------------------------------
// variables
// ----------------------------------

uint32_t lastLogInMs = 0;
uint32_t cycleCount = 0;

// ----------------------------------
// Main
// ----------------------------------

void setup() {
    mtr::initialize();
    io::initialize();

    Serial.begin(115200);
    while (!Serial) {
        // wait for serial port to connect. Needed for native USB
    }

    Serial.println("tournigateau ---");
    Serial.print("version ");
    Serial.println(version);
}

void loop() {
    uint32_t timeInMs = millis();
    io::doCycle(timeInMs);
    autom::doCycle(timeInMs);
    mtr::doCycle(timeInMs);

    if ((timeInMs - lastLogInMs) > logPeriodInMs) {
        Serial.print("[");
        Serial.print(timeInMs);
        Serial.print("]");
        Serial.print(" cycles ");
        Serial.print(cycleCount);
        Serial.print(", knob ");
        Serial.print(io::potRate);
        Serial.print(", rpm ");
        Serial.print(mtr::currentRpm);
        Serial.print(", freq ");
        Serial.println(ledcReadFreq(mtrPwmChannel));
        lastLogInMs += logPeriodInMs;

        cycleCount = 0;
    }

    cycleCount++;
    delay(1);
}
