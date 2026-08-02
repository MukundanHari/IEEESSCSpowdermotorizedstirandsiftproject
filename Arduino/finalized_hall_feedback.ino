#include <Servo.h>
#include <Adafruit_Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



// Hall feedback additions
const int hallPin = 3;
volatile unsigned long hallPulses=0;
unsigned long lastRPMTime=0;
unsigned long lastPulseCount=0;
float lastRPM=0;
float pwmCorrection=0;
const float kP=0.15;

// ==================================================
// OLED
// ==================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);


// ==================================================
// SERVO
// ==================================================

Servo myServo;

const int servoPin = 2;
const int servoButtonPin = A2;

int angle = 0;
int direction = 1;

unsigned long previousServoMillis = 0;
const unsigned long servoInterval = 8;


// ==================================================
// DC MOTOR + MOMENTARY BUTTON
// ==================================================

const int motorButtonPin = A3;
const int motorPin = 11;


// ==================================================
// MOTOR SPEED POTENTIOMETER
// ==================================================

const int motorSpeedPotPin = A6;


// ==================================================
// BUZZER
// ==================================================

const int buzzerPin = 12;


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
// Arduino:     D4   D5   D6   D7   D8   D9   D10

byte rowPins[ROWS] = {
  5,    // R1
  10,   // R2
  9,    // R3
  7     // R4
};

