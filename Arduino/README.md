# Arduino Source Code

This folder contains the Arduino programs developed throughout the project, including early prototypes, subsystem tests, and the final integrated software. All files except **HESTester** were developed by the project's software lead, Vedant.

## ServoGrateOscillation

Controls the oscillating servo mounted beneath the grate to sift protein powder into the container.

## dcmotorstirmomentarypush

Controls the stirring motor using a momentary push button. The motor rotates while the button is held and stops when released.

## dcmotorwithkeypad

Adds keypad functionality to the stirring system. Entering a time and pressing **#** runs the stirrer for the specified duration. Pressing * before starting clears the entry. During operation, pressing the momentary push button pauses the countdown until it is released.

## finalized_hall_feedback

Final integrated program including:

- OLED user interface
- Keypad timer
- Servo-powered sifting
- Motorized stirring
- Hall-effect sensor RPM feedback
- Closed-loop PI motor speed control
- Rotating team information
- Accessibility buzzer feedback

The PI controller maintains a consistent stirring speed by measuring motor RPM with a DRV5033 Hall-effect sensor and adjusting the PWM duty cycle to compensate for changing load conditions. A potentiometer allows the user to adjust the target motor speed.

## HESTester

Test program used after mounting the DRV5033 Hall-effect sensor to verify reliable magnet detection. Since the DRV5033 has an open-drain output, the signal pin must be configured using `INPUT_PULLUP` (or an external pull-up resistor).

## motorkeypadandstir

Combines the stirring and sifting mechanisms into a single program.

## stirsiftkeypadOLED

Extends `motorkeypadandstir` by adding the OLED interface.
