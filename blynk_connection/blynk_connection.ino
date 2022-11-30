// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
#define BLYNK_TEMPLATE_ID "TMPLiGjOag8o"
#define BLYNK_DEVICE_NAME "Water Gun Trigger"
#define BLYNK_AUTH_TOKEN  "5dY6oh0BlhF6Q3s_A-WFy40wHrRq7RNO"
#define BLYNK_PRINT Serial

#include <SPI.h>
#include <WiFi101.h>
#include <BlynkSimpleWiFiShield101.h>
#include <Adafruit_SleepyDog.h>  // https://github.com/adafruit/Adafruit_SleepyDog/blob/master/examples/BasicUsage/BasicUsage.ino

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Brown-Guest";
char pass[] = "";

BlynkTimer timer;

int value = -1;
int prevval = -1;  // for tracking a "rising edge" of virtual button push

// This function is called every time the Virtual Pin 0 state changes; VP0 connected to our shoot button!
BLYNK_WRITE(V0)
{
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

  int countdownMS = Watchdog.enable(1000);  // enable watchdog with specified number of milliseconds before reset
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
}

// returns if the button was pressed or not by detecting rising edge
bool detect_rise()
{
  if (prevval == 0 && value == 1){
    prevval = -1;  // since polling time of loop is larger than WiFi polling time, we don't want this to return True a bunch of times
    return 1;
  }
  else{
    return 0;
  }
}



