#include <Stepper.h>
#include <Servo.h>

#define LEFT_BTN_PIN 4
#define RIGHT_BTN_PIN 5
#define LOWER_BOUND -2000
#define UPPER_BOUND 2000

const int stepsPerRevolution = 2000;
Stepper myStepper = Stepper(stepsPerRevolution, 12, 10, 11, 9);

volatile int steps = 0;
int pos = 0;

Servo myServo;
int firePos = 0;

void setup() {
  pinMode(LEFT_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(LEFT_BTN_PIN), rotateISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_BTN_PIN), rotateISR, CHANGE);
  myStepper.setSpeed(15);
  myServo.attach(6);
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


void fire() {
  for (firePos = 0; firePos <= 180; firePos += 1) {
    myServo.write(firePos);
    delay(15);
  }
}


void unfire() {
  for (firePos = 180; firePos >= 0; firePos -= 1) {
    myServo.write(firePos);
    delay(15);
  }
}
