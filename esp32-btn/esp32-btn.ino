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


// ----------------------------------
// Services
// ----------------------------------

// stepper

const int mtrPwmResolution = 10;
const int mtrPwmChannel = 0;

void mtrInit() {
    pinMode(mtrDirPin, OUTPUT);
    pinMode(mtrStepPin, OUTPUT);
    pinMode(mtrEnPin, OUTPUT);
    pinMode(mtrMs1Pin, OUTPUT);
    pinMode(mtrMs2Pin, OUTPUT);

    // disable
    digitalWrite(mtrEnPin, HIGH);

    // 1/8 microstepping
    digitalWrite(mtrMs1Pin, LOW);
    digitalWrite(mtrMs2Pin, LOW);
    
    // configure PWM
    ledcSetup(mtrPwmChannel, 1, mtrPwmResolution);
    ledcWrite(mtrPwmChannel, 0);

    ledcAttachPin(mtrStepPin, mtrPwmChannel);
}

void mtrSet(bool isEnabled, float rpm) {
    digitalWrite(mtrEnPin, isEnabled ? LOW : HIGH);
    if (rpm != 0) {
        if (rpm < 0) {
            digitalWrite(mtrDirPin, HIGH);
            rpm = -1 * rpm;
        } else {
            digitalWrite(mtrDirPin, LOW);
        }
        
        // TODO convert rmp to freq
        float mtrPwmfreq = rpm;
        ledcSetup(mtrPwmChannel, mtrPwmfreq, mtrPwmResolution);
        ledcWrite(mtrPwmChannel, 0x1FF);
    } else {
        ledcWrite(mtrPwmChannel, 0);
    }
}

long mtrRampStartTimeInMs = 0;
long mtrRampDeltaTimeInMs = 0;
float mtrRampStartFreq = 0;
float mtrRampDeltaFreq = 0;
bool mtrIsRampDone = false;
long mtrRampLastCycleInMs = 0;

void mtrRampInit(long timeInMs, float fromRmp, float toRmp, float rmpPerSec) {
    // TODO convert rmp to freq
    float fromFreq = fromRmp;
    float toFreq = toRmp;

    mtrRampStartTimeInMs = timeInMs;
    mtrRampStartFreq = fromFreq;

    mtrRampDeltaFreq = toFreq - fromFreq;
    mtrRampDeltaTimeInMs = (1000 * abs(mtrRampDeltaFreq)) / rmpPerSec;

    mtrIsRampDone = false;
    mtrRampLastCycleInMs = 0;
    mtrSet(true, fromRmp);

    Serial.print("ramp: ");
    Serial.print(mtrRampDeltaFreq);
    Serial.print(" Hz over ");
    Serial.print(mtrRampDeltaTimeInMs);
    Serial.println(" ms");
}

void mtrRampCycle(long timeInMs) {
    if (mtrIsRampDone) {
        return;
    }
    if ((timeInMs - mtrRampLastCycleInMs) < 100) {
        return;
    }

    float rate = (float)(timeInMs - mtrRampStartTimeInMs) / mtrRampDeltaTimeInMs;
    float freq = 0;
    if (rate > 0 && rate < 1) {
        freq = mtrRampStartFreq + (mtrRampDeltaFreq * rate);
    } else if (rate >= 1) {
        freq = mtrRampStartFreq + mtrRampDeltaFreq;
        mtrIsRampDone = true;
    }
    ledcWriteTone(mtrPwmChannel, freq);

    mtrRampLastCycleInMs = timeInMs;
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

const long logPeriodInMs = 1000;
long lastLogInMs = 0;
bool isRunning = false;

void loop() {
    long timeInMs = millis();
    btnRead();

    if (btnHasSwitched) {
        if (!btnIsPressed) {
            if (isRunning) {
                mtrSet(false, 0);
                isRunning = false;
            } else {
                mtrRampInit(timeInMs, 10, 200, 50);
                isRunning = true;
            }
            Serial.print("isRunning ");
            Serial.println(isRunning);
        }
    }

    if (isRunning) {
        mtrRampCycle(timeInMs);
    }

    if ((timeInMs - lastLogInMs) > logPeriodInMs) {
        Serial.print("[");
        Serial.print(timeInMs);
        Serial.print("]");
        Serial.print(" freq ");
        Serial.print(ledcReadFreq(mtrPwmChannel));
        Serial.print(" ramp ");
        Serial.println(!mtrIsRampDone);
        lastLogInMs += logPeriodInMs;
    }

    digitalWrite(ledPin, isRunning);

    delay(5);
}
