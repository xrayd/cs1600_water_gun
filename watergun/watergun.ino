// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
#define BLYNK_TEMPLATE_ID "TMPLiGjOag8o"
#define BLYNK_DEVICE_NAME "Water Gun Trigger"
#define BLYNK_AUTH_TOKEN  "5dY6oh0BlhF6Q3s_A-WFy40wHrRq7RNO"
#define BLYNK_PRINT Serial
#define LEFT_BTN_PIN 4
#define RIGHT_BTN_PIN 5
#define LOWER_BOUND -2000
#define UPPER_BOUND 2000

#include <SPI.h>
#include <Servo.h>
#include <Stepper.h>
#include <WiFi101.h>
#include <BlynkSimpleWiFiShield101.h>
#include <Adafruit_SleepyDog.h>  // https://github.com/adafruit/Adafruit_SleepyDog/blob/master/examples/BasicUsage/BasicUsage.ino

const int stepsPerRevolution = 2000;
Stepper myStepper = Stepper(stepsPerRevolution, 12, 10, 11, 9);

volatile int steps = 0;
int pos = 0;

int servoRotation = 50;

Servo myServo;
int firePos = 0;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Brown-Guest";
char pass[] = "";

BlynkTimer timer;

int value = -1;
int prevval = -1;  // for tracking a "rising edge" of virtual button push

int last_action_time = millis();

int STATE = 0;  // 0 is idle, 1 is rotate, 2 is reset, 3 is fire, 4 is unfire

// This function is called every time the Virtual Pin 0 state changes; VP0 connected to our shoot button!
BLYNK_WRITE(V0) {  // WIFI REQUIREMENT
  // Save value of our button as a value
  prevval = value;
  value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()  // WIFI REQUIREMENT
{
  Serial.println("Connected to Blynk; waiting for shoot commands!");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  Blynk.virtualWrite(V2, millis() / 1000);
}

void setup()
{
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);

  // Setup a function to be called every second by Blynk timer
  timer.setInterval(1000L, myTimerEvent);

  pinMode(LEFT_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(LEFT_BTN_PIN), rotateISR, CHANGE);  // ISR REQUIREMENT
  attachInterrupt(digitalPinToInterrupt(RIGHT_BTN_PIN), rotateISR, CHANGE);  // ISR REQUIREMENT
  myStepper.setSpeed(15);
  myServo.attach(6);
  unfire();
  STATE = 0;

  Serial.println("Starting FSM testing...");
  testFSM(0, 40000, 0, 0, 0, 1, 1, 2, "State 2 (1)");  // tests if we reach state 2
  testFSM(0, 0, 0, 0, 0, 1, 1, 0, "State 0 (2)");  // tests if we reach state 0
  testFSM(0, 20000, 10000, 0, 1, 1, 1, 1, "State 1 (3)");  // tests if we reach state 1
  testFSM(2, 20000, 10000, 0, 50000, 1, 1, 0, "State 0 (4)");  // tests fail condition for state 1
  testFSM(2, 20000, -10000, 0, -50000, 1, 1, 0, "State 0 (5)");  // tests fail condition for state 1
  testFSM(2, 40000, 0, 0, 0, 1, 1, 0, "State 0 (6)");  // tests fail condition for state 2
  testFSM(0, 0, 0, 0, 0, 0, 1, 3, "State 3 (7)");  // tests if we reach state 3
  testFSM(0, 0, 0, 0, 0, 1, 0, 4, "State 4 (8)");  // tests if we reach state 4
  testFSM(0, 0, 0, 0, 0, 0, 0, 0, "State 0 (9)");  // tests fail condition for states 3 and 4 (prevval and val both 0)
  testFSM(0, 0, 0, 0, 0, 1, 1, 0, "State 0 (10)");  // tests fail condition for states 3 and 4 (prevval and val both 1)
}

void loop()
{
  updateFSM(STATE, millis(), steps, last_action_time, pos, prevval, value, 1);
}

int updateFSM(int state, int mils, int steps, int last, int position, int pv, int v, bool doAction)  // last parameter is to distinguish between testing and actually running
{
  if (state <= 1 && mils - last > 30000 && steps == 0){  // state transition from any other state into state 1, or a reset, occurs when no action is taken for 3 seconds
    STATE = 2;
    if (doAction){
      Serial.println("Inactive for 30 seconds, resetting...");
      last_action_time = millis();
      reset();
    }
    return STATE;
  }
  else if (steps != 0 && !(position <= LOWER_BOUND && steps < 0) && !(position >= UPPER_BOUND && steps > 0)){  // state transition into state 1 when our rotate isn't out of bounds and we've received rotate commands from ISR
    STATE = 1;
    if (doAction){
      myStepper.step(steps);  // DAC REQUIREMENT
      pos += steps;
    }
    return STATE;
  }
  else if (pv == 0 && v == 1){  // state transition into 3 happens when a rise is detected in our virtual pin, meaning the button is pressed to fire the gun!
    STATE = 3;
    if (doAction){
      Serial.println("Gun Firing!");
      fire();
      prevval = -1;  // since polling time of loop is larger than WiFi polling time, we don't want this to return True a bunch of times
      last_action_time = millis();
    }
    return STATE;
  } 
  else if (pv == 1 && v == 0) { // state transition into 4 happens when a fall is detected in our virtual pin, meaning the button is unpressed to stop firing the gun!
    STATE = 4;
    if (doAction){
      Serial.println("Gun Stopping...");
      unfire();
      prevval = -1;
      last_action_time = millis();
    }
    return STATE;
  } 
  else{  // Idle state (0) if none of the other states are active, when we poll for shooting commands and then activate shooting itself
    STATE = 0;
    if (doAction){
      Blynk.run();
      timer.run();
    }
    return STATE;
  }
}

// used to test inputs/outputs of FSM to ensure correct states
void testFSM(int state, int mils, int steps, int last, int position, int pv, int v, int correctState, String name){
  int testedState = updateFSM(state, mils, steps, last, position, pv, v, 0);
  if (testedState == correctState){
    Serial.println("Test " + name + " passed!");
  }
  else{
    Serial.println("Test " + name + " FAILED.");
  }
}

// ISR function to handle rotations upon button press
void rotateISR() {  // ISR REQUIREMENT
  if (digitalRead(LEFT_BTN_PIN) == HIGH) {
    Serial.println("Rotating Left");
    steps = 1;  // DAC REQUIREMENT
    last_action_time = millis();
  } else if (digitalRead(RIGHT_BTN_PIN) == HIGH) {
    Serial.println("Rotating Right");
    steps = -1;  // DAC REQUIREMENT
    last_action_time = millis();
  } else {
    steps = 0;  // DAC REQUIREMENT
  }
}

// Resets system by returning everything to its original position
void reset() {  // DAC REQUIREMENT
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

// Fires the gun by pulling the servo motor back
void fire() {
  int countdownMS = Watchdog.enable(50);  // WATCHDOG REQUIREMENT
  for (firePos = 0; firePos <= servoRotation; firePos += 1) {
    myServo.write(firePos);  // PWM REQUIREMENT
    delay(15);
    Watchdog.reset();
  }
  Watchdog.disable();
}

// Resets the gun to unfire more by pushing the servo motor forward
void unfire() {
  int countdownMS = Watchdog.enable(50); // WATCHDOG REQUIREMENT
  for (firePos = servoRotation; firePos >= 0; firePos -= 1) {
    myServo.write(firePos);  // PWM REQUIREMENT
    delay(15);
    Watchdog.reset();
  }
  Watchdog.disable();
}
