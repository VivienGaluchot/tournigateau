// ----------------------------------
// Pins
// ----------------------------------

// general io

const int btnPin = 23;
const int ledPin = 22;
const int potPin = 36;

// stepper

// Driver   Gpio
// -------------
// Dir      2
// Step     0
// Clk      4
// Pdn      16
// Nc       17
// Ms2      5
// Ms1      18
// En       19

const int mtrDirPin = 2;
const int mtrStepPin = 0;
const int mtrClkPin = 4;
const int mtrPdnPin = 16;
const int mtrNcPin = 17;
const int mtrMs2Pin = 5;
const int mtrMs1Pin = 18;
const int mtrEnPin = 19;

// Turn ratio
// Signal -> Driver -> Motor -> Gear

const float gearRatio = 90.0 / 20.0;
const float mtrRatio = 360 / 1.8;
const float driverRatio = 16;
const int stepPerTurn = driverRatio * mtrRatio * gearRatio;


// ----------------------------------
// Services
// ----------------------------------

// utils

float clamp(float x, float a, float b) {
    float min = a;
    float max = b;
    if (a > b) {
        min = b;
        max = a;
    }
    if (x < min)
        return min;
    if (x > max)
        return max;
    return x;
}

float lerp(float x, float from, float to) {
    return from + x * (to - from);
}

float unlerp(float l, float from, float to) {
    return (l - from) / (to - from);
}

float floatSquish(float x, float xFrom, float xTo, float outFrom, float outTo) {
    if (outFrom == outTo)
        return outTo;
    float l;
    if (xFrom != xTo) {
        l = unlerp(x, xFrom, xTo);
    } else {
        l = 0.5;
    }
    l = lerp(l, outFrom, outTo);
    return clamp(l, outFrom, outTo);
}

// stepper

const int mtrPwmResolution = 10;
const int mtrPwmChannel = 0;
float mtrCurrentRpm = 0;
bool mtrIsEnabled = false;

void mtrInit() {
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

void mtrEnable(bool isEnabled) {
    digitalWrite(mtrEnPin, isEnabled ? LOW : HIGH);
    mtrIsEnabled = isEnabled;
}

void mtrSetStepRpm(float rpm) {
    mtrEnable(rpm != 0 && !isnan(rpm));
    float freq = mtrRpmToStepPerSec(rpm);
    if (freq >= 0) {
        ledcWriteTone(mtrPwmChannel, freq);
        digitalWrite(mtrDirPin, HIGH);
    } else {
        ledcWriteTone(mtrPwmChannel, -1 * freq);
        digitalWrite(mtrDirPin, LOW);
    }
    mtrCurrentRpm = rpm;
}

uint32_t mtrRampStartTimeInMs = 0;
uint32_t mtrRampEndTimeInMs = 0;
float mtrRampStartRpm = 0;
float mtrRampEndRmp = 0;
bool mtrIsRampEnabled = false;
uint32_t mtrRampLastCycleInMs = 0;

float mtrRpmToStepPerSec(float rmp) {
    return rmp * (stepPerTurn / 60);
}

void mtrRampSetup(uint32_t timeInMs, float fromRmp, float toRmp, float rmpPerSec) {
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

const uint32_t mtrRampMinPeriodInMs = 50;

void mtrRampCycle(uint32_t timeInMs) {
    if (mtrIsRampEnabled && ((timeInMs - mtrRampLastCycleInMs) >= mtrRampMinPeriodInMs)) {
        float rpm = floatSquish(timeInMs, mtrRampStartTimeInMs, mtrRampEndTimeInMs, mtrRampStartRpm, mtrRampEndRmp);
        if (timeInMs >= mtrRampEndTimeInMs) {
            mtrIsRampEnabled = false;
        }
        mtrSetStepRpm(rpm);
        mtrRampLastCycleInMs = timeInMs;
    }
}

// button

const uint32_t bntDebounceDelay = 50;
uint32_t bntLastDebounceTime = 0;
bool bntLastReading = false;
bool btnIsPressed = false;
bool btnHasSwitched = false;

void btnInit() {
    pinMode(btnPin, INPUT_PULLUP);
}

void btnRead(uint32_t timeInMs) {
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

// potentiometer

const uint32_t potDebounceDelay = 50;
const uint16_t potEps = 10;
float potAvg = 0;
uint16_t potLastDebounceTime = 0;
uint16_t potValue = 0;
uint16_t potHasChanged = 0;
float potRate = 0;

void potInit() {
    pinMode(potPin, ANALOG);
}

void potRead(uint32_t timeInMs) {
    potHasChanged = false;
    uint16_t raw = analogRead(potPin);
    potAvg = potAvg * 0.9 + raw * 0.1;
    bool hasDiff = abs(potAvg - potValue) >= potEps;
    if (hasDiff) {
        if ((timeInMs - potLastDebounceTime) > potDebounceDelay) {
            potValue = round(potAvg);
            potHasChanged = true;
            potRate = clamp(potValue / 4096.0, 0, 1);
        } else {
            // wait to check if the diff is maintained
        }
    } else {
        potLastDebounceTime = timeInMs;
    }
}


// ----------------------------------
// Main
// ----------------------------------

void setup() {
    mtrInit();
    btnInit();
    potInit();
    pinMode(ledPin, OUTPUT);

    Serial.begin(115200);
    while (!Serial) {
        // wait for serial port to connect. Needed for native USB
    }

    Serial.println("---");
    Serial.println("started");
}


const uint32_t logPeriodInMs = 1000;
uint32_t lastLogInMs = 0;
uint32_t cycleCount = 0;

void loop() {
    uint32_t timeInMs = millis();
    btnRead(timeInMs);
    potRead(timeInMs);

    if (btnHasSwitched) {
        if (btnIsPressed) {
            Serial.println("btn on");
        } else {
            Serial.println("btn off");
        }
    }

    if (potHasChanged) {
        float pos = floatSquish(potRate, 0.40, .95, 0, 20);
        float neg = floatSquish(potRate, 0.30, .05, 0, -20);
        float cmd = pos + neg;
        mtrRampSetup(timeInMs, mtrCurrentRpm, cmd, 20);
        digitalWrite(ledPin, cmd != 0);
    }

    mtrRampCycle(timeInMs);

    if ((timeInMs - lastLogInMs) > logPeriodInMs) {
        Serial.print("[");
        Serial.print(timeInMs);
        Serial.print("]");
        Serial.print(" cycles ");
        Serial.print(cycleCount);
        Serial.print(", knob ");
        Serial.print(potRate);
        Serial.print(", rpm ");
        Serial.print(mtrCurrentRpm);
        Serial.print(", freq ");
        Serial.println(ledcReadFreq(mtrPwmChannel));
        lastLogInMs += logPeriodInMs;

        cycleCount = 0;
    }

    cycleCount++;
    delay(1);
}
