#include <Servo.h>
#include <Adafruit_Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// ==================================================
// OLED
// ==================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET);


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
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};


// Physical keypad wiring, LEFT TO RIGHT:
//
// Keypad pin:  C2   R1   C1   R4   C3   R3   R2
// Arduino:     D6   D7   D8   D9   D10  D11  D12

byte rowPins[ROWS] = {
  7,
  12,
  11,
  9
};

byte colPins[COLS] = {
  8,
  6,
  10
};

Adafruit_Keypad customKeypad = Adafruit_Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS);


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
// OLED UPDATE TIMER
// ==================================================

unsigned long previousDisplayMillis = 0;

const unsigned long displayUpdateInterval = 100;


// ==================================================
// SETUP
// ==================================================

void setup() {

  // ==================================================
  // SERVO
  // ==================================================

  myServo.attach(servoPin);
  myServo.write(angle);

  pinMode(servoButtonPin, INPUT_PULLUP);


  // ==================================================
  // DC MOTOR
  // ==================================================

  pinMode(motorButtonPin, INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);

  digitalWrite(motorPin, LOW);


  // ==================================================
  // KEYPAD
  // ==================================================

  customKeypad.begin();


  // ==================================================
  // SERIAL
  // ==================================================

  Serial.begin(115200);

  Serial.println("System ready.");


  // ==================================================
  // OLED
  // ==================================================

  Wire.begin();

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS)) {

    Serial.println("OLED allocation failed.");

    while (true)
      ;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("System Ready");

  display.display();

  delay(1000);
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

    if (
      currentServoMillis - previousServoMillis >= servoInterval) {

      previousServoMillis =
        currentServoMillis;

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

  }

  else {

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

  if (
    motorButtonPressed && previousMotorButtonState == HIGH) {

    if (
      keypadTimerRunning && !timerPaused) {

      remainingTime =
        timerEndTime - millis();

      timerPaused = true;

      Serial.print(
        "Timer paused. Remaining time: ");

      Serial.print(
        remainingTime / 1000);

      Serial.println(" seconds.");
    }
  }


  // ==================================================
  // MOMENTARY BUTTON RELEASED
  // ==================================================

  if (
    !motorButtonPressed && previousMotorButtonState == LOW) {

    if (
      keypadTimerRunning && timerPaused) {

      timerEndTime =
        millis() + remainingTime;

      timerPaused = false;

      Serial.print(
        "Timer resumed. Remaining time: ");

      Serial.print(
        remainingTime / 1000);

      Serial.println(" seconds.");
    }
  }


  previousMotorButtonState =
    motorButtonPressed ? LOW : HIGH;


  // ==================================================
  // KEYPAD
  // ==================================================

  customKeypad.tick();


  while (
    customKeypad.available()) {

    keypadEvent e =
      customKeypad.read();


    if (
      e.bit.EVENT == KEY_JUST_PRESSED) {

      char key =
        (char)e.bit.KEY;


      // ----------------------------------------------
      // IGNORE KEYPAD WHILE MOMENTARY BUTTON IS HELD
      // ----------------------------------------------

      if (motorButtonPressed) {

        continue;
      }


      // ----------------------------------------------
      // NUMBER PRESSED
      // ----------------------------------------------

      if (
        key >= '0' && key <= '9') {

        enteredTime =
          enteredTime * 10 + (key - '0');

        Serial.print(
          "Entered time: ");

        Serial.println(
          enteredTime);
      }


      // ----------------------------------------------
      // STAR BUTTON
      // ----------------------------------------------

      else if (
        key == '*') {

        if (
          keypadTimerRunning) {

          keypadTimerRunning =
            false;

          timerPaused =
            false;

          remainingTime =
            0;

          enteredTime =
            0;

          Serial.println(
            "Timer stopped and reset.");
        }

        else {

          enteredTime =
            0;

          Serial.println(
            "Time entry cleared.");
        }
      }


      // ----------------------------------------------
      // HASH BUTTON
      // ----------------------------------------------

      else if (
        key == '#') {

        if (
          enteredTime > 0) {

          remainingTime =
            enteredTime * 1000UL;

          timerEndTime =
            millis() + remainingTime;

          keypadTimerRunning =
            true;

          timerPaused =
            false;

          Serial.print(
            "Motor running for ");

          Serial.print(
            enteredTime);

          Serial.println(
            " seconds.");

          enteredTime =
            0;
        }
      }
    }
  }


  // ==================================================
  // CHECK IF KEYPAD TIMER FINISHED
  // ==================================================

  if (
    keypadTimerRunning && !timerPaused) {

    if (
      millis() >= timerEndTime) {

      keypadTimerRunning =
        false;

      remainingTime =
        0;

      Serial.println(
        "Timer finished.");
    }
  }


  // ==================================================
  // DC MOTOR CONTROL
  // ==================================================

  if (
    motorButtonPressed) {

    digitalWrite(
      motorPin,
      HIGH);
  }

  else if (
    keypadTimerRunning) {

    digitalWrite(
      motorPin,
      HIGH);
  }

  else {

    digitalWrite(
      motorPin,
      LOW);
  }


  // ==================================================
  // OLED DISPLAY
  // ==================================================

  if (
    millis() - previousDisplayMillis >= displayUpdateInterval) {

    previousDisplayMillis =
      millis();


    display.clearDisplay();


    // ==================================================
    // TOP: TIME
    // ==================================================

    display.setTextSize(1);
    display.setCursor(0, 0);


    if (
      keypadTimerRunning) {

      display.print(
        "Remaining Time: ");


      unsigned long displayRemainingTime;


      if (
        timerPaused) {

        displayRemainingTime =
          remainingTime;
      }

      else {

        displayRemainingTime =
          timerEndTime - millis();
      }


      unsigned long secondsLeft =
        displayRemainingTime / 1000;


      display.print(
        secondsLeft);

      display.println(
        " s");
    }

    else {

      display.print(
        "Enter Time: ");


      if (
        enteredTime > 0) {

        display.print(
          enteredTime);

        display.println(
          " s");
      }

      else {

        display.println();
      }
    }


    // ==================================================
    // STIR STATUS
    // ==================================================

    display.setCursor(0, 20);

    display.print(
      "Stir: ");


    if (
      keypadTimerRunning || motorButtonPressed) {

      display.println(
        "Active");
    }

    else {

      display.println(
        "Ready");
    }


    // ==================================================
    // SIFT STATUS
    // ==================================================

    display.setCursor(0, 32);

    display.print(
      "Sift: ");


    if (
      digitalRead(servoButtonPin) == LOW) {

      display.println(
        "Active");
    }

    else {

      display.println(
        "Ready");
    }


    // ==================================================
    // OVERALL STATUS
    // ==================================================

    display.setCursor(0, 52);

    display.print(
      "Status: ");


    if (
      keypadTimerRunning || motorButtonPressed || digitalRead(servoButtonPin) == LOW) {

      display.println(
        "Active");
    }

    else {

      display.println(
        "Ready");
    }


    display.display();
  }
}
