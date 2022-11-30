#include <Stepper.h>

#define LEFT_BTN_PIN 4
#define RIGHT_BTN_PIN 5
#define LOWER_BOUND -500
#define UPPER_BOUND 500

const int stepsPerRevolution = 2000;
Stepper myStepper = Stepper(stepsPerRevolution, 12, 10, 11, 9);

volatile int steps = 0;
int pos = 0;

void setup() {
  pinMode(LEFT_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(LEFT_BTN_PIN), rotateISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_BTN_PIN), rotateISR, CHANGE);
  myStepper.setSpeed(15);
}

void loop() {
  if (pos <= LOWER_BOUND && steps < 0) return;
  if (pos >= UPPER_BOUND && steps > 0) return;
  
  myStepper.step(steps);
  pos += steps;
}

void rotateISR() {
  if (digitalRead(LEFT_BTN_PIN) == HIGH) {
    steps = 1;
  } else if (digitalRead(RIGHT_BTN_PIN) == HIGH) {
    steps = -1;
  } else {
    steps = 0;
  }
}

void reset() {
  if (pos < 0) {
    while (pos < 0) {
      myStepper.step(1);
      pos++;
    }
  } else {
    while (pos > 0) {
      myStepper.step(-1);
      pos--;
    }
  }
}
