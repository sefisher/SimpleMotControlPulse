#ifndef VolatileVars_H_
#define VolatileVars_H_
/*
   (c)2023 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/

// this header lists all the volatile vars used to transfer cmds/data between your loop() and WiFiDataHandling
// for code clarity _v is appended to volatile variables
extern volatile unsigned long loopCount_v;
extern volatile unsigned long maxLoopTime_v; // max loop time since last cleared

// variables to pickup stepper position speed
extern volatile float speed_v;
extern volatile uint32_t position_v;

// variable to control stepper
enum StepperControlEnum { STOP, RUN, HOME };
extern volatile StepperControlEnum stepperCtrl_v;
#endif