byte colPins[COLS] = {
  6,    // C1
  4,    // C2
  8     // C3
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
// SERVO BUTTON STATE
// ==================================================

bool previousServoButtonState = HIGH;


// These are global so the OLED function can access them.

bool motorButtonPressed = false;
bool servoButtonPressed = false;


// ==================================================
// BUZZER MORSE CODE
// ==================================================

struct MorseSymbol {
  bool isBeep;
  unsigned long duration;
};

MorseSymbol morseSequence[100];

int morseLength = 0;
int morsePosition = 0;

bool morsePlaying = false;
unsigned long previousMorseMillis = 0;


// Morse timing

const unsigned long morseDot = 70;
const unsigned long morseDash = 210;
const unsigned long morseSymbolGap = 70;
const unsigned long morseLetterGap = 210;


// ==================================================
// STARTUP MELODY
// ==================================================

int startupNote = 0;

const int startupNotes[] = {
  523, 659, 784, 1047
};

const unsigned long startupDurations[] = {
  120, 120, 120, 300
};

const int startupNoteCount = 4;

bool startupPlaying = true;

unsigned long startupStartMillis = 0;


// ==================================================
// OLED SCROLLING TEXT
// ==================================================

const char scrollingText[] =
  "Team WGHS - Mukundan, Nathan and Vedant";

int scrollX = SCREEN_WIDTH;

unsigned long previousScrollMillis = 0;

const unsigned long scrollInterval = 40;


// ==================================================
// OLED UPDATE TIMING
// ==================================================

unsigned long previousDisplayMillis = 0;

const unsigned long displayUpdateInterval = 100;


// ==================================================
// MORSE FUNCTIONS
// ==================================================

void addMorseBeep(unsigned long duration) {

  morseSequence[morseLength].isBeep = true;
  morseSequence[morseLength].duration = duration;

  morseLength++;
}


void addMorseSilence(unsigned long duration) {

  morseSequence[morseLength].isBeep = false;
  morseSequence[morseLength].duration = duration;

  morseLength++;
}


void addMorseLetter(const char* morseCode) {

  for (int i = 0; morseCode[i] != '\0'; i++) {

    if (morseCode[i] == '.') {

      addMorseBeep(morseDot);
      addMorseSilence(morseSymbolGap);
    }

    else if (morseCode[i] == '-') {

      addMorseBeep(morseDash);
      addMorseSilence(morseSymbolGap);
    }
  }

  // Extra gap between letters

  addMorseSilence(morseLetterGap);
}


void startMorse(const char* message) {

  morseLength = 0;
  morsePosition = 0;
  morsePlaying = true;

  for (int i = 0; message[i] != '\0'; i++) {

    if (message[i] == 'S') {
      addMorseLetter("...");
    }

    else if (message[i] == 'T') {
      addMorseLetter("-");
    }

    else if (message[i] == 'I') {
      addMorseLetter("..");
    }

    else if (message[i] == 'R') {
      addMorseLetter(".-.");
    }

    else if (message[i] == 'F') {
      addMorseLetter("..-.");
    }
  }

  previousMorseMillis = millis();

  if (morseSequence[0].isBeep) {
    digitalWrite(buzzerPin, HIGH);
  }

  else {
    digitalWrite(buzzerPin, LOW);
  }
}


void updateMorse() {

  if (!morsePlaying) {
    return;
  }

  unsigned long currentMillis = millis();

  if (
    currentMillis - previousMorseMillis >=
    morseSequence[morsePosition].duration
  ) {

    previousMorseMillis = currentMillis;

    morsePosition++;

    if (morsePosition >= morseLength) {

      morsePlaying = false;

      digitalWrite(buzzerPin, LOW);

      return;
    }

    if (morseSequence[morsePosition].isBeep) {

      digitalWrite(buzzerPin, HIGH);
    }

    else {

      digitalWrite(buzzerPin, LOW);
    }
  }
}


// ==================================================
// STARTUP MELODY FUNCTION
// ==================================================

void updateStartupMelody() {

  if (!startupPlaying) {
    return;
  }

  unsigned long currentMillis = millis();


  if (
    startupNote == 0 &&
    startupStartMillis == 0
  ) {

    startupStartMillis = currentMillis;

    tone(
      buzzerPin,
      startupNotes[startupNote]
    );
  }


  if (
    currentMillis - startupStartMillis >=
    startupDurations[startupNote]
  ) {

    startupNote++;

    startupStartMillis = currentMillis;


    if (startupNote >= startupNoteCount) {

      noTone(buzzerPin);

      startupPlaying = false;
    }

    else {

      tone(
        buzzerPin,
        startupNotes[startupNote]
      );
    }
  }
}


// ==================================================
// OLED DISPLAY
// ==================================================

void updateOLED() {

  unsigned long currentMillis = millis();


  // ------------------------------------------------
  // UPDATE SCROLLING TEXT
  // ------------------------------------------------

  if (
    currentMillis - previousScrollMillis >=
    scrollInterval
  ) {

    previousScrollMillis = currentMillis;

    scrollX--;

    int textWidth =
      strlen(scrollingText) * 6;

    if (scrollX < -textWidth) {

      scrollX = SCREEN_WIDTH;
    }
  }


  // ------------------------------------------------
  // UPDATE OLED PERIODICALLY
  // ------------------------------------------------

  if (
    currentMillis - previousDisplayMillis <
    displayUpdateInterval
  ) {

    return;
  }

  previousDisplayMillis = currentMillis;


  // ------------------------------------------------
  // DETERMINE STIR STATUS
  // ------------------------------------------------

  bool stirActive =
    motorButtonPressed ||
    keypadTimerRunning;


  // ------------------------------------------------
  // DETERMINE SIFT STATUS
  // ------------------------------------------------

  bool siftActive =
    servoButtonPressed;


  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);


  // ------------------------------------------------
  // TIME DISPLAY
  // ------------------------------------------------

  display.setTextSize(1);

  display.setCursor(
    0,
    0
  );

  display.print(
    "Time: "
  );


  if (
    keypadTimerRunning &&
    !timerPaused
  ) {

    unsigned long timeLeft =
      timerEndTime - millis();

    unsigned long secondsLeft =
      (timeLeft + 999) / 1000;

    display.print(
      secondsLeft
    );
  }

  else if (
    keypadTimerRunning &&
    timerPaused
  ) {

    display.print(
      (remainingTime + 999) / 1000
    );
  }

  else {

    display.print(
      enteredTime
    );
  }

  display.println(
    "s"
  );


  // ------------------------------------------------
  // STIR STATUS
  // ------------------------------------------------

  display.setCursor(
    0,
    16
  );

  display.print(
    "Stir: "
  );

  if (
    stirActive
  ) {

    display.println(
      "Active"
    );
  }

  else {

    display.println(
      "Ready"
    );
  }


  // ------------------------------------------------
  // SIFT STATUS
  // ------------------------------------------------

  display.setCursor(
    0,
    28
  );

  display.print(
    "Sift: "
  );

  if (
    siftActive
  ) {

    display.println(
      "Active"
    );
  }

  else {

    display.println(
      "Ready"
    );
  }


  // ------------------------------------------------
  // SCROLLING TEAM TEXT
  // ------------------------------------------------

  display.setCursor(
    scrollX,
    54
  );

  display.print(
    scrollingText
  );


  display.display();
}


// ==================================================
// SETUP
// ==================================================


void hallISR(){
  pinMode(hallPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hallPin), hallISR, FALLING);
 hallPulses++; }

float getRPM(){
  unsigned long now=millis();
  if(now-lastRPMTime>=100){
    noInterrupts();
    unsigned long p=hallPulses;
    interrupts();
    unsigned long d=p-lastPulseCount;
    lastPulseCount=p;
    lastRPM=d*600.0; //1 pulse/rev,100ms
    lastRPMTime=now;
  }
  return lastRPM;
}

void setup() {

  // ------------------------------------------------
  // SERVO
  // ------------------------------------------------

  myServo.attach(
    servoPin
  );

  myServo.write(
    angle
  );

  pinMode(
    servoButtonPin,
    INPUT_PULLUP
  );


  // ------------------------------------------------
  // DC MOTOR
  // ------------------------------------------------

  pinMode(
    motorButtonPin,
    INPUT_PULLUP
  );

  pinMode(
    motorPin,
    OUTPUT
  );

  analogWrite(
    motorPin,
    0
  );


  // ------------------------------------------------
  // BUZZER
  // ------------------------------------------------

  pinMode(
    buzzerPin,
    OUTPUT
  );


  // ------------------------------------------------
  // KEYPAD
  // ------------------------------------------------

  customKeypad.begin();


  // ------------------------------------------------
  // SERIAL MONITOR
  // ------------------------------------------------

  Serial.begin(
    115200
  );


  // ------------------------------------------------
  // OLED
  // ------------------------------------------------

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
    )
  ) {

    Serial.println(
      "OLED allocation failed."
    );
  }


  else {

    display.clearDisplay();

    display.setTextColor(
      SSD1306_WHITE
    );

    display.setTextSize(
      1
    );

    display.setCursor(
      0,
      0
    );

    display.println(
      "Team WGHS"
    );

    display.setCursor(
      0,
      16
    );

    display.println(
      "Protein Shaker"
    );

    display.display();
  }


  Serial.println(
    "System ready."
  );
}


// ==================================================
// LOOP
// ==================================================

void loop() {


  // ==================================================
  // STARTUP MELODY
  // ==================================================

  updateStartupMelody();


  // ==================================================
  // SERVO OSCILLATION
  // ==================================================

  servoButtonPressed =
    (
      digitalRead(
        servoButtonPin
      ) == LOW
    );


  if (
    servoButtonPressed
  ) {

    unsigned long currentServoMillis =
      millis();


    if (
      currentServoMillis -
      previousServoMillis >=
      servoInterval
    ) {

      previousServoMillis =
        currentServoMillis;


      angle +=
        direction * 3;


      if (
        angle >= 180
      ) {

        angle = 180;

        direction = -1;
      }


      if (
        angle <= 0
      ) {

        angle = 0;

        direction = 1;
      }


      myServo.write(
        angle
      );
    }
  }


  else {

    // Hold current servo position

    myServo.write(
      angle
    );
  }


  // ==================================================
  // DETECT START OF SIFTING
  // ==================================================

  if (
    servoButtonPressed &&
    previousServoButtonState == HIGH
  ) {

    startMorse(
      "SIFT"
    );
  }


  previousServoButtonState =
    servoButtonPressed
    ? LOW
    : HIGH;


  // ==================================================
  // READ DC MOTOR BUTTON
  // ==================================================

  motorButtonPressed =
    (
      digitalRead(
        motorButtonPin
      ) == LOW
    );


  // ==================================================
  // MOMENTARY BUTTON PRESSED
  // ==================================================

  if (
    motorButtonPressed &&
    previousMotorButtonState == HIGH
  ) {


    // If a keypad timer is currently running,
    // pause it and save the remaining time.

    if (
      keypadTimerRunning &&
      !timerPaused
    ) {

      remainingTime =
        timerEndTime -
        millis();

      timerPaused =
        true;


      Serial.print(
        "Timer paused. Remaining time: "
      );

      Serial.print(
        remainingTime / 1000
      );

      Serial.println(
        " seconds."
      );
    }


    // Morse-code STIR when stirring begins

    startMorse(
      "STIR"
    );
  }


  // ==================================================
  // MOMENTARY BUTTON RELEASED
  // ==================================================

  if (
    !motorButtonPressed &&
    previousMotorButtonState == LOW
  ) {


    // Resume the keypad timer after releasing
    // the momentary button.

    if (
      keypadTimerRunning &&
      timerPaused
    ) {

      timerEndTime =
        millis() +
        remainingTime;

      timerPaused =
        false;


      Serial.print(
        "Timer resumed. Remaining time: "
      );

      Serial.print(
        remainingTime / 1000
      );

      Serial.println(
        " seconds."
      );
    }
  }


  previousMotorButtonState =
    motorButtonPressed
    ? LOW
    : HIGH;


  // ==================================================
  // KEYPAD
  // ==================================================

  customKeypad.tick();


  while (
    customKeypad.available()
  ) {

    keypadEvent e =
      customKeypad.read();


    if (
      e.bit.EVENT ==
      KEY_JUST_PRESSED
    ) {

      char key =
        (char)e.bit.KEY;


      // ------------------------------------------------
      // IGNORE KEYPAD WHILE MOMENTARY BUTTON IS HELD
      // ------------------------------------------------

      if (
        motorButtonPressed
      ) {

        continue;
      }


      // ------------------------------------------------
      // NUMBER PRESSED
      // ------------------------------------------------

      if (
        key >= '0' &&
        key <= '9'
      ) {

        enteredTime =
          enteredTime * 10 +
          (key - '0');


        Serial.print(
          "Entered time: "
        );

        Serial.println(
          enteredTime
        );
      }


      // ------------------------------------------------
      // STAR BUTTON
      // ------------------------------------------------

      else if (
        key == '*'
      ) {


        // ============================================
        // IMPORTANT:
        //
        // * ALWAYS CLEARS THE ENTERED TIME.
        //
        // If the motor timer is running, it ALSO
        // STOPS THE MOTOR BY STOPPING THE TIMER.
        // ============================================

        enteredTime =
          0;


        if (
          keypadTimerRunning
        ) {

          keypadTimerRunning =
            false;

          timerPaused =
            false;

          remainingTime =
            0;

          timerEndTime =
            0;


          Serial.println(
            "Timer stopped and reset."
          );
        }

        else {

          Serial.println(
            "Time entry cleared."
          );
        }
      }


      // ------------------------------------------------
      // HASH BUTTON
      // ------------------------------------------------

      else if (
        key == '#'
      ) {

        if (
          enteredTime > 0
        ) {

          remainingTime =
            enteredTime *
            1000UL;


          timerEndTime =
            millis() +
            remainingTime;


          keypadTimerRunning =
            true;


          timerPaused =
            false;


          Serial.print(
            "Motor running for "
          );

          Serial.print(
            enteredTime
          );

          Serial.println(
            " seconds."
          );


          // Clear the entry after starting the timer.
          // OLED now shows the countdown instead.

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
    keypadTimerRunning &&
    !timerPaused
  ) {

    if (
      millis() >=
      timerEndTime
    ) {

      keypadTimerRunning =
        false;


      remainingTime =
        0;


      timerEndTime =
        0;


      Serial.println(
        "Timer finished."
      );
    }
  }


  // ==================================================
  // READ MOTOR SPEED POTENTIOMETER
  // ==================================================

  int potValue =
    analogRead(
      motorSpeedPotPin
    );


  // ==================================================
  // CONVERT POTENTIOMETER POSITION TO MOTOR PWM
  // ==================================================

  // 0 kΩ   → PWM 255 → Full speed
  // 50 kΩ  → PWM ~127 → Half speed
  // 100 kΩ → PWM 0 → Motor stopped

  int motorPWM =
    map(
      potValue,
      0,
      1023,
      255,
      0
    );


  motorPWM =
    constrain(
      motorPWM,
      0,
      255
    );

  float currentRPM = getRPM();
  static float previousRPM = 0;
  if (currentRPM > 0 && previousRPM > 0) {
    pwmCorrection += kP * (previousRPM - currentRPM);
  }
  previousRPM = currentRPM;
  motorPWM = constrain((int)(motorPWM + pwmCorrection),0,255);


  // ==================================================
  // DC MOTOR CONTROL
  // ==================================================

  if (
    motorButtonPressed
  ) {

    // Momentary button has priority.
    // Motor runs while held.
    // Speed is controlled by potentiometer.

    analogWrite(
      motorPin,
      motorPWM
    );
  }


  else if (
    keypadTimerRunning
  ) {

    // Keypad timer controls motor.
    // Speed is controlled by potentiometer.

    analogWrite(
      motorPin,
      motorPWM
    );
  }


  else {

    // Motor off.

    analogWrite(
      motorPin,
      0
    );

    pwmCorrection = 0;
    previousRPM = 0;
  }


  // ==================================================
  // UPDATE MORSE CODE
  // ==================================================

  updateMorse();


  // ==================================================
  // UPDATE OLED
  // ==================================================

  updateOLED();
}
