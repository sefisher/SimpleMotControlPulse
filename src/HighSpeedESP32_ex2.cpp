/*
   (c)2023 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/
// CPU freq 240Mhz
// NOTE: set Tool -> Core Debug Level:None   other settings slow down the loop()
// runing Events on Core 1 or 0 does not seem affect stepper latency (no listeners/handlers set??)
// float and long vars are 4byte (32bit) size and so are loaded/stored in one instruction on the ESP32
// so no lock are needed for inter-thread, inter-core transfer of single data values, just declare vars as volatile

#include <HS_AsyncTCP.h>
#include <WiFi.h>
#include "SpeedStepper.h"
#include "millisDelay.h"
#include "secrets.h"

#include "VolatileVars.h" // control and data vars

// set your network settings here
// #define WLAN_SSID       "xxxxxxx"        // cannot be longer than 32 characters!
// #define WLAN_PASS       "xxxxxxx"
static const char staticIP[] = "192.168.1.251";  // set this the static IP you want, e.g. "10.1.1.200" or leave it as "" for DHCP. DHCP is not recommended.
static const int portNo = 4989; // What TCP port to listen on for connections.
static const unsigned long CONNECTION_TIMEOUT_MS = 10000; // 0 => never time out, else close connection if no data received for this time (ms)
// see asyncConnectionTimeout(CONNECTION_TIMEOUT_MS); at the bottom of setup()

// change these pin definitions to match you motor driver
// Not SparkFun ESP32 Redboard can not use pins 6,7 for driving stepper
const int STEP_PIN = 17;
const int DIR_PIN = 18;
const int ENA_PIN = 13;
SpeedStepper stepper(STEP_PIN, DIR_PIN);

millisDelay printDelay;
// this print interval is slow enought that the print( ) statement nevers blocks
unsigned long PRINT_DELAY_MS = 0;// 0=> no prints else 1000 for print once per sec

void setup() {
 
  Serial.begin(115200); 
  for (int i = 9; i > 0; i--) {
    delay(500);
    Serial.print(i);
  }
  Serial.println();
  //setAsyncDebugPtr(&Serial); // enabled debug output for HP_AsyncTCP (slows down loop())
  // Control pins
  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(ENA_PIN, LOW); // Enable the stepper driver
  Serial.print("Stepper Enabled");
  delay(500);

  Serial.print("setup core:"); Serial.println(xPortGetCoreID());
  Serial.print("LWIP core:"); Serial.println(CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU0 ? "0" : "1");

  // Explicitly set the ESP32 to be a WiFi-client, otherwise, it by default,
  // would try to act as both a client and an access-point and could cause
  // network-issues with your other WiFi-devices on your WiFi-network.
  WiFi.mode(WIFI_STA);

  /* Initialise wifi module */
  if (*staticIP != '\0') {
    IPAddress ip;
    ip.fromString(staticIP);
    IPAddress gateway(ip[0], ip[1], ip[2], 1); // set gatway to ... 1
    Serial.print(F("Setting gateway to: "));
    Serial.println(gateway);
    Serial.print(F("Connecting with ip: "));
    Serial.println(ip);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
  }
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");

  if (initAsyncServer(portNo)) {
    Serial.println("HS_AsyncTCP started");
  } else {
    Serial.println("HS_AsyncTCP failed to started");
  }
  // asyncConnectionTimeout(CONNECTION_TIMEOUT_MS); // optional to disconnect is nothing sent/received for 10s

  // initialize stepper
  stepper.setPlusLimit(30000);
  stepper.setMinusLimit(0);
  stepper.setMaxSpeed(5000);
  stepper.setMinSpeed(1);
  stepper.stopAndSetHome();
  stepper.setAcceleration(1000);

  if (PRINT_DELAY_MS) {
    printDelay.start(PRINT_DELAY_MS);
  }
}

unsigned long last_us  = 0; // last us for maxLoopTime
unsigned long lastPrint_us  = 0;
unsigned long lastPrintCount_us  = 0;

void loop() {
  loopCount_v++;
  unsigned long us = micros();
  unsigned long deltaT = us - last_us;
  last_us = us;
  if (deltaT > maxLoopTime_v) {
    maxLoopTime_v = deltaT;
  }
  // handle cmds
  switch(stepperCtrl_v) {
    case STOP:
     stepper.stop();
     break;
    case RUN:
     stepper.setSpeed(5000);
     break;
    case HOME:
     stepper.goHome();
     break;
  }
  stepper.run(); // process stepper
  speed_v = stepper.getSpeed();
  position_v = stepper.getCurrentPosition();

  if (printDelay.justFinished()) {
    printDelay.repeat();
    Serial.print(" millis:"); Serial.print(millis());
    deltaT = us - lastPrint_us;
    lastPrint_us = us;
    float usPerLoop = (float)deltaT / (loopCount_v - lastPrintCount_us);
    lastPrintCount_us = loopCount_v;
    Serial.print(" avg us/loop:"); Serial.print(usPerLoop);
    Serial.print(" Speed:");Serial.print(speed_v);
    Serial.print(" Position:");Serial.print(position_v);
    Serial.println();
  }
  
}
