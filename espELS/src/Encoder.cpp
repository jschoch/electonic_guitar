#include "Encoder.h"
#include "Stepper.h"
#include "motion.h"

volatile int16_t encoder0Pos = 0;


portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

portMUX_TYPE dmux = portMUX_INITIALIZER_UNLOCKED;

volatile boolean EA_state = 0;
volatile boolean EB_state = 0;

volatile int32_t spindlePos = 0;

int spindle_encoder_resolution=2400 ;

void IRAM_ATTR updatePos(){
  if(getDir()){
    spindlePos++;
  }else{
    spindlePos--;
  }

  if(encoder0Pos == spindle_encoder_resolution){
    encoder0Pos = 0;
  }
  if(encoder0Pos == -1){
    encoder0Pos = spindle_encoder_resolution -1;
  }

}


void IRAM_ATTR doEncoderA() {
  portENTER_CRITICAL(&mux);
  EA_state = digitalRead(EA);
  EB_state = digitalRead(EB);

  // look for a low-to-high on channel A
  if (EA_state == HIGH) {

    // check channel B to see which way encoder is turning
    if (EB_state == LOW) {
      setDir(true);
      encoder0Pos = encoder0Pos + 1;         // CW
    }
    else {
      setDir(false);
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }

  else   // must be a high-to-low edge on channel A
  {
    // check channel B to see which way encoder is turning
    if (EB_state == HIGH) {
       setDir(true);
      encoder0Pos = encoder0Pos + 1;          // CW
    }
    else {
      setDir(false);
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  // use for debugging - remember to comment out
  //calcDelta();
  updatePos();
  portEXIT_CRITICAL(&mux);
  //Serial.println (encoder0Pos, DEC);
}

void IRAM_ATTR doEncoderB() {
  portENTER_CRITICAL(&mux);
  EA_state = digitalRead(EA);
  EB_state = digitalRead(EB);

  // look for a low-to-high on channel B
  if (EB_state == HIGH) {

    // check channel A to see which way encoder is turning
    if (EA_state == HIGH) {
      encoder0Pos = encoder0Pos + 1;         // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }

  // Look for a high-to-low on channel B

  else {
    // check channel B to see which way encoder is turning
    if (EA_state == LOW) {
      encoder0Pos = encoder0Pos + 1;          // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  updatePos();
  portEXIT_CRITICAL(&mux);
}


void init_encoder(){

  //we must initialize rorary encoder 
  pinMode(EA,INPUT_PULLUP);
  pinMode(EB,INPUT_PULLUP);
  

  encoder0Pos = 0;
  // encoder interrupts
  attachInterrupt(digitalPinToInterrupt(25),doEncoderA,CHANGE);
  attachInterrupt(digitalPinToInterrupt(26),doEncoderB,CHANGE);

  /*
  xTaskCreate(
      calcDelta,           // Task function.
      "calcDelta",        // name of task. 
      1000,                    // Stack size of task 
      NULL,                     // parameter of the task 
      1,                        // priority of the task 
      NULL
      );                    // Task handle to keep track of created task 
  */
}