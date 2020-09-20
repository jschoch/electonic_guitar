#include <Arduino.h>
#include "config.h"
#include "util.h"
#include "display.h"

#include <Wire.h>
#include "SSD1306Wire.h"
#include "neotimer.h"

SSD1306Wire  display(0x3c, 5, 4);
Neotimer display_timer = Neotimer(100);
Neotimer print_timer = Neotimer(1000);

int display_mode = STARTUP;

void clear(){
  display.clear();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
}
// TODO: figure out best way to deal with different displays or using Serial.

void init_display(){
#ifdef USESSD1306
  // setup the display
  
  display.init();
  delay(200);
  clear();

  display.drawString(0, 0, "Hello world");
  display.display();
#endif

}

static String sp = String(" ");




// this is the feed mode
void do_startup_display(){
 
  //display.drawString(0,0,String(FEED_MODE[mode_select]));
  //display.drawString(0,0,"M: "+ String(mode_select));
  //display.drawString(0,11,"P:" + String(pitch));
  //display.display();
}

// this is the config mode
void do_configure_display(){
  clear();
  display.drawString(0,0, "config: ");
  display.display();
}


void do_status_display(){
  
  display.drawString(0,0, "enc Pos: ");
  
  display.drawString(40, 0,  String(encoder0Pos));

  //display.drawString(0,45,String(isrAt - lastIsrAt));
  display.drawString(10,45,String(factor));

  display.drawString(0,15,"pend");
  display.drawString(40,15,String(calculated_stepper_pulses));

  // synced variable fuckied ambigious compiler error and you are too fucking stupid to know how scoping works in this shitbox.
  /*
  display.drawString(60,15,(String("D:") + 
      String(getDir()) + sp + 
      String(button_left)+ sp + 
      String(synced))
      );
  */
  display.drawString(0,30,String(delta));
  display.drawString(40, 30, String(toolPos));
  if( mode_select == 0){
    //display.drawString(41,1," lathe "); 
    }
  else{
    //display.drawString(41,1," prog");
  }
  display.display();
}

void do_display(){

  if(display_timer.repeat()){

    /*
    clear();
    switch (display_mode) {
      case STARTUP:
        do_startup_display();
        break;
      case CONFIGURE:
        do_configure_display();
        break; 
      case DSTATUS: 
        do_status_display();
        break;
    }
    */
  }
  
  if (print_timer.repeat()){
    Serial.print("display mode ");
    Serial.print(DISPLAY_MODE[display_mode]);
    Serial.print(" ");
    Serial.print(String(FEED_MODE[mode_select]));
    Serial.println();
  }
}