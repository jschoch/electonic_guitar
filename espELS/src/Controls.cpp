#include "Controls.h"
#include "config.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include "neotimer.h"

Neotimer button_read_timer = Neotimer(30);

int32_t left_limit_max = 2147483646;
volatile int32_t left_limit = 2147483646;
uint8_t btn_mode = FEED;
uint8_t menu = 33; 
volatile bool button_left = false;
bool button_right = false;
bool button_up = false;
bool button_down = false;
bool button_menu = false;


// Debouncer
Bounce debLBP = Bounce(); 
Bounce debRBP = Bounce();
Bounce debSBP = Bounce();
Bounce debUBP = Bounce();
Bounce debDBP = Bounce();

int mode_select = FEED;

void init_controls(){
  pinMode(LBP,INPUT_PULLUP);
  pinMode(RBP,INPUT_PULLUP);
  pinMode(UBP,INPUT_PULLUP);
  pinMode(DBP,INPUT_PULLUP);
  pinMode(SBP,INPUT_PULLUP);

  debLBP.attach(LBP);
  debLBP.interval(20);

  debRBP.attach(RBP);
  debRBP.interval(20);

  debSBP.attach(SBP);
  debSBP.interval(20);

  debUBP.attach(UBP);
  debUBP.interval(20);

  debDBP.attach(DBP);
  debDBP.interval(20);

  
}


void readConfigureButtons(){

  handleSBP();
  if(debLBP.rose()){
    menu++;
  }
  if(debRBP.rose()){
    menu--;
  }

}

void readStartupButtons(){
  handleLBP();
  handleRBP();
  handleSBP();
  handleUBP();
  handleDBP();

  if(button_down){
    mode_select = THREAD;
  }
  if(button_up){
    mode_select = FEED;
  }
  if(button_left){
    display_mode = READY;
    delay(200);
  }

}

void readReadyButtons(){
  handleLBP();
  handleRBP();
  handleSBP();
  handleUBP();
  handleDBP();
  if(button_menu){
    display_mode = STARTUP;
    return;
  }
  if(button_left){
    display_mode = DSTATUS;
    return;
  }
}
void readDstatusButtons(){
  handleLBP();
  handleRBP();
  handleSBP();
  handleUBP();
  handleDBP();

  if(button_left){

  }
  if(button_right){
    
  }
  if(button_menu){
    display_mode = STARTUP;
  }
  if(button_up){
    menu++;
  }
  if(button_down){
    menu--;
  }
  
}

void handleLBP(){
  if(debLBP.read() == LOW){
    button_left = true;
    Serial.print(" LBP ");
  }else{
    button_left = false;
  }
}
void handleRBP(){
  if(debRBP.read() == LOW){
    button_right = true;
    Serial.print(" RBP ");
  }else{
    button_right = false;
  }
}

void handleSBP(){
  if(debSBP.rose()){
    button_menu = true;
    switch (display_mode) {
      case STARTUP:
        //btn_mode = 1;
        break;
      case CONFIGURE:
        //btn_mode = 2;
        break;
      case DSTATUS:
        //btn_mode = 0;
        break;
      default: 
        break;
    }
  }else{
    button_menu = false;
  }


}
void read_buttons(){
  if(button_read_timer.repeat()){
    debLBP.update();
    debRBP.update();
    debSBP.update();
    debUBP.update();
    debDBP.update();
    switch (display_mode){
      case STARTUP:
        // same as 2 for now
        readStartupButtons();
        break;
      case CONFIGURE:
        readConfigureButtons();
        break;
      case DSTATUS: 
        readDstatusButtons();
        break;
      case READY: 
        readReadyButtons();
        break;
    
    }
  }
    
}

void handleUBP(){
  if(debUBP.read() == LOW){
    button_up = true;
    Serial.print(" UBP ");
  }else{
    button_up = false;
  }
}
// this seems to set the left limit

void handleStatusUBP(){
  if(debUBP.rose()){
    if(left_limit != left_limit_max){
      left_limit = left_limit_max;
    }
    else{
      left_limit = toolPos; 
    }
  }
}

void handleDBP(){
  if(debDBP.read() == LOW){
    button_down = true;
    Serial.print(" DBP ");
  }else{
    button_down = false;
  }
}

void setFactor(){


         if(menu<4){
           factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution);            
          }
          else
            {
            if(menu<20)
              {
                // the depth of cut in mm on the compound slide I need for each thread pitch.  
                // I use this during operation rather than looking it up each time

               depth=pitch_factor*25.4/tpi;

                // the imperial factor needed to account for details of lead screw pitch, 
                // stepper motor #pulses/rev and encoder #pulses/rev
               factor= motor_steps*25.4/(tpi*lead_screw_pitch*spindle_encoder_resolution);  
               }
             else
               {
                // the depth of cut in mm on the compound slide
               depth=pitch_factor*pitch; 
                // the metric factor needed to account for details of lead screw pitch, 
                // stepper motor #pulses/rev and encoder #pulses/rev
               factor=pitch*motor_steps/(lead_screw_pitch*spindle_encoder_resolution);
               }
             }
                                                     
} 

//this defines the parameters for the thread and turning for both metric and imperial threads

void thread_parameters()                                           
  { 
  switch(menu) {
    case(1):     pitch=0.085;                  break;  // Normal Turning
    case(2):     pitch=0.050;                  break;  // Fine Turning
    case(3):     pitch=0.160;                  break;  // Coarse Turning
    //...........................................................................................imperial data              
    case(4):     tpi=11;   break;
    case(6):     tpi=12;   break;
    case(7):     tpi=13;   break;
    case(8):     tpi=16;   break;
    case(9):     tpi=18;   break;
    case(10):    tpi=20;   break;
    case(11):    tpi=24;   break;
    case(12):    tpi=28;   break;
    case(13):    tpi=32;   break;
    case(14):    tpi=36;   break;
    case(15):    tpi=40;   break;
    case(16):    tpi=42;   break;
    case(17):    tpi=44;   break;
    case(18):    tpi=48;   break;
    case(19):    tpi=52;   break;
    //.............................................................................................metric data               
    case(20):    pitch=0.4;   break;      
    case(21):    pitch=0.5;   break;      
    case(22):    pitch=0.7;   break;      
    case(23):    pitch=0.75;  break;      
    case(24):    pitch=0.8;   break;      
    case(25):    pitch=1.0;   break;      
    case(26):    pitch=1.25;  break;      
    case(27):    pitch=1.5;   break;      
    case(28):    pitch=1.75;  break;      
    case(29):    pitch=2.0;   break;      
    case(30):    pitch=2.5;   break;      
    case(31):    pitch=3.0;   break;      
    case(32):    pitch=3.5;   break;      
    case(33):    pitch=4.0;   break;      
    case(34):    pitch=5.0;   break;
    case(35):    pitch=7.0;   break;
    }
}
