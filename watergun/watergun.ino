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

  int countdownMS = Watchdog.enable(30000);  // enable watchdog with specified number of milliseconds before reset

  pinMode(LEFT_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(LEFT_BTN_PIN), rotateISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_BTN_PIN), rotateISR, CHANGE);
  myStepper.setSpeed(15);
  myServo.attach(6);
}

void loop()
{
  // Blynk recommends avoiding the delay() function here
  // TODO: connect this code with the rest of our implementation!
  Blynk.run();
  timer.run();
  if (detect_rise()){
    Serial.println("Button pressed!");
  }

  Watchdog.reset();  // reset the watchdog

  if (pos <= LOWER_BOUND && steps < 0) return;
  if (pos >= UPPER_BOUND && steps > 0) return;
  
  myStepper.step(steps);
  pos += steps;
}

// returns if the button was pressed or not by detecting rising edge
bool detect_rise()
{
  if (prevval == 0 && value == 1){
    fire();
    Serial.println("fire");
    prevval = -1;  // since polling time of loop is larger than WiFi polling time, we don't want this to return True a bunch of times
    return 1;
  } else if (prevval == 1 && value == 0) {
    unfire();
Serial.println("unfire");
    prevval = -1;
    return 0;
  } else{
//    Serial.println("other");
    return 0;
  }
}

void rotateISR() {
  if (digitalRead(LEFT_BTN_PIN) == HIGH) {
    Serial.println("rotate left");
    steps = 1;
  } else if (digitalRead(RIGHT_BTN_PIN) == HIGH) {
    Serial.println("rotate right");
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
