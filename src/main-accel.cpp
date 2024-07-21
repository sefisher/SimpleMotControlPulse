/*
This code takes position commands in [mm] and provides pulse, dir, and enable outputs for 2 
close loop actuators (assumes the actuator does its own closed loop verification of its poisition
based on the number of pulses sent to it). Commands are accepted via Serial for testing a single actuator.  
The dual control is intended to be streamed from a program like SimTools or FlyPT and takes 2-byte numbers.
TODO - figure out what the SimTool input should be (pulses or mm positions)

-The position commands are limited to +/- MAXLIM an zero is the centered position.
-There are top and bottom limit switches that should be posiitoned at MAXLIM+MAXOFFSET 
and -(MAXLIM+MAXOFFSET) respectively.
-TARGETTOL sets how close to the positon it tries to get (creates a deadband) to avoid hunting.
-DELAYDUR sets how long to pause when going from running to another state (or enabling/disabling the motor)
-REVERSE_PAUSE sets how long to pause when reversing direction to reduce current surges or damage the motor

*/

//#define CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID

#include <Arduino.h>
#include <AccelStepper.h>
#include "UltimateDebounce.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"


//ACTUATOR PINS
const int pulsePin[] = {17,16};
const int directionPin[] = {18,5};
const int enablePin[] = {13,33};
const int topLimitSwitchPin[] = {15,23};
const int bottomLimitSwitchPin[] = {4,22};

//these set geometric limits of actuator and delays for motor mode changes.  distance in 
// "ticks" where for now a tick = 0.2mm. (1.0 mm is 1/5 of a rotation)
#define MAXLIM 500       // limit in ticks to either top (+MAXLIM) or bottom (-MAXLIM) (clamps position count)
#define MAXOFFSET 2      // distance in ticks to move away from limit switch when hit
#define TARGETTOL 2      // tolerance in ticks to ariving at target 
#define MINPULSEDUR 15   // minimum duration in microsec for pulse (f=1/(2*MINPULSEDUR). 20 for T=40, f=25kHz.
#define DELAYDUR 100     // pause after discrete mode changes
#define REVERSE_PAUSE 10 // pause after direction change

// LIMIT SWITCH VARIABLES
// Note: 0 in second position of constructor = active low
UltimateDebounce topls[] = {UltimateDebounce(topLimitSwitchPin[0], 0),UltimateDebounce(topLimitSwitchPin[1], 0)}; 
UltimateDebounce botls[] = {UltimateDebounce(bottomLimitSwitchPin[0], 0),UltimateDebounce(bottomLimitSwitchPin[1], 0)}; 
volatile bool topActive[] = {false,false};
volatile bool bottomActive[] = {false,false};
volatile bool topJustPressed[] = {false,false};
volatile bool bottomJustPressed[] = {false,false};

//TEST VARIABLES--------------
int topPressCount[] = {0,0};
int bottomPressCount[] = {0,0};
int topReleasedCount[] = {0,0};
int bottomReleasedCount[] = {0,0};
//----------------------------

// MOTOR SETUP
const int microsteps = 4;             // Number of microsteps per step
const int stepsPerRevolution = 200;   // Full steps per revolution for your motor
const int pulsePerMillimeter = microsteps*stepsPerRevolution/5;   // Set for a 1605 ballscrew

//ACTUATOR MOTION CONTROL VARIABLES
volatile int position[] = {0,0};                 // the relative position in 0.2 mm increments
volatile int targetPosition[] = {0,0};           // the position being targetted
volatile int currentPulseDuration[] = {MINPULSEDUR,MINPULSEDUR}; // the current pulse duration
volatile bool directionIsUp[] = {false,false};   // direction of travel; true = HIGH = UP, false = LOW = down;
volatile bool running[] = {false,false};         // set true if the motor should be running
volatile unsigned long timeToNextTransition[] = {0,0};    // time that the current pulse state or pause should end
enum ActuatorMode {
  atTarget, // 0
  moving, // 1
  clampingTop, // 2
  clampingBottom, // 3
  seekingTop, // 4
  seekingBottom, // 5
  errorDisable, // 6
};
volatile ActuatorMode mode[2] = {ActuatorMode::atTarget, ActuatorMode::atTarget};
TaskHandle_t MotorRunTask;
TaskHandle_t MainLoopTask;

long lastUpdateTime = 0;

//TEST VARIABLES for loop speed measurement
int countLoopsPerSecond = 0;
//----------------------------------------

// function declarations
void runMotor(int actuatorNumber); //incement motor position
void updatePosition(int change, int actuatorNumber); //adds 'change' to actuator position
void gotoPosition(int newPosition, int actuatorNumber); // sets new target for actuator position
void reverseDir(int actuatorNumber); // function to decel, pause pulses, and reverse direction
void hitTopStop(int actuatorNumber);  // function to stop if top limit switch hit
void hitBottomStop(int actuatorNumber); // function to stop if bottom limit switch hit
void releasedTopStop(int actuatorNumber); // function called when top limit swtich released
void releasedBottomStop(int actuatorNumber); // function called when bottom limit swtich released
void showLimitSwitches(); //prints limit swtich info
void updateLimitSwStatus(); //gets limit switch info, called by 5 mSec timer
void checkSerial(); //reads serial input stream for new positoin
void pulse_uSteps(int uSteps, int actuatorNumber); //runs number of steps on actuator

void MainLoopTaskCode(void* pvParameters); //function runs loop on core 0 - does main loop tasks
void MotorRunTaskCode(void* pvParameters); //function runs loop on core 1 - handles motor movement

//void printCoreLoad(); //load test function

// timer function for limit switch testing - runs every 5 milliseconds, needs 3 reads to debounce = 15 mSec delay
hw_timer_t *Timer0_Cfg = NULL;
void IRAM_ATTR Timer0_ISR() { updateLimitSwStatus(); }

void setup()
{
    for (int i=0; i<2; i++) {
        // control pins
        pinMode(pulsePin[i], OUTPUT);
        pinMode(directionPin[i], OUTPUT);
        pinMode(enablePin[i], OUTPUT);
        //limit switch pins
        pinMode(topLimitSwitchPin[i], INPUT_PULLUP);
        pinMode(bottomLimitSwitchPin[i], INPUT_PULLUP);
        // Set initial direction
        digitalWrite(directionPin[i], LOW); // LOW = down
        directionIsUp[i] = false;           // false = down
        mode[i] = ActuatorMode::atTarget;
    }
 
    // Attach timer to test limit switches every 5000 uSec
    Timer0_Cfg = timerBegin(0, 80, true);
    timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
    timerAlarmWrite(Timer0_Cfg, 5000, true);
    timerAlarmEnable(Timer0_Cfg);

    //create a task loop for motor movement on core1 and main loop functions on core0
    //xTaskCreatePinnedToCore(MotorRunTaskCode, "MotorRunTask", 10000, NULL, 2, &MotorRunTask, 1);
    //TEST - using loop
    //xTaskCreatePinnedToCore(MainLoopTaskCode, "MainLoopTask", 10000, NULL, 2, &MainLoopTask, 0);

    // Initialize serial communication
    Serial.begin(115200);

    // Give some time to the serial monitor to initialize
    delay(1000);

        // Print the frequency of the XTAL crystal
    Serial.print("XTAL Crystal Frequency: ");
    Serial.print(getXtalFrequencyMhz());
    Serial.println(" MHz");

    // Print the CPU frequency
    Serial.print("CPU Frequency: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println(" MHz");

    // Print the APB bus frequency
    Serial.print("APB Bus Frequency: ");
    Serial.print(getApbFrequency());
    Serial.println(" Hz");
}

//Nothing done in the normal loop.  See MainLoopTaskCode.
void loop(){
        // Check for new limit switch activation
        for(int i=0;i<2;i++){
            if (topJustPressed[i]){hitTopStop(i);}
            if (bottomJustPressed[i]){hitBottomStop(i);}
        }
        //------------------------------------

        // Test code for timing
        countLoopsPerSecond++;
        //--------------------

        // get input from serial
        checkSerial();

        // determine if time for displaying data
        unsigned long currentTime = millis();

        unsigned long elapsedTime = currentTime - lastUpdateTime;
        if (elapsedTime >= 2000)
        { // Update info every second
            //printCoreLoad();
            
            noInterrupts();
            lastUpdateTime = currentTime;
            interrupts();
            showLimitSwitches();
            for(int i=0;i<2;i++){
                Serial.print("Act. ");Serial.print(i);
                Serial.print(" pos(mm): ");
                Serial.println(position[i] / 5.0);
            }
            Serial.print(". delT(msec): ");
            Serial.print(elapsedTime);
            Serial.print(". numLoops: ");
            Serial.println(countLoopsPerSecond);
            countLoopsPerSecond = 0;
        } 

        runMotor(0);
        runMotor(1);
}

//Main loop functions
void MainLoopTaskCode(void* pvParameters) {
    Serial.print("MainLoopTask running on core ");
    Serial.println(xPortGetCoreID());
    for (;;) {
        // Check for new limit switch activation
        for(int i=0;i<2;i++){
            if (topJustPressed[i]){hitTopStop(i);}
            if (bottomJustPressed[i]){hitBottomStop(i);}
        }
        //------------------------------------

        // Test code for timing
        countLoopsPerSecond++;
        //--------------------

        // get input from serial
        checkSerial();

        // determine if time for displaying data
        unsigned long currentTime = millis();

        unsigned long elapsedTime = currentTime - lastUpdateTime;
        if (elapsedTime >= 1000)
        { // Update info every second
            noInterrupts();
            lastUpdateTime = currentTime;
            interrupts();
            showLimitSwitches();
            for(int i=0;i<2;i++){
                Serial.print("Act. ");Serial.print(i);
                Serial.print(" pos(mm): ");
                Serial.print(position[i] / 5.0);
            }
            Serial.print(". delT(msec): ");
            Serial.print(elapsedTime);
            Serial.print(". numLoops: ");
            Serial.println(countLoopsPerSecond);
            countLoopsPerSecond = 0;
        }    
        delayMicroseconds(500);
    }
}

void MotorRunTaskCode(void* pvParameters) {
    Serial.print("MotorRunTask running on core ");
    Serial.println(xPortGetCoreID());
    for (;;) {
        runMotor(0);
        runMotor(1);
        delayMicroseconds(5);  
    }
}

// Function to check the limit switches
void showLimitSwitches()
{
    Serial.println("---------------------------");
    for(int i=0;i<2;i++){
        Serial.print("Actuator Number ");Serial.print(i);Serial.println(":");
        if (topls[i].is_down())
        {
            Serial.println("Top limit switch ACTIVE.");
        }
        if (botls[i].is_down())
        {
            Serial.println("Bottom limit switch ACTIVE.");
        }
        Serial.print("TP: ");
        Serial.print(topPressCount[i]);
        Serial.print(" TR: ");
        Serial.print(topReleasedCount[i]);
        Serial.print(" BP: ");
        Serial.print(bottomPressCount[i]);
        Serial.print(" BR: ");
        Serial.println(bottomReleasedCount[i]);
    }
    Serial.println("---------------------------");
}

/*
Function to update limit switches.  Call often.
Limit work done by this function since called as interrupt.
*/
void updateLimitSwStatus()
{
    for(int i=0;i<2;i++){
        topls[i].update();
        botls[i].update();
        if (topls[i].is_pressed())
        {
            topPressCount[i]++;
            topActive[i] = true;
            topJustPressed[i] = true;
        }
        else if (topls[i].is_released())
        {
            topReleasedCount[i]++;
            topActive[i] = false;
        }
        if (botls[i].is_pressed())
        {
            bottomPressCount[i]++;
            bottomActive[i] = true;
            bottomJustPressed[i] = true;
        }
        else if (botls[i].is_released())
        {
            bottomReleasedCount[i]++;
            bottomActive[i] = false;
        }
    }
}

// Return true if top limit switch is active
bool readTopLimitSw(int actuatorNumber)
{
    return digitalRead(topLimitSwitchPin[actuatorNumber]) == LOW; // Active LOW,
}

// Return true if bottom limit switch is active
bool readBottomLimitSw(int actuatorNumber)
{
    return digitalRead(bottomLimitSwitchPin[actuatorNumber]) == LOW; // Active LOW
}

// move motor the number of microsteps provided (200*4=800 is one rotation)
void pulse_uSteps(int uSteps, int actuatorNumber)
{
    for (int i = 0; i < (int)(uSteps); i++)
    { // Total microsteps
        digitalWrite(pulsePin[actuatorNumber], HIGH);
        delayMicroseconds(currentPulseDuration[actuatorNumber]); 
        digitalWrite(pulsePin[actuatorNumber], LOW);
        delayMicroseconds(currentPulseDuration[actuatorNumber]);
    }
}

// function to run toward a target position
void runMotor(int actuatorNumber)
{
        /*
     TODO:
     Outline for running mode:

           - if only one limit switch is closed verify that the direction_pin is moving away from limit and pulse once and increment/decrement position
           - elif check if target and current position are not within tolerance (if at target w/in tol then set mode to stop and exit)
           - elif determine if direction reversed based on target vs position
           - elif [maybe: update pulse rate (P setting)] and pulse and increment/decrement position
    */
    switch(mode[actuatorNumber]){
        case ActuatorMode::moving:
            if (abs(position[actuatorNumber] - targetPosition[actuatorNumber]) > TARGETTOL)
            {
                Serial.print(">");
                // TODO - test with 0.2 mm (move 0.2 mm at a time = 32 pulses)
                pulse_uSteps(32,actuatorNumber);

                if (directionIsUp[actuatorNumber])
                {
                    updatePosition(1,actuatorNumber);
                }
                else
                {
                    updatePosition(-1,actuatorNumber);
                }
            }else{
                mode[actuatorNumber] = ActuatorMode::atTarget;
            }
        break;
        case ActuatorMode::clampingBottom:
            //verify direction
            //move
        break;
        case ActuatorMode::clampingTop:
            //verify direction
            //move
        break;
        case ActuatorMode::seekingBottom:
            //verify direction
            //move slowly
        break;
    }

}

// update the position following movement
// position is in [0.2 mm] running from -MAXLIM to +MAXLIM (notionally -500 to 500 (200 mm range) with 0 parked)
void updatePosition(int change, int actuatorNumber)
{
    position[actuatorNumber] = position[actuatorNumber] + change;
}

// goto new target position
void gotoPosition(int newPosition, int actuatorNumber)
{
    /*
    TODO:
    - verify a limit switch isn't closed
    --- if it is don't update target unless it is further away from limit than curent position; then exit.
    --- if it is not then do what is below
    */

    if (newPosition > MAXLIM)
    {
        targetPosition[actuatorNumber] = MAXLIM;
    }
    else if (newPosition < -MAXLIM)
    {
        targetPosition[actuatorNumber] = -MAXLIM;
    }
    else
    {
        targetPosition[actuatorNumber] = newPosition;
    }
    Serial.print("Target: ");
    Serial.println(targetPosition[actuatorNumber]);
    if (((targetPosition[actuatorNumber] > position[actuatorNumber]) && !directionIsUp[actuatorNumber]) || ((targetPosition[actuatorNumber] < position[actuatorNumber]) && directionIsUp[actuatorNumber]))
    {
        reverseDir(actuatorNumber);
    }
}

// function to decel, pause pulses, and reverse direction
void reverseDir(int actuatorNumber)
{
    Serial.print("Reversing Actuator #");Serial.print(actuatorNumber);Serial.print(". Shifting to ");
    if (directionIsUp[actuatorNumber])
    {
        Serial.println("down.");
    }
    else
    {
        Serial.println("up.");
    }
   
    //ensure delay in pulse while changing dir pin state
    timeToNextTransition[actuatorNumber] = micros() + REVERSE_PAUSE; 

    directionIsUp[actuatorNumber] = !directionIsUp[actuatorNumber];
    if (directionIsUp[actuatorNumber])
    {
        digitalWrite(directionPin[actuatorNumber], HIGH);
    }
    else
    {
        digitalWrite(directionPin[actuatorNumber], LOW);
    }

    //slow the pulse rate
    currentPulseDuration[actuatorNumber] = MINPULSEDUR * 2;
    
    // if (directionIsUp[actuatorNumber])
    // {
    //     Serial.println("Now going up.");
    // }
    // else
    // {
    //     Serial.println("Now going down.");
    // }
}

// function to stop if top limit switch hit
void hitTopStop(int actuatorNumber)
{
    topJustPressed[actuatorNumber] = false;
    Serial.print("\n>>Actuator ");Serial.print(actuatorNumber);
    Serial.println(" Top Limit Switch Hit.<<");
    mode[actuatorNumber] = ActuatorMode::clampingTop;
    if(directionIsUp[actuatorNumber]){
        reverseDir(actuatorNumber);
    }
}

//function when backed off the top limit
void releasedTopStop(int actuatorNumber){
    mode[actuatorNumber] = ActuatorMode::atTarget;
    targetPosition[actuatorNumber] = MAXLIM;
    position[actuatorNumber] = MAXLIM;
}

// function to stop if top limit switch hit
void hitBottomStop(int actuatorNumber)
{
    bottomJustPressed[actuatorNumber] = false;
    Serial.print("\n>>Actuator ");Serial.print(actuatorNumber);
    Serial.println(" Bottom Limit Switch Hit.<<");
    mode[actuatorNumber] = ActuatorMode::clampingBottom;
    if(!directionIsUp[actuatorNumber]){
        reverseDir(actuatorNumber);
    }
}

//function when backed off the bottom limit
void releasedBottomStop(int actuatorNumber){
    mode[actuatorNumber] = ActuatorMode::atTarget;
    targetPosition[actuatorNumber] = -MAXLIM;
    position[actuatorNumber] = -MAXLIM;
}

// This function is for testing.  Takes a number or commands.
void checkSerial()
{
    if (Serial.available() > 0)
    {
        // Read the incoming byte
        String input = Serial.readStringUntil('\n');
        //TODO - make this work for 2 actuators - for testing hard coding for actuatorNumber 0
        gotoPosition(input.toInt(),0);
    }
}

// Function to check for serial input for SimTool position streams
// SimTools should be set to stream 2 16-bit numbers with no separators
// each number corresponds to a position of one actuator running from
// -500 to +500 in 0.2 mm steps (200 mm range)
// 200mm/5mm pitch = 40 rotations.
// 800 steps per rotation = 32000 pulses over range.
void checkSerialSimTools(void *parameter)
{
    // Check if there are at least 4 bytes available to read
    if (Serial.available() >= 4)
    {
        // Read the first 2-byte number
        uint16_t num1 = Serial.read() | (Serial.read() << 8);

        // Read the second 2-byte number
        uint16_t num2 = Serial.read() | (Serial.read() << 8);

        // Print the received numbers
        Serial.print("Received numbers: ");
        Serial.print(num1);
        Serial.print(", ");
        Serial.println(num2);
    }
}


// void printCoreLoad() {
//   UBaseType_t numTasks = uxTaskGetNumberOfTasks();
//   TaskStatus_t *taskStatusArray = (TaskStatus_t *)pvPortMalloc(numTasks * sizeof(TaskStatus_t));
//   UBaseType_t tasksWritten = uxTaskGetSystemState(taskStatusArray, numTasks, NULL);

//   uint32_t totalRunTime = 0;
//   for (UBaseType_t i = 0; i < tasksWritten; i++) {
//     totalRunTime += taskStatusArray[i].ulRunTimeCounter;
//   }

//   Serial.printf("Task count: %u\n", tasksWritten);

//   for (UBaseType_t i = 0; i < tasksWritten; i++) {
//     float load = ((float)taskStatusArray[i].ulRunTimeCounter / totalRunTime) * 100;
//     Serial.printf("Task: %s, Core: %u, Run Time: %u, Load: %.2f%%\n",
//                   taskStatusArray[i].pcTaskName,
//                   taskStatusArray[i].xCoreID,
//                   taskStatusArray[i].ulRunTimeCounter,
//                   load);
//   }

//   vPortFree(taskStatusArray);
// }
