#include <SPI.h>
#include <Wire.h>
#include "SSD1306Wire.h"

SSD1306Wire  display(0x3c, 5, 4);




#include <neotimer.h>

Neotimer print_timer = Neotimer(100);
Neotimer display_timer = Neotimer(100);
Neotimer factor_timer = Neotimer(400);


#define EA 25
#define EB 26 


int sc = 0;
int16_t encoderValue = 0;
int32_t mPos = 0;

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

// 30 is borderline with 12v
int timertics = 40;



// User parameters to be altered depending on lathe parameters...................................................... 
 // To use this code one must replace these parameters with correct values for your lathe. These are 'spindle_encoder_resolution','lead_screw_pitch' and 'motor_steps'.  
 // For example, if you had a spindle encoder of 200 step/rev... then edit the current 'spindle_encoder_resolution=1024' to read 'spindle_encoder_resolution=200'.
 // The applies to the 'lead_screw_pitch parameter' and the 'motor_steps' parameter. I have used numbers which apply to a Wabeco D6000 lathe. You must edit for your particular lathe. 
 
 int spindle_encoder_resolution=2400 ;   // the number of pulses per revolution of the spindle encoder
 float lead_screw_pitch=2.0;            // the pitch of the lathe lead screw in mm

int microsteps = 32;
int native_steps = 200;  // 1.8 degree == 200, 0.9 degree == 400, 3 phase is 1.2 degree etc etc
int motor_steps= microsteps * native_steps * lead_screw_pitch;                   // the number of steps per revolution of the lead screw

 //float pitch=0.085;                     // the pitch to be cut in millimeters.  It also defines the lathe pitch for turning when first power on. 

float pitch=2.0; // TODO: for testing set back to a decent feed rate
 
 //......................................................................................................................


 int encoder0PinA = EA;                              //the input pin for knob rotary encoder 'I' input
 int encoder0PinB = EB;                              //the input pin for knob rotary encoder 'Q' input
 int buttonPin=2;                                   // the button for the knob rotary encoder push button line


 int menu = 29;                                      // the parameter for the menu select

 int mode_select=0;                                 // a parameter to define the programming versus operation settings
 int tpi;                                           // a paremter to define the number of threads/inch

 float depth;                                       // a parameter to define the thread depth in mm on the compound slide. This is set at 75% of the pitch which seems to work
 float pitch_factor=0.75;                           // a parameter to define how deep to push the oblique cutter for each thread pitch in mm. May differ depending on thread design. This one works
 volatile int32_t input_counter=0;                     //a parameter for the interrupt to count input pulses

volatile int32_t encoder0Pos = 0;
 volatile float factor;                             // the ratio of needs steps/rev for stepper to spindle_encoder_resolution for each thread pitch we pick, this is calculated in the programme 
volatile int32_t delivered_stepper_pulses=0;          //number of steps delivered to the lead screw stepper motor
volatile float calculated_stepper_pulses=0;        //number of steps we should have delivered for a given lead screw pitch

// used to figure out how many steps we need to get to the right position
volatile int32_t delta = 0;

volatile uint8_t _currValueAB = 0;
volatile uint8_t _prevValueAB = 0;
volatile bool z_dir = true; //CW
volatile bool z_moving = false;

int z_step_pin = 13;
int z_dir_pin = 12;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

portMUX_TYPE dmux = portMUX_INITIALIZER_UNLOCKED;



void do_display(){
  display.clear();
  display.drawString(0,0, "e: ");
  encoderValue =  0;
  //display.drawString(10, 0,  String(input_counter));
  display.drawString(10, 0,  String(encoder0Pos));

  display.drawString(0,45,String(isrAt - lastIsrAt));
  display.drawString(10,45,String(factor));


  display.drawString(0,15,String(calculated_stepper_pulses));
  display.drawString(60,15,(String("D:") + String(z_dir)));
  display.drawString(0,30,String(delta));
  display.drawString(40, 30, String(mPos));
  if( mode_select == 0){
    display.drawString(41,1," lathe "); 
    }
  else{
    display.drawString(41,1," prog");
  }
  display.display();
}

