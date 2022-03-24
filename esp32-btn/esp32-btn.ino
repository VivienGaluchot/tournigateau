// ----------------------------------
// Pins
// ----------------------------------

// general io

const int btnPin = 23;
const int ledPin = 32;

// stepper

const int mtrStepPin = 19;
const int mtrDirPin = 18;
const int mtrEnPin = 5;
const int mtrMs1Pin = 17;
const int mtrMs2Pin = 16;

// Turn ratio
// Signal -> Driver -> Motor -> Gear

const float gearRatio = 90.0 / 20.0;
const float mtrRatio = 360 / 1.8;
const float driverRatio = 8;
const int stepPerTurn = driverRatio * mtrRatio * gearRatio;


// ----------------------------------
// Services
// ----------------------------------

// stepper

const int mtrPwmResolution = 10;
const int mtrPwmChannel = 0;
float mtrCurrentRpm = 0;

void mtrInit() {
    pinMode(mtrDirPin, OUTPUT);
    pinMode(mtrStepPin, OUTPUT);
    pinMode(mtrEnPin, OUTPUT);
    pinMode(mtrMs1Pin, OUTPUT);
    pinMode(mtrMs2Pin, OUTPUT);

    // disable
    digitalWrite(mtrEnPin, HIGH);

    // 1/8 microstepping
    // driverRatio shall be set accordingly
    digitalWrite(mtrMs1Pin, LOW);
    digitalWrite(mtrMs2Pin, LOW);
    
    // configure PWM
    ledcSetup(mtrPwmChannel, 1, mtrPwmResolution);
    ledcWrite(mtrPwmChannel, 0);

    ledcAttachPin(mtrStepPin, mtrPwmChannel);
}

void mtrEnable(bool isEnabled) {
    digitalWrite(mtrEnPin, isEnabled ? LOW : HIGH);
}

void mtrSetStepRpm(float rpm) {
    mtrEnable(rpm != 0);
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
uint32_t mtrRampDeltaTimeInMs = 0;
float mtrRampStartRpm = 0;
float mtrRampDeltaRmp = 0;
bool mtrIsRampEnabled = false;
uint32_t mtrRampLastCycleInMs = 0;

float mtrRpmToStepPerSec(float rmp) {
    return rmp * (stepPerTurn / 60);
}

void mtrRampSetup(uint32_t timeInMs, float fromRmp, float toRmp, float rmpPerSec) {
    mtrRampStartTimeInMs = timeInMs;

    mtrRampStartRpm = fromRmp;
    mtrRampDeltaRmp = toRmp - fromRmp;
    mtrRampDeltaTimeInMs = (1000 * abs(mtrRampDeltaRmp)) / rmpPerSec;

    mtrIsRampEnabled = true;
    mtrRampLastCycleInMs = timeInMs;

    mtrSetStepRpm(fromRmp);

    Serial.print("ramp: ");
    Serial.print(fromRmp);
    Serial.print(" rmp (");
    Serial.print(mtrRpmToStepPerSec(fromRmp));
    Serial.print(" Hz) -> ");
    Serial.print(toRmp);
    Serial.print(" rmp (");
    Serial.print(mtrRpmToStepPerSec(toRmp));
    Serial.print(" Hz) in ");
    Serial.print(mtrRampDeltaTimeInMs);
    Serial.println(" ms");
}

const uint32_t mtrRampMinPeriodInMs = 100;

void mtrRampCycle(uint32_t timeInMs) {
    if (mtrIsRampEnabled && ((timeInMs - mtrRampLastCycleInMs) >= mtrRampMinPeriodInMs)) {
        float rate = (float)(timeInMs - mtrRampStartTimeInMs) / mtrRampDeltaTimeInMs;
        float rmp = 0;
        if (rate > 0 && rate < 1) {
            rmp = mtrRampStartRpm + (mtrRampDeltaRmp * rate);
        } else {
            rmp = mtrRampStartRpm + mtrRampDeltaRmp;
            mtrIsRampEnabled = false;
            Serial.println("ramp: done");
        }
        mtrSetStepRpm(rmp);

        mtrRampLastCycleInMs = timeInMs;
    }
}

// button

const unsigned long bntDebounceDelay = 50;
unsigned long bntLastDebounceTime = 0;
bool bntLastReading = false;
bool btnIsPressed = false;
bool btnHasSwitched = false;

void btnRead() {
    bool reading = digitalRead(btnPin) == LOW;
    if (reading != bntLastReading) {
        bntLastDebounceTime = millis();
    }
    bntLastReading = reading;
    btnHasSwitched = false;
    if ((millis() - bntLastDebounceTime) > bntDebounceDelay) {
        if (reading != btnIsPressed) {
            btnHasSwitched = true;
        }
        btnIsPressed = reading;
    }
}


// ----------------------------------
// Main
// ----------------------------------

void setup() {
    pinMode(ledPin, OUTPUT);
    pinMode(btnPin, INPUT_PULLUP);
    mtrInit();

    Serial.begin(115200);
    while (!Serial) {
        // wait for serial port to connect. Needed for native USB
    }

    Serial.println("---");
    Serial.println("started");
}


const uint32_t logPeriodInMs = 1000;
uint32_t lastLogInMs = 0;
bool isRunning = false;

void loop() {
    uint32_t timeInMs = millis();
    btnRead();

    if (btnHasSwitched) {
        if (!btnIsPressed) {
            if (isRunning) {
                mtrRampSetup(timeInMs, mtrCurrentRpm, 0, 20);
                isRunning = false;
            } else {
                mtrRampSetup(timeInMs, mtrCurrentRpm, 20, 20);
                isRunning = true;
            }
            Serial.print("isRunning ");
            Serial.println(isRunning);
        }
    }

    mtrRampCycle(timeInMs);

    if ((timeInMs - lastLogInMs) > logPeriodInMs) {
        Serial.print("[");
        Serial.print(timeInMs);
        Serial.print("]");
        Serial.print(" rpm ");
        Serial.print(mtrCurrentRpm);
        Serial.print(" freq ");
        Serial.println(ledcReadFreq(mtrPwmChannel));
        lastLogInMs += logPeriodInMs;
    }

    digitalWrite(ledPin, isRunning);

    delay(5);
}
