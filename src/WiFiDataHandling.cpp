/*
   (c)2023 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/
// WiFiDataHandling.cpp
/**
   This file handles incomming commands and outgoing data
   Settings/data transferred to the loop() via volatile vars listed in volatileVars.cpp / h
*/
#include <Arduino.h>
#include "WiFiDataHandling.h"
#include "VolatileVars.h"
#include "millisDelay.h"
#include "SafeString.h"

static unsigned long lastLoopCount;
static unsigned long last_us;

static millisDelay dataTimer;
static unsigned long DATA_TIMER_MS = 2000; // log data every 2sec
static unsigned long microsStart = 0;

/**
   asyncSetup
   runs on WiFi core 0, in HS_Async thread (task). Use volatiles to interact with your Arduino loop() code
   Called just once when the HS_AsyncTCP task on core 0 starts
   Use this to do any initialization of WiFi task local variables, e.g. start timers etc
*/
void asyncSetup() {
  dataTimer.start(DATA_TIMER_MS);
  microsStart = micros();
  lastLoopCount = 0;
}

// this is called from core 1 asyncTCP task
// approximately every 1.5ms, but can be delayed upto ~20ms if the core 1 is busy with the WiFi
void asyncLoop(Stream &stream) {
  // clean up Home cmd when we get home.
  if ((speed_v == 0.0) && (position_v == 0) && (stepperCtrl_v == HOME)) {
    stepperCtrl_v = STOP; // got home
  }
  if (dataTimer.justFinished()) {
    dataTimer.restart();
    unsigned long loopCount = loopCount_v;
    unsigned long us = micros();
    unsigned long deltaT_us = us - microsStart;
    unsigned long deltaCount = loopCount - lastLoopCount;
    lastLoopCount = loopCount;
    microsStart = us;
    stream.print(millis());
    stream.print(",");
    if (deltaCount != 0) {
      stream.print(((float)deltaT_us) / (deltaCount), 2);
    } else {
      stream.print("inf");
    }
    stream.print(","); stream.print(maxLoopTime_v);
    maxLoopTime_v = 0; // reset for next time
    stream.print(","); stream.print(speed_v);
    stream.print(","); stream.print(position_v);
    stream.println();
  }
}
// msg to send back when connection opened
void asyncConnected(Stream &stream) {
  stream.println("Stepper cmds: s->stops r->runs h->sends home");
  stream.println("Results output every 2sec.");
  stream.println("millis,avg us/loop, max us/loop, speed,position");
}

void asyncDisconnected() {
  // nothing here at the moment
}

// this is called from core 1 by the WiFi support
void asyncDataReceived(Stream &stream) {
  while (stream.available()) {
    char c = stream.read();
    // s for stop, r for run, h for home
    if (c == 's') {
      stepperCtrl_v = STOP;
    } else if (c == 'r') {
      stepperCtrl_v = RUN;
    } else if (c == 'h') {
      stepperCtrl_v = HOME;
    } // ignore other chars
  }
}
