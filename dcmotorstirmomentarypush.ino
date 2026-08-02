const int buttonPin = A3;
const int motorPin = 3;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);

  digitalWrite(motorPin, LOW);  // Motor OFF initially
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    digitalWrite(motorPin, HIGH);  // Gate gets voltage → motor ON
  } else {
    digitalWrite(motorPin, LOW);   // Gate pulled LOW by Arduino → motor OFF
  }
}