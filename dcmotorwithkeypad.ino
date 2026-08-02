#include <Adafruit_Keypad.h>

// =========================
// KEYPAD
// =========================

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Physical keypad wiring, left to right:
// C2 -> D6
// R1 -> D7
// C1 -> D8
// R4 -> D9
// C3 -> D10
// R3 -> D11
// R2 -> D12

byte rowPins[ROWS] = {7, 12, 11, 9};
byte colPins[COLS] = {8, 6, 10};

Adafruit_Keypad customKeypad = Adafruit_Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);


// =========================
// MOTOR AND BUTTON
// =========================

const int buttonPin = A3;
const int motorPin = 3;


// =========================
// TIMER VARIABLES
// =========================

unsigned long enteredTime = 0;

// Remaining keypad timer time, in milliseconds
unsigned long remainingTime = 0;

// When the currently active timer period ends
unsigned long timerEndTime = 0;

bool keypadTimerRunning = false;
bool timerPaused = false;


// =========================
// MOMENTARY BUTTON STATE
// =========================

bool previousButtonState = HIGH;


// =========================
// SETUP
// =========================

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);

  digitalWrite(motorPin, LOW);

  customKeypad.begin();

  Serial.begin(115200);

  Serial.println("System ready.");
}


// =========================
// MAIN LOOP
// =========================

void loop() {

  customKeypad.tick();

  bool buttonPressed = (digitalRead(buttonPin) == LOW);


  // ============================================
  // MOMENTARY BUTTON PRESSED
  // ============================================

  if (buttonPressed && previousButtonState == HIGH) {

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


  // ============================================
  // MOMENTARY BUTTON RELEASED
  // ============================================

  if (!buttonPressed && previousButtonState == LOW) {

    // If the timer was paused, resume it.

    if (keypadTimerRunning && timerPaused) {

      timerEndTime = millis() + remainingTime;

      timerPaused = false;

      Serial.print("Timer resumed. Remaining time: ");
      Serial.print(remainingTime / 1000);
      Serial.println(" seconds.");
    }
  }


  previousButtonState = buttonPressed ? LOW : HIGH;


  // ============================================
  // KEYPAD INPUT
  // ============================================

  while (customKeypad.available()) {

    keypadEvent e = customKeypad.read();

    if (e.bit.EVENT == KEY_JUST_PRESSED) {

      char key = (char)e.bit.KEY;


      // ========================================
      // IF MOMENTARY BUTTON IS HELD
      // ========================================

      if (buttonPressed) {

        // Ignore keypad input while the
        // momentary button is being held.

        continue;
      }


      // ========================================
      // NUMBER
      // ========================================

      if (key >= '0' && key <= '9') {

        enteredTime = enteredTime * 10 + (key - '0');

        Serial.print("Entered time: ");
        Serial.println(enteredTime);
      }


      // ========================================
      // STAR
      // ========================================

      else if (key == '*') {

        if (keypadTimerRunning) {

          // While the timer is running,
          // * stops and resets the timer.

          keypadTimerRunning = false;
          timerPaused = false;
          remainingTime = 0;
          enteredTime = 0;

          Serial.println("Timer stopped and reset.");
        }

        else {

          // While entering a time,
          // * clears the entered number.

          enteredTime = 0;

          Serial.println("Time entry cleared.");
        }
      }


      // ========================================
      // HASH
      // ========================================

      else if (key == '#') {

        if (enteredTime > 0) {

          remainingTime = enteredTime * 1000UL;

          timerEndTime = millis() + remainingTime;

          keypadTimerRunning = true;
          timerPaused = false;

          Serial.print("Motor running for ");
          Serial.print(enteredTime);
          Serial.println(" seconds.");

          enteredTime = 0;
        }
      }
    }
  }


  // ============================================
  // TIMER COUNTDOWN
  // ============================================

  if (keypadTimerRunning && !timerPaused) {

    if (millis() >= timerEndTime) {

      keypadTimerRunning = false;
      remainingTime = 0;

      Serial.println("Timer finished.");
    }
  }


  // ============================================
  // MOTOR CONTROL
  // ============================================

  if (buttonPressed) {

    // Momentary button has priority.
    // Motor runs while held.

    digitalWrite(motorPin, HIGH);
  }

  else if (keypadTimerRunning) {

    // Keypad timer controls motor.

    digitalWrite(motorPin, HIGH);
  }

  else {

    // Nothing is controlling the motor.

    digitalWrite(motorPin, LOW);
  }
}