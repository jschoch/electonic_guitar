#include "Stepper.h"

// stepper timer stuff

volatile int interruptCounter;
int totalInterruptCounter;
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;
volatile uint32_t isrAt = 0;
hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint8_t current_accel = 20;
volatile bool synced = false;

// 30 is borderline stalling without acceleration and with 12v
int timertics = 40;

// used to figure out how many steps we need to get to the right position
volatile int32_t delta = 0;

volatile uint8_t _currValueAB = 0;
volatile uint8_t _prevValueAB = 0;
volatile bool z_dir = true; //CW
volatile bool z_moving = false;

int z_step_pin = 13;
int z_dir_pin = 12;

bool getDir(){
  return z_dir;
}
void setDir(bool d){
  z_dir = d;
}

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  lastIsrAt = isrAt;
  isrAt = millis();


  // ensure driving button is engaged, and synced
  if(button_left && (encoder0Pos == 0 || synced)){
    synced = true;
    }


  // ensure left limit is left of current position

  if(toolPos > left_limit && z_dir){
    synced = false;
    digitalWrite(z_step_pin, LOW);
    z_moving = false;
    
  }


  if(button_left && synced){
    // if delta "left" try to get there
    if(z_moving ){
      z_moving = false;
      digitalWrite(z_step_pin, LOW);
    }
  
    if( delta < 0 && z_dir){
      z_moving = true;
      digitalWrite(z_dir_pin, HIGH);
      digitalWrite(z_step_pin, HIGH);
      toolPos++;
    }
  
    if( delta > 0 && !z_dir){
      z_moving = true;
      digitalWrite(z_dir_pin, LOW);
      digitalWrite(z_step_pin, HIGH); 
      toolPos--;
    }
  }

  // if not driving ensure synced flag goes off
  if(!button_left && synced){
    synced = false;
  }

  
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void init_stepper(){

  pinMode(z_dir_pin, OUTPUT);
  pinMode(z_step_pin, OUTPUT);


  // setup a timer to handle stepper pulses
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);

  // wait in us
  timerAlarmWrite(timer, timertics, true);
  timerAlarmEnable(timer);

}