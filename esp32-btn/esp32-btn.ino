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

const double mtrPwmfreq = 10000;
const int mtrPwmResolution = 2;
const int mtrPwmChannel = 0;

void mtrInit() {
    pinMode(mtrDirPin, OUTPUT);
    pinMode(mtrStepPin, OUTPUT);
    pinMode(mtrEnPin, OUTPUT);
    pinMode(mtrMs1Pin, OUTPUT);
    pinMode(mtrMs2Pin, OUTPUT);

    // disable
    digitalWrite(mtrEnPin, HIGH);

    // 1/16 microstepping
    digitalWrite(mtrMs1Pin, HIGH);
    digitalWrite(mtrMs2Pin, HIGH);
    
    // configure PWM
    ledcSetup(mtrPwmChannel, mtrPwmfreq, mtrPwmResolution);
    ledcWrite(mtrPwmChannel, 0);

    ledcAttachPin(ledPin, mtrPwmChannel);
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

        ledcWrite(mtrPwmChannel, 1);
    } else {
        ledcWrite(mtrPwmChannel, 0);
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

void loop() {
    btnRead();

    if (btnHasSwitched) {
        Serial.print("button ");
        Serial.println(btnIsPressed);

        if (btnIsPressed) {
            mtrSet(true, 1);
        } else {
            mtrSet(false, 0);
        }

        Serial.print("freq ");
        Serial.println(ledcReadFreq(mtrPwmChannel));
    }

    // digitalWrite(ledPin, btnIsPressed);
}