void IRAM_ATTR calcDelta(){
  calculated_stepper_pulses = factor*encoder0Pos;
  delta = mPos - calculated_stepper_pulses;
}


void doEncoderA() {
  portENTER_CRITICAL(&mux);
  // look for a low-to-high on channel A
  if (digitalRead(EA) == HIGH) {

    // check channel B to see which way encoder is turning
    if (digitalRead(EB) == LOW) {
      z_dir = true;
      encoder0Pos = encoder0Pos + 1;         // CW
    }
    else {
      z_dir = false;
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }

  else   // must be a high-to-low edge on channel A
  {
    // check channel B to see which way encoder is turning
    if (digitalRead(EB) == HIGH) {
      z_dir = true;
      encoder0Pos = encoder0Pos + 1;          // CW
    }
    else {
      z_dir = false;
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  // use for debugging - remember to comment out
  //calcDelta();
  portEXIT_CRITICAL(&mux);
  //Serial.println (encoder0Pos, DEC);
}

void doEncoderB() {
  portENTER_CRITICAL(&mux);
  // look for a low-to-high on channel B
  if (digitalRead(EB) == HIGH) {

    // check channel A to see which way encoder is turning
    if (digitalRead(EA) == HIGH) {
      encoder0Pos = encoder0Pos + 1;         // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }

  // Look for a high-to-low on channel B

  else {
    // check channel B to see which way encoder is turning
    if (digitalRead(EA) == LOW) {
      encoder0Pos = encoder0Pos + 1;          // CW
    }
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  portEXIT_CRITICAL(&mux);
}

// old count ISR
void IRAM_ATTR count2()    //this is the interrupt routine for the floating point division algorithm
  {
    portENTER_CRITICAL(&mux);
    input_counter++;                                                            // increments a counter for the number of spindle pulses received
    calculated_stepper_pulses=round(factor*input_counter);                      // calculates the required number of stepper pulses which should have occured based on the number spindle pulses (input_counter number)

    // if the calculated number of pulses is greated than the delivered pulses, we deliver one more stepper 
    // pulse only if mode_select is set for lathe (==0)

    if((calculated_stepper_pulses>delivered_stepper_pulses)&&(mode_select==0))  
       {

        /*
        digitalWrite(stepper_pin,HIGH);                              // turns the stepper_pin output pin to HIGH
        delayMicroseconds(10);                                       // keeps that level HIGH for 10 microseconds
        digitalWrite(stepper_pin,LOW);                               // turns the stepper_pin output pin to LOW
        */
        delivered_stepper_pulses++;                                  // increment the number of delivered_stepper_pulses to reflect the pulse just delivered
      }
    portEXIT_CRITICAL(&mux);
  }

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  lastIsrAt = isrAt;
  isrAt = millis();


  // if delta "left" try to get there
  if(z_moving ){
    z_moving = false;
    digitalWrite(z_step_pin, LOW);
  }

  if( delta < 0 && z_dir){
    z_moving = true;
    digitalWrite(z_dir_pin, HIGH);
    digitalWrite(z_step_pin, HIGH);
    mPos++;
  }

  if( delta > 0 && !z_dir){
    z_moving = true;
    digitalWrite(z_dir_pin, LOW);
    digitalWrite(z_step_pin, HIGH); 
    mPos--;
  }

  /*
  if(z_do_move){
    z_do_move = false;
    z_moving = true;
    digitalWrite(z_step_pin, HIGH); 
  }
  */


  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  //calcDelta();

  // TODO:  based on delta try to step in the right direction and inc/dec mPos when the step is completed

  
  
  // TODO:  need a flag to say do_step if mPos != calculated_stepper_pulses
}



 //this defines the parameters for the thread and turning for both metric and imperial threads

void thread_parameters()                                           
  { 

   /*  move button detection somewhere else

   newButtonState = digitalRead(buttonPin);                     // Get the current state of the button
      if (newButtonState == HIGH && oldButtonState == LOW)     // Has the button gone high since we last read it?
         { mode_select=!mode_select;}
                      
         if (mode_select == 0)                                  //mode_select==0 for lathe operation which I call "lathe"   
           {       
         //lcd.setCursor(11,1);
         //lcd.print("lathe");
            }
          else
            {
             mode_select=1; 
          //lcd.setCursor(11,1);                                  // mode_select==1 for parameter selection which I call "prog" for programme
          //lcd.print(" prog");
            }
         oldButtonState = newButtonState;   
  */

            
  if(mode_select==1)
          {
          /* 
                 if ((encoder0PinALast == LOW) && (n == HIGH)) {                 //true if button got pushed?
                       if (digitalRead(encoder0PinB) == LOW) {                   //this is the quadrature routine for the rotary encoder
                       menu++;
                 } else {
                       menu--;
                 }
          */
                 //Serial.println(menu);
                  if(menu>35){                                      //the next four lines allows the rotary select to go around the menu as a loop in either direction
                             menu=35;
                             }
                  if(menu<1){
                             menu=1;
                             }
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

        delivered_stepper_pulses=0;
        input_counter=0;  
         }
 //}


void setFactor(){


         if(menu<4){
           factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution); 
           /*
           switch(menu) {
           case(1):     
           lcd.setCursor(0,0);     
           lcd.print("Turning         ");     
           lcd.setCursor(0,1);     
           lcd.print("Normal     ");     
           break;
           case(2):     
           lcd.setCursor(0,0);     
           lcd.print("Turning         ");     
           lcd.setCursor(0,1);     
           lcd.print("Fine       ");     
           break;
           case(3):     
           lcd.setCursor(0,0);     
           lcd.print("Turning         ");     
           lcd.setCursor(0,1);     
           lcd.print("Coarse     ");     
           break;
           
          }
          */
          }
          else
            {
            if(menu<20)
              {
               depth=pitch_factor*25.4/tpi;                      //the depth of cut in mm on the compound slide I need for each thread pitch.  I use this during operation rather than looking it up each time
               factor= motor_steps*25.4/(tpi*lead_screw_pitch*spindle_encoder_resolution);            //the imperial factor needed to account for details of lead screw pitch, stepper motor #pulses/rev and encoder #pulses/rev
               //lcd.setCursor(0,0);  lcd.print("Imperial ");  lcd.print(tpi);       lcd.print(" tpi ");
               //lcd.setCursor(0,1);  lcd.print("depth="); lcd.print(depth);     lcd.print(" mm"); 
               }
             else
               {
               depth=pitch_factor*pitch;                          //the depth of cut in mm on the compound slide
               factor=pitch*motor_steps/(lead_screw_pitch*spindle_encoder_resolution);         //the metric factor needed to account for details of lead screw pitch, stepper motor #pulses/rev and encoder #pulses/rev
               //lcd.setCursor(0,0);     lcd.print("Metric ");  lcd.print(pitch);     lcd.print(" mm");
               //lcd.setCursor(0,1);     lcd.print("depth=");         lcd.print(depth);     lcd.print(" mm");
               }
             }
                                                     
} 

void setup() {

  Serial.begin(115200);

  //we must initialize rorary encoder 
  pinMode(EA,INPUT_PULLUP);
  pinMode(EB,INPUT_PULLUP);

  pinMode (buttonPin,INPUT_PULLUP);                    //input for the button of the switch rotary encoder
  pinMode(z_dir_pin, OUTPUT);
  pinMode(z_step_pin, OUTPUT);
  //pinMode(stepper_pin, OUTPUT);                        // sets up the stepper_pin as an output for the stepper pulses


  // encoder interrupts
  attachInterrupt(digitalPinToInterrupt(25),doEncoderA,CHANGE);
  attachInterrupt(digitalPinToInterrupt(26),doEncoderB,CHANGE);
   factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution); //initial factor when turning on the ELS to deliver a turning operation of "medium" pitch


  //encoder.attachHalfQuad(EA,EB);

  // setup the display

  display.init();

  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Hello world");
  display.display();


  // setup a timer to handle stepper pulses
  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);

  // wait in us
  timerAlarmWrite(timer, timertics, true);
  timerAlarmEnable(timer);

  Serial.println("setup done");
}

void loop() {
  thread_parameters();
  calcDelta();  
  if(display_timer.repeat()){
    do_display();
  }
  if(factor_timer.repeat()){
    setFactor();
  }
}
