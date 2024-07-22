/*
   (c)2023 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/

#include <Arduino.h>
#include "VolatileVars.h"
// These volatiles communicate between your Arduino loop() and the WiFiDataHandling
// for code clarity _v is appended to volatile variables
volatile unsigned long loopCount_v  = 0; // current loop count
volatile unsigned long maxLoopTime_v  = 0; // max loop time since last cleared

// variables to pickup stepper position speed
volatile float speed_v;
volatile uint32_t position_v;

// variable to control stepper
volatile StepperControlEnum stepperCtrl_v = STOP;
