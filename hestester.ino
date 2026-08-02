const int hallPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(hallPin, INPUT_PULLUP);
}

void loop() {
  Serial.println(digitalRead(hallPin));
}