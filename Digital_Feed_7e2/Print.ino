//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ***** Print ***** /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <neotimer.h>
extern Adafruit_SSD1306 display;

Neotimer pt = Neotimer(100);

void display_mode(){
  display.clearDisplay();
  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("M:");
  display.print(ModeS[Mode]);
  display.print(" ");
}

void print_thread(){
  display.print(Enc_Pos);
  if      (Sub_Mode_Thread == Sub_Mode_Thread_Int) {
    //display.setCursor(0,20);
    display.print(" Int ");
    //snprintf(LCD_Row_2, 17, "Int  Max:%s", Thread_Info[Thread_Step].Limit_Print);
  }
  else if (Sub_Mode_Thread == Sub_Mode_Thread_Man) {
    //display.setCursor(8,0);
    display.print(Thread_Info[Thread_Step].Thread_Print);
    display.print(" ");
    display.print(Thread_Info[Thread_Step].Limit_Print);
    //snprintf(LCD_Row_2, 17, "Man  Max:%s", Thread_Info[Thread_Step].Limit_Print);
  }
  else if (Sub_Mode_Thread == Sub_Mode_Thread_Ext) {
    //display.setCursor(0,20);
    display.print(" Ext ");
    //snprintf(LCD_Row_2, 17, "Ext  Max:%s", Thread_Info[Thread_Step].Limit_Print);
  }
  display_limits(); 
  display_motors();
};

void display_limits(){
  if(Limit_Pos_Left != Limit_Pos_Max ){
    display.print(" LL ");
  }
  if(Limit_Pos_Right != Limit_Pos_Min ){
    display.print(" LR ");
  } 
  
}

void display_motors(){
  display.print(" MPZ:");
  display.print(Motor_Z_Pos);
}

void ser_print(){
  Serial.print("M");
  Serial.print(Mode);
  Serial.println(" ");
        Serial.print("Thrd  ");
        Serial.print(Thread_Info[Thread_Step].Thread_Print);
  
        Serial.print(" Man Max:   ");
        Serial.println(Thread_Info[Thread_Step].Limit_Print);
        Serial.print("LL:");
        Serial.print(Limit_Pos_Left);
        Serial.print(",LR:");
        Serial.print(Limit_Pos_Right);
        Serial.print(" ");
        Serial.print(Motor_Z_Pos);
        Serial.print(",");
        Serial.print(Enc_Pos);
        Serial.print("\n");
 
}

void Print()
{
  display_mode();
   if (Mode == Mode_Thread)  //////////////////////////////////////////////////////////
   {
      //snprintf(LCD_Row_1, 17, "Thrd      %s", Thread_Info[Thread_Step].Thread_Print);
      print_thread(); 
   } 

  
   else if (Mode == Mode_Feed)  //////////////////////////////////////////////////////////
   {
      //snprintf(LCD_Row_1, 17, "Feed mm/rev %1d.%02dmm", Feed_mm/100, Feed_mm%100);
      
      /*
      if      (Sub_Mode_Feed == Sub_Mode_Feed_Int) snprintf(LCD_Row_2, 17, "Int  Pq:%1d Ap:%1d.%02d", Pass_Total-Pass_Nr+1, Ap/100, Ap%100);
      else if (Sub_Mode_Feed == Sub_Mode_Feed_Man) snprintf(LCD_Row_2, 17, "Man  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_Feed == Sub_Mode_Feed_Ext) snprintf(LCD_Row_2, 17, "Ext  Pq:%1d Ap:%1d.%02d", Pass_Total-Pass_Nr+1, Ap/100, Ap%100);
      */
   }


   else if (Mode == Mode_aFeed)  //////////////////////////////////////////////////////////
   {
      //snprintf(LCD_Row_1, 17, "Feed mm/min  %3d", aFeed_mm);
      
      /* 
      if      (Sub_Mode_aFeed == Sub_Mode_aFeed_Int) snprintf(LCD_Row_2, 17, "Int  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_aFeed == Sub_Mode_aFeed_Man) snprintf(LCD_Row_2, 17, "Man  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_aFeed == Sub_Mode_aFeed_Ext) snprintf(LCD_Row_2, 17, "Ext  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      */
   }
 
   
   else if (Mode == Mode_Cone_L)  //////////////////////////////////////////////////////////////
   {
      //snprintf(LCD_Row_1, 17, "Cone < %s %1d.%02dmm", Cone_Info[Cone_Step].Cone_Print, Feed_mm/100, Feed_mm%100);

      /*
      if      (Sub_Mode_Cone == Sub_Mode_Cone_Int) snprintf(LCD_Row_2, 17, "Int  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_Cone == Sub_Mode_Cone_Man) snprintf(LCD_Row_2, 17, "Man  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_Cone == Sub_Mode_Cone_Ext) snprintf(LCD_Row_2, 17, "Ext  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      */
   }

   
   else if (Mode == Mode_Cone_R)  //////////////////////////////////////////////////////////////
   {
      //snprintf(LCD_Row_1, 17, "Cone > %s %1d.%02dmm", Cone_Info[Cone_Step].Cone_Print, Feed_mm/100, Feed_mm%100);
      
      /*
      if      (Sub_Mode_Cone == Sub_Mode_Cone_Int) snprintf(LCD_Row_2, 17, "Int  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_Cone == Sub_Mode_Cone_Man) snprintf(LCD_Row_2, 17, "Man  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      else if (Sub_Mode_Cone == Sub_Mode_Cone_Ext) snprintf(LCD_Row_2, 17, "Ext  Pq:%1d Ap:%1d.%02d", Pass_Total, Ap/100, Ap%100);
      */
   }


   else if (Mode == Mode_Reserve)  //////////////////////////////////////////////////////////////
   {
      //snprintf(LCD_Row_1, 17, "Reserve         ");
      //snprintf(LCD_Row_2, 17, "Reserve         ");
   }
   

   else if (Mode == Mode_Sphere)  //////////////////////////////////////////////////////////////
   {
      if (!key_sel_flag)
      {
         //snprintf(LCD_Row_1, 17, "Sphr %2ld.%01ldmm %1d.%02dmm", Sph_R_mm * 2 / 100, Sph_R_mm * 2 / 10 %10, Feed_mm/100, Feed_mm%100);

         /*
         if      (Sub_Mode_Sphere == Sub_Mode_Sphere_Int) snprintf(LCD_Row_2, 17, "Mode not Exist  ");
         else if (Sub_Mode_Sphere == Sub_Mode_Sphere_Man) snprintf(LCD_Row_2, 17, "     BarDia %2ld.%01ld", Bar_R_mm*2/100, Bar_R_mm*2%100);
         else if (Sub_Mode_Sphere == Sub_Mode_Sphere_Ext) snprintf(LCD_Row_2, 17, "Ext  BarDia %2ld.%01ld", Bar_R_mm*2/100, Bar_R_mm*2%100);
         */
      }
      
      else
      {
         //snprintf(LCD_Row_1, 17, "Cut.Width %1d.%02dmm", Cutter_Width/100, Cutter_Width%100);
         //snprintf(LCD_Row_2, 17, "Cut.StepZ %1d.%02dmm", Cutting_Width/100, Cutting_Width%100);
      }
   }

    
   else if (Mode == Mode_Divider)  /////////////////////////////////////////////////////////////
   { 
      long Spindle_Angle = Enc_Pos * 36000 / ENC_TICK;
      long Required_Angle = 36000 * (Current_Tooth - 1) / Total_Tooth;
      //snprintf(LCD_Row_1, 17, "Req:%3ld.%02ld z:%3d", Required_Angle/100, Required_Angle%100, Total_Tooth);
      //snprintf(LCD_Row_2, 17, "Rea:%3ld.%02ld a:%3d", Spindle_Angle/100, Spindle_Angle%100, Current_Tooth);
   }

   // Error
   if      (err_1_flag == true) {
    //snprintf(LCD_Row_2, 17, "Limits not Set  ");
    //Serial.println("limit not set error");
    display.print("limit not set error");
    delay(500);
   }
   else if (err_2_flag == true) {
    //snprintf(LCD_Row_2, 17, "Move to Init Pos");   
    //Serial.println("move to init pos");
    display.print("move to init pos");
    delay(500);
   }
      
   display.display();
}
