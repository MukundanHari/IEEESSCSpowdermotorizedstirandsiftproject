#include <Servo.h>

Servo myServo;

const int servoPin = 5;
const int buttonPin = A2;

int angle = 0;
int direction = 1;

unsigned long previousMillis = 0;
const unsigned long interval = 8;   // Moderate speed

void setup() {
  myServo.attach(servoPin);
  myServo.write(angle);

  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      angle += direction * 3;

      if (angle >= 180) {
        angle = 180;
        direction = -1;
      }

      if (angle <= 0) {
        angle = 0;
        direction = 1;
      }

      myServo.write(angle);
    }

  } else {
    myServo.write(angle);   // Hold current position when button released
  }
}