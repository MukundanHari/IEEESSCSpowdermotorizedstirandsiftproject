#include <Servo.h>
#include <Adafruit_Keypad.h>


// ==================================================
// SERVO
// ==================================================

Servo myServo;

const int servoPin = 5;
const int servoButtonPin = A2;

int angle = 0;
int direction = 1;

unsigned long previousServoMillis = 0;
const unsigned long servoInterval = 8;


// ==================================================
// DC MOTOR + MOMENTARY BUTTON
// ==================================================

const int motorButtonPin = A3;
const int motorPin = 3;


// ==================================================
// KEYPAD
// ==================================================

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};


// Physical keypad wiring, LEFT TO RIGHT:
//
// Keypad pin:  C2   R1   C1   R4   C3   R3   R2
// Arduino:     D6   D7   D8   D9   D10  D11  D12

byte rowPins[ROWS] = {
  7,    // R1
  12,   // R2
  11,   // R3
  9     // R4
};

byte colPins[COLS] = {
  8,    // C1
  6,    // C2
  10    // C3
};

Adafruit_Keypad customKeypad = Adafruit_Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);


// ==================================================
// DC MOTOR TIMER VARIABLES
// ==================================================

unsigned long enteredTime = 0;

unsigned long remainingTime = 0;
unsigned long timerEndTime = 0;

bool keypadTimerRunning = false;
bool timerPaused = false;


// ==================================================
// MOMENTARY BUTTON STATE
// ==================================================

bool previousMotorButtonState = HIGH;


// ==================================================
// SETUP
// ==================================================

void setup() {

  // Servo
  myServo.attach(servoPin);
  myServo.write(angle);

  pinMode(servoButtonPin, INPUT_PULLUP);


  // DC motor
  pinMode(motorButtonPin, INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);

  digitalWrite(motorPin, LOW);


  // Keypad
  customKeypad.begin();


  // Serial monitor
  Serial.begin(115200);

  Serial.println("System ready.");
}


// ==================================================
// LOOP
// ==================================================

void loop() {

  // ==================================================
  // SERVO OSCILLATION
  // ==================================================

  if (digitalRead(servoButtonPin) == LOW) {

    unsigned long currentServoMillis = millis();

    if (currentServoMillis - previousServoMillis >= servoInterval) {

      previousServoMillis = currentServoMillis;

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

    // Hold current servo position
    myServo.write(angle);
  }


  // ==================================================
  // READ DC MOTOR BUTTON
  // ==================================================

  bool motorButtonPressed =
    (digitalRead(motorButtonPin) == LOW);


  // ==================================================
  // MOMENTARY BUTTON PRESSED
  // ==================================================

  if (motorButtonPressed &&
      previousMotorButtonState == HIGH) {

    // If a keypad timer is currently running,
    // pause it and save the remaining time.

    if (keypadTimerRunning && !timerPaused) {

      remainingTime = timerEndTime - millis();

      timerPaused = true;

      Serial.print("Timer paused. Remaining time: ");
      Serial.print(remainingTime / 1000);
      Serial.println(" seconds.");
    }
  }


  // ==================================================
  // MOMENTARY BUTTON RELEASED
  // ==================================================

  if (!motorButtonPressed &&
      previousMotorButtonState == LOW) {

    // Resume the keypad timer after releasing
    // the momentary button.

    if (keypadTimerRunning && timerPaused) {

      timerEndTime = millis() + remainingTime;

      timerPaused = false;

      Serial.print("Timer resumed. Remaining time: ");
      Serial.print(remainingTime / 1000);
      Serial.println(" seconds.");
    }
  }


  previousMotorButtonState =
    motorButtonPressed ? LOW : HIGH;


  // ==================================================
  // KEYPAD
  // ==================================================

  customKeypad.tick();


  while (customKeypad.available()) {

    keypadEvent e = customKeypad.read();

    if (e.bit.EVENT == KEY_JUST_PRESSED) {

      char key = (char)e.bit.KEY;


      // ------------------------------------------------
      // IGNORE KEYPAD WHILE MOMENTARY BUTTON IS HELD
      // ------------------------------------------------

      if (motorButtonPressed) {
        continue;
      }


      // ------------------------------------------------
      // NUMBER PRESSED
      // ------------------------------------------------

      if (key >= '0' && key <= '9') {

        enteredTime =
          enteredTime * 10 + (key - '0');

        Serial.print("Entered time: ");
        Serial.println(enteredTime);
      }


      // ------------------------------------------------
      // STAR BUTTON
      // ------------------------------------------------

      else if (key == '*') {

        if (keypadTimerRunning) {

          // While timer is running:
          // * = STOP AND RESET

          keypadTimerRunning = false;
          timerPaused = false;

          remainingTime = 0;
          enteredTime = 0;

          Serial.println(
            "Timer stopped and reset."
          );
        }

        else {

          // While entering a time:
          // * = CLEAR ENTRY

          enteredTime = 0;

          Serial.println(
            "Time entry cleared."
          );
        }
      }


      // ------------------------------------------------
      // HASH BUTTON
      // ------------------------------------------------

      else if (key == '#') {

        if (enteredTime > 0) {

          remainingTime =
            enteredTime * 1000UL;

          timerEndTime =
            millis() + remainingTime;

          keypadTimerRunning = true;
          timerPaused = false;

          Serial.print(
            "Motor running for "
          );

          Serial.print(enteredTime);

          Serial.println(
            " seconds."
          );

          enteredTime = 0;
        }
      }
    }
  }


  // ==================================================
  // CHECK IF KEYPAD TIMER FINISHED
  // ==================================================

  if (keypadTimerRunning &&
      !timerPaused) {

    if (millis() >= timerEndTime) {

      keypadTimerRunning = false;

      remainingTime = 0;

      Serial.println(
        "Timer finished."
      );
    }
  }


  // ==================================================
  // DC MOTOR CONTROL
  // ==================================================

  if (motorButtonPressed) {

    // Momentary button has priority.
    // Motor runs while held.

    digitalWrite(
      motorPin,
      HIGH
    );
  }

  else if (keypadTimerRunning) {

    // Keypad timer controls motor.

    digitalWrite(
      motorPin,
      HIGH
    );
  }

  else {

    // Motor off.

    digitalWrite(
      motorPin,
      LOW
    );
  }
}
