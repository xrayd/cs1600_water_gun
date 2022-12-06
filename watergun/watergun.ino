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

Servo myServo;
int firePos = 0;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Brown-Guest";
char pass[] = "";

BlynkTimer timer;

int value = -1;
int prevval = -1;  // for tracking a "rising edge" of virtual button push

int cur_time = millis();
int last_action_time = millis();

// This function is called every time the Virtual Pin 0 state changes; VP0 connected to our shoot button!
BLYNK_WRITE(V0) {
  // Save value of our button as a value
  prevval = value;
  value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
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
  attachInterrupt(digitalPinToInterrupt(LEFT_BTN_PIN), rotateISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_BTN_PIN), rotateISR, CHANGE);
  myStepper.setSpeed(15);
  myServo.attach(6);
  unfire();
}

void loop()
{
  Blynk.run();
  timer.run();

  // TODO: add reset if we haven't pressed a button or sent a shoot command in last 30 seconds! (use millis for this)

  bool risen = detect_rise();

  if (pos <= LOWER_BOUND && steps < 0) return;
  if (pos >= UPPER_BOUND && steps > 0) return;
  
  myStepper.step(steps);
  pos += steps;

  cur_time = millis();

  if (cur_time - last_action_time > 30000){
    Serial.println("Inactive for 30 seconds, resetting...");
    last_action_time = millis();
    reset();
  }
}

// returns if the button was pressed or not by detecting rising edge
bool detect_rise()
{
  if (prevval == 0 && value == 1){
    Serial.println("Gun Firing!");
    fire();
    prevval = -1;  // since polling time of loop is larger than WiFi polling time, we don't want this to return True a bunch of times
    last_action_time = millis();
    return 1;
  } else if (prevval == 1 && value == 0) {
    Serial.println("Gun Stopping...");
    unfire();
    prevval = -1;
    last_action_time = millis();
    return 0;
  } else{
    return 0;
  }
}

// ISR function to handle rotations upon button press
void rotateISR() {
  if (digitalRead(LEFT_BTN_PIN) == HIGH) {
    Serial.println("Rotating Left");
    steps = 1;
    last_action_time = millis();
  } else if (digitalRead(RIGHT_BTN_PIN) == HIGH) {
    Serial.println("Rotating Right");
    steps = -1;
    last_action_time = millis();
  } else {
    steps = 0;
  }
}

// Resets system by returning everything to its original position
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

// Fires the gun by pulling the servo motor back
void fire() {
  int countdownMS = Watchdog.enable(50);  // enables quick watchdog to ensure pulling mechanism works
  for (firePos = 0; firePos <= 180; firePos += 1) {
    myServo.write(firePos);
    delay(15);
    Watchdog.reset();
  }
  Watchdog.disable();
  Watchdog.enable(2000);  // enables longer watchdog to ensure we can't be shooting for more than two seconds
}

// Resets the gun to unfire more by pushing the servo motor forward
void unfire() {
  Watchdog.reset();  // pets the longer watchdog that was enabled upon firing the gun
  Watchdog.disable();
  int countdownMS = Watchdog.enable(50); 
  for (firePos = 180; firePos >= 0; firePos -= 1) {
    myServo.write(firePos);
    delay(15);
    Watchdog.reset();
  }
  Watchdog.disable();
}
