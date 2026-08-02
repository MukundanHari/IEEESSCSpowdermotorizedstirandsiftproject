# Arduino Source Code

This folder contains the Arduino programs developed throughout the project, including early prototypes, subsystem tests, and the final integrated software. All files except **HESTester** were developed by the project's software lead, Vedant.

## ServoGrateOscillation

ServoGrateOscillation is the simple program that oscillates the servo that was intended for to be mounted under the grate for the sifting mechanism.

## dcmotorstirmomentarypush

dcmotorstirmomentarypush is the program for using the momentary push with the dc motor. When pressed, the motor rotates, and vice versa.

## dcmotorwithkeypad

dcmotorwithkeypad is the program that combines the aforementioned program with the keypad. Every thing is the same, but when having the motor rotate when momentary push is pressed, the keypad doesn't work. When the keypad time is passing by, if the momentary push is pressed, then the remaining time for the keypad is paused, and will continue once the button is unpressed.

## finalized_hall_feedback

finalized_hall_feedback is the final program for this project, which integrates all of the mechanisms that are intended, which are the OLED displaying the status of the stir/sift, the rotating text for our team, as well as the entry space for the keypad time which ticks down and follows the rules of the previous program (the push stopping the keypad). There is also a PI control system on the stir mechanism to maintain a consistent rotation speed regardless of the resistance to the motor, which is done by using the hall effect sensor (HES) as an RPM tracker and attaching a magnet on the stir fan. This was programmed and tuned to maintain whatever speed, either lowering or raising the duty cycle if there is too less or too much resistance to the motor. Additionally, there is a potentiometer that changes the baseline speed that the control system seeks to maintain, and a buzzer that uses morse code to say the stir/sift mechanisms if the respective momentary pushes are pressed to help identify the mechanisms for disabled folk.

## hestester

hestester was the program that was used following the mounting of the hall effect sensor (HES) on the inner enclosure of the mechanical build. It was to test if the magnets would trigger a signal from the HES. We used a DRV5033 which is open drain, so the pin must be treated as an `INPUT_PULLUP` pin in the code, or with an external resistor between the out pin of the HES and the arduino pin, in our case, the D3 pin.

## motorkeypadandstir

 motorkeypadandstir is the program that combines the sift and the stir mechanism that was explained.

## stirsiftkeypadOLED

motorkeypadandstir with the OLED interface that I had explained.
