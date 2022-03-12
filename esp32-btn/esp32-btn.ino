const int buttonPin = 23;
const int ledPin =  2;

const unsigned long bntDebounceDelay = 50;
unsigned long bntLastDebounceTime = 0;
bool bntLastReading = 0;
bool isButtonOn = 0;
bool hasButtonSwitched = 0;

void readButton() {
  bool reading = digitalRead(buttonPin) == LOW;
  if (reading != bntLastReading) {
    bntLastDebounceTime = millis();
  }
  bntLastReading = reading;
  hasButtonSwitched = false;
  if ((millis() - bntLastDebounceTime) > bntDebounceDelay) {
    if (reading != isButtonOn) {
      hasButtonSwitched = true;
    }
    isButtonOn = reading;
  }
}


void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial) {// wait for serial port to connect. Needed for native USB
  }

  Serial.println("started");
}

void loop() {
  readButton();

  if (hasButtonSwitched) {
    Serial.print("button ");
    Serial.println(isButtonOn);
  }

  if (isButtonOn) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}
