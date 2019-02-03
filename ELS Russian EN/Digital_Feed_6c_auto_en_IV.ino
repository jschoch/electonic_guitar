#include <avr/pgmspace.h>


//////////////////////////////////
// ***** PARAMETERS OF IRON ***** //

#define ENC_LINE_PER_REV     1800     // Number of lines of the encoder per 1 revolution of the spindle 
#define MOTOR_Z_STEP_PER_REV 200      // Number of steps / rev, Z-axis, longitudinal 
#define SCREW_Z              1.50     // Step of the longitudinal screw Z 
#define McSTEP_Z             4        // Microstep, Z-axis, longitudinal
#define MOTOR_X_STEP_PER_REV 200      // Number of steps / rev, X-axis, transverse
#define SCREW_X              1.00     // Step of transverse screw X
#define REBOUND_X            400      // Bouncing the cutter in microsteps, for auto-threading, there should be more backlash
#define McSTEP_X             4        // Microstep, X axis, transverse
#define ACCEL                15       // From which we will accelerate on Threads, Accel + Ks should be <255

// Accelerated Displacements                           
#define MAX_RAPID_MOTION     26                       // Less is a larger final speed          //16000000/32/((26+1)*2)/800*60=694rpm
#define MIN_RAPID_MOTION     (MAX_RAPID_MOTION + 125) // More - lower initial speed, max 255  //16000000/32/((125+26+1)*2)/800*60=123rpm
#define REPEAt               (McSTEP_Z * 2)           // Number of repetitions for a constant speed within a full step 
                                                      // Acceleration time = 125/2 * REPEAT (8) / Microstep (4) = 125 full steps acceleration cycle
//////////////////////////////////


// ***** MY CONSTANT *****
#define CW               0
#define CCW              1
#define ON               1
#define OFF              0


// ***** LCD *****
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
char LCD_Row_1[17];
char LCD_Row_2[17];


// ***** Buzzer *****
#define Beeper_Init()                DDRE |= (1<<4)           // Pin2, Output PE4
#define Beeper_On()                  PORTE &= ~(1<<4)         // Pin2 0
#define Beeper_Off()                 PORTE |= (1<<4)          // Pin2 1


// ***** Stepper Motor *****
#define MotorPort                    PORTL                    //
#define Motor_Init()                 DDRL=B1111101           // Is configured as an output, PL2 input T5

#define Motor_Z_SetPulse()           MotorPort &= ~(1<<0)     // Pin49 0
#define Motor_Z_RemovePulse()        MotorPort |= (1<<0)      // Pin49 1
#define Motor_Z_InvertPulse()        MotorPort ^= (1<<0)      // Pin49
#define Read_Z_State                 (PINL & (1<<0))

#define Motor_X_SetPulse()           MotorPort &= ~(1<<1)     // Pin48 0
#define Motor_X_RemovePulse()        MotorPort |= (1<<1)      // Pin48 1
#define Motor_X_InvertPulse()        MotorPort ^= (1<<1)      // Pin48
#define Read_X_State                 (PINL & (1<<1))

#define Motor_Z_CW()                 MotorPort &= ~(1<<7)     // Pin42 0
#define Motor_Z_CCW()                MotorPort |= (1<<7)      // Pin42 1

#define Motor_X_CW()                 MotorPort &= ~(1<<3)     // Pin46 0
#define Motor_X_CCW()                MotorPort |= (1<<3)      // Pin46 1

#define Motor_Z_Enable()             MotorPort |= (1<<4)      // Pin45 1
#define Motor_Z_Disable()            MotorPort &= ~(1<<4)     // Pin45 0

#define Motor_X_Enable()             MotorPort |= (1<<5)      // Pin44 1
#define Motor_X_Disable()            MotorPort &= ~(1<<5)     // Pin44 0


// ***** Tacho *****
#define TachoPort                    PORTL
#define TachoSetPulse()              TachoPort |= (1<<6)      // Pin43 1
#define TachoRemovePulse()           TachoPort &= ~(1<<6)     // Pin43 0


// ***** Encoder *****
#define ENC_TICK                     (ENC_LINE_PER_REV * 2)   // Working number of pulses

///////// Error /////
//#define EncoderPort                  PORTD                    // Port "D", configured as an input *** I think this was commented out in the original code.

#define EncoderPort                  PORTD                    // Port "D", configured as an input ** set properly **
#define Encoder_Init()               DDRD=B00000000;\
                                     PORTD = B00001111        // Lifting PIN_21, 20, 19, 18 // external pull-up to +5 is desirable through 1K resistors
#define Enc_Read                     (PIND & B00000010)


//***** Accelerated movement *****
#define Timer2_Init()                TCCR2A = (1<<WGM21);\
                                     TCCR2B = (1<<CS20)|(1<<CS21); // 16MHz/32 = 500kHz


//***** Limit Buttons & LEDs *****
#define LimitPort                    PORTC                    // PIN_31, 33, 35, 37 - inputs, PIN_30, 32, 34, 36 - outputs
#define LimitInit()                  DDRC=B10101010;\
                                     PORTC = B11111111        // PORT_C pullup // external pullup to +5 is desired through 1K resistors PIN_31, 33, 35, 37

#define Limit_Left_LED_On()          LimitPort &= ~(1<<7)     // PC7, Pin30 0
#define Limit_Left_LED_Off()         LimitPort |= (1<<7)      // PC7, Pin30 1
#define Limit_Right_LED_On()         LimitPort &= ~(1<<5)     // PC5, Pin32 0
#define Limit_Right_LED_Off()        LimitPort |= (1<<5)      // PC5, Pin32 1
#define Limit_Front_LED_On()         LimitPort &= ~(1<<3)     // PC3, Pin34 0
#define Limit_Front_LED_Off()        LimitPort |= (1<<3)      // PC3, Pin34 1
#define Limit_Rear_LED_On()          LimitPort &= ~(1<<1)     // PC1, Pin36 0
#define Limit_Rear_LED_Off()         LimitPort |= (1<<1)      // PC1, Pin36 1

#define Limit_Left_Button            (PINC & B01000000)       // PC6, Pin31
#define Limit_Right_Button           (PINC & B00010000)       // PC4, Pin33
#define Limit_Front_Button           (PINC & B00000100)       // PC2, Pin35
#define Limit_Rear_Button            (PINC & B00000001)       // PC0, Pin37

#define Limit_Pos_Max                1073741824
#define Limit_Pos_Min               -1073741824


//***** Key & Joy *****
enum Pressed_Key
{
  Key_None,
  Key_Right,
  Key_Up,
  Key_Down,
  Key_Left,
  Key_Select
};

//#define JoyPort                     PORTK
#define Joy                         PINK
#define Joy_Left                    (PINK & B00000001)        // PK0
#define Joy_Right                   (PINK & B00000010)        // PK1
#define Joy_Front                   (PINK & B00000100)        // PK2
#define Joy_Rear                    (PINK & B00001000)        // PK3
#define Button_Rapid                (PINK & B00010000)        // PK4
#define Joy_Init()                  DDRK = B10000000;\
                                    PORTK = B00011111;        // Lifting PIN_A8, A9, A10, A11, A12 // external pull-up to +5 is desirable through 1K resistors


// ***** Mode *****
enum Mode
{
  Mode_Feed = 1,
  Mode_Cone_L,
  Mode_Cone_R,
  Mode_Thread,
  Mode_Divider
};

enum Sub_Mode_Thread
{
  Sub_Mode_Thread_Int = 1,
  Sub_Mode_Thread_Man,
  Sub_Mode_Thread_Ext
};


// ***** Feeds *****
// On the T5 meter we will build the feed
#define Timer5_Init()                     TCCR5A = 0;\
                                          TCCR5B = (1 << WGM52) | (1 << CS52) | (1 << CS51) | (1 << CS50)
                                          
struct feed_info_type
{
  byte  Div_Z;                                        // Divider for "longitudinal feed" the whole part
  byte  Div_X;                                        // Divider for "cross feed" the whole part
  char  Feed_Print[7];
  char  Limit_Print[8];
};
const feed_info_type Feed_Info[] =
{
   { 113, 75,   "0.03mm", " ---rpm" },   // We calculate according to the formula:
   {  56, 38,   "0.06mm", " ---rpm" },   // Enc_Line(1800)/(Step_Per_Revolution/Feed_Screw*Feed_mm)
   {  38, 25,   "0.09mm", " ---rpm" },
   {  28, 19,   "0.12mm", " ---rpm" },
   {  23, 15,   "0.15mm", " ---rpm" },
   {  19, 13,   "0.18mm", " ---rpm" },
   {  16, 11,   "0.21mm", " ---rpm" },
};
#define TOTAL_FEEDS (sizeof(Feed_Info) / sizeof(Feed_Info[0]))  //


// ***** Cone *****
struct cone_info_type
{
  byte Cs_Div;            // Divisor whole part
  int  Cm_Div;            // Divisor fractional part
  char Cone_Print[6];
};
const cone_info_type Cone_Info[] =
{
   { 25, 6160,  "  KM0" },   // k = (Cone * X_Steps_mm / Z_Steps_mm) * 2
   { 26, 7293,  "  KM1" },  // KM0 etc, are the Equivalent of MT0 (Morse Taper) as defined by GOST
   { 26, 6933,  "  KM2" },  // B0 are a shortened version of MT
   { 26, 5627,  "  KM3" },
   { 25, 6720,  "  KM4" },
   { 25, 3360,  "  KM5" },
   { 25, 5733,  "  KM6" },
   {  5, 3333,  "  1:4" },
   {  6, 6667,  "  1:5" },
   {  9, 3333,  "  1:7" },
   { 13, 3333,  " 1:10" },
   { 21, 3333,  " 1:16" },
   { 26, 6667,  " 1:20" },
   { 32,    0,  " 1:24" },
   { 40,    0,  " 1:30" },
   { 66, 6667,  " 1:50" },
   { 12, 1905,  " 7:64" },
   { 4,  7436,  " 8deg" },
   { 3,  7809,  "10deg" },
   { 2,  4880,  "15deg" },
   { 1,  1547,  "30deg" }
};
#define TOTAL_CONE (sizeof(Cone_Info) / sizeof(Cone_Info[0]))


// ***** Threads *****
struct thread_info_type
{
  byte Ks_Div_Z;                                      // Divider for "Carving" the whole part
  int  Km_Div_Z;                                      // Divisor for "Thread" fractional part
  byte Ks_Div_X;                                      // Divider for "Cross-Thread" the whole part
  int  Km_Div_X;                                      // Divider for "Transverse Carving" fractional part
  char Thread_Print[7];                               // Text that will show up on screen for selection
  float Step;                                         // Initial step used to calculate the threading steps in automatic mode
  byte Pass;                                          // Number of passes to complete thread in automatic mode
  char Limit_Print[8];
};
const thread_info_type Thread_Info[] =
//  Z    Z     X     X       Screen  Thread  No of  Max RPM
//  Axis Axis  Axis  Axis    Pitch   Pitch   Thread Screen
// whole dec. whole  dec.    Text     mm     Passes Text                                 
{                                                              
   { 27,    0,   18,    0,   "0.25mm", 0.250,  4, " 999rpm" }, // We calculate according to the formula:
   { 22, 5000,   15,    0,   "0.30mm", 0.300,  4, " 999rpm" }, // Enc_Tick(3600)/(Step_Per_Revolution/Feed_Screw*Thread_mm)
   { 19, 2857,   12, 8571,   "0.35mm", 0.350,  4, " 999rpm" }, // It is calculated under 800 step / turn (1/4 splitting up, 1.5mm, 1.0 pitch of screws)
   { 16, 8750,   11, 2500,   "0.40mm", 0.400,  4, " 999rpm" }, // See spreadsheet Calc_Feeds_Threads_Cone_EN.ods to calculate new settings
   { 13, 5000,    9,    0,   "0.50mm", 0.500,  4, " 999rpm" },
   { 11, 2500,    7, 5000,   "0.60mm", 0.600,  4, " 950rpm" },
   {  9, 6429,    6, 4286,   "0.70mm", 0.700,  4, " 810rpm" },
   {  9,    0,    6,    0,   "0.75mm", 0.750,  5, " 760rpm" },
   {  8, 4375,    5, 6250,   "0.80mm", 0.800,  5, " 710rpm" },
   {  6, 7500,    4, 5000,   "1.00mm", 1.000,  6, " 560rpm" },
   {  5, 4000,    3, 6000,   "1.25mm", 1.250,  7, " 460rpm" },
   {  4, 5000,    3,    0,   "1.50mm", 1.500,  7, " 380rpm" },
   {  3, 8571,    2, 5714,   "1.75mm", 1.750,  8, " 320rpm" },
   {  3, 3750,    2, 2500,   "2.00mm", 2.000,  9, " 280rpm" },
   {  2, 7000,    1, 8000,   "2.50mm", 2.500, 11, " 220rpm" },
   {  2, 2500,    1, 5000,   "3.00mm", 3.000, 15, " 190rpm" },
   {  1, 6875,    1, 1250,   "4.00mm", 4.000, 22, " 140rpm" },
   
   { 21, 2598,   14, 1732,   "80tpi ", 0.318,  4, " 999rpm" },
   { 19, 1339,   12, 7559,   "72tpi ", 0.353,  4, " 999rpm" },
   { 17,   79,   11, 3386,   "64tpi ", 0.397,  4, " 999rpm" },
   { 15, 9449,   10, 6299,   "60tpi ", 0.423,  4, " 999rpm" },
   { 14, 8819,    9, 9213,   "56tpi ", 0.454,  4, " 999rpm" },
   { 12, 7559,    8, 5039,   "48tpi ", 0.529,  4, " 999rpm" },
   { 11, 6929,    7, 7953,   "44tpi ", 0.577,  4, " 950rpm" },
   { 10, 6299,    7,  866,   "40tpi ", 0.635,  4, " 900rpm" },
   {  9, 5669,    6, 3780,   "36tpi ", 0.706,  5, " 760rpm" },
   {  8, 5039,    5, 6693,   "32tpi ", 0.794,  5, " 710rpm" },
   {  7, 4409,    4, 9606,   "28tpi ", 0.907,  5, " 650rpm" },
   {  7, 1752,    4, 7835,   "27tpi ", 0.941,  5, " 600rpm" },
   {  6, 9095,    4, 6063,   "26tpi ", 0.977,  6, " 570rpm" },
   {  6, 3780,    4, 2520,   "24tpi ", 1.058,  6, " 500rpm" },
   {  5, 8465,    3, 8976,   "22tpi ", 1.155,  6, " 450rpm" },
   {  5, 3150,    3, 5433,   "20tpi ", 1.270,  7, " 440rpm" },
   {  5,  492,    3, 3661,   "19tpi ", 1.337,  7, " 420rpm" },
   {  4, 7835,    3, 1890,   "18tpi ", 1.411,  7, " 380rpm" },
   {  4, 2520,    2, 8346,   "16tpi ", 1.588,  8, " 350rpm" },
   {  3, 7205,    2, 4803,   "14tpi ", 1.814,  9, " 320rpm" },
   {  3, 1890,    2, 1260,   "12tpi ", 2.117, 10, " 270rpm" },
   {  2, 6575,    1, 7717,   "10tpi ", 2.540, 11, " 220rpm" },
   {  2, 3917,    1, 5945,   " 9tpi ", 2.822, 14, " 190rpm" },
   {  2, 1260,    1, 4173,   " 8tpi ", 3.175, 16, " 170rpm" },
   {  1, 8602,    1, 2402,   " 7tpi ", 3.629, 19, " 150rpm" },
   {  1, 5945,    1,  630,   " 6tpi ", 4.233, 24, " 140rpm" }
};
#define TOTAL_THREADS (sizeof(Thread_Info) / sizeof(Thread_Info[0]))
#define PASS_FINISH   3



// ***** Interrupts *****
#define INT0_Init()               EICRA |= (1 << ISC00)   //Pin 21, encoder STEP, External Interrupt Control Register A set INT0 to trigger on ANY logic change (If there is a step up or down)

#define Enable_INT0()             EIMSK |= (1 << INT0)    //Turns on INT0 Interup
#define Disable_INT0()            EIMSK &= ~(1 << INT0)   //

#define Enable_INT_OCR2A()        TCNT2 = 0;\
                                  TIFR2 = (1<<OCF2A);\
                                  TIMSK2 = (1 << OCIE2A)
#define Disable_INT_OCR2A()       TIMSK2 &= ~(1 << OCIE2A)

#define Enable_INT_OCR2B()        TCNT2 = 0;\
                                  TIFR2 = (1<<OCF2B);\
                                  TIMSK2 = (1 << OCIE2B)
#define Disable_INT_OCR2B()       TIMSK2 &= ~(1 << OCIE2B)

#define Enable_INT_OCR5A()        TCNT5 = 0;\
                                  TIFR5 = (1<<OCF5A);\
                                  TIMSK5 = (1 << OCIE5A)
#define Disable_INT_OCR5A()       TIMSK5 &= ~(1 << OCIE5A)

#define Enable_INT_OCR5B()        TCNT5 = 0;\
                                  TIFR5 = (1<<OCF5B);\
                                  TIMSK5 = (1 << OCIE5B)
#define Disable_INT_OCR5B()       TIMSK5 &= ~(1 << OCIE5B)

////////////////////////////////////////////////////////////
#define Ena_INT_Thrd()            Disable_INT_OCR2A();\
                                  Disable_INT_OCR2B();\
                                  Disable_INT_OCR5A();\
                                  Disable_INT_OCR5B();\
                                  Enable_INT0()

#define Ena_INT_Z_Feed()          Disable_INT0();\
                                  Disable_INT_OCR2A();\
                                  Disable_INT_OCR2B();\
                                  Disable_INT_OCR5B();\
                                  Enable_INT_OCR5A()
                                     
#define Ena_INT_X_Feed()          Disable_INT0();\
                                  Disable_INT_OCR2A();\
                                  Disable_INT_OCR2B();\
                                  Disable_INT_OCR5A();\
                                  Enable_INT_OCR5B()
                                     
#define Ena_INT_Z_Rapid()         Disable_INT0();\
                                  Disable_INT_OCR2B();\
                                  Disable_INT_OCR5A();\
                                  Disable_INT_OCR5B();\
                                  Enable_INT_OCR2A()
                                     
#define Ena_INT_X_Rapid()         Disable_INT0();\
                                  Disable_INT_OCR2A();\
                                  Disable_INT_OCR5A();\
                                  Disable_INT_OCR5B();\
                                  Enable_INT_OCR2B()
                                     

// ***** My Flags *****
typedef struct
{ 
   uint8_t bit0 : 1;
   uint8_t bit1 : 1;
   uint8_t bit2 : 1;
   uint8_t bit3 : 1;
   uint8_t bit4 : 1;
   uint8_t bit5 : 1;
   uint8_t bit6 : 1;
   uint8_t bit7 : 1;
}FLAG;
#define Spindle_Dir        ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit0    // CW-0, CCW-1
#define Motor_Z_Dir        ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit1    // CW-0, CCW-1
#define Joy_Z_flag         ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit2    // On-1, Off-0
#define Step_Z_flag        ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit3    // On-1, Off-0
#define Motor_X_Dir        ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit4    // CW-0, CCW-1
#define Joy_X_flag         ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit5    // On-1, Off-0
#define Step_X_flag        ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit6    // On-1, Off-0
#define Cone_flag          ((volatile FLAG*)_SFR_MEM_ADDR(GPIOR0))->bit7    // On-1, Off-0

bool Spindle_flag = OFF;
bool Feed_Z_flag = OFF;
bool Feed_X_flag = OFF;

bool Rapid_Step_Z_flag = OFF;
bool Rapid_Step_X_flag = OFF;
bool Rapid_Z_flag = OFF;
bool Rapid_X_flag = OFF;

bool Limit_Pos_Left_flag = OFF;
bool Limit_Pos_Right_flag = OFF;
bool Limit_Pos_Front_flag = OFF;
bool Limit_Pos_Rear_flag = OFF;

bool Limit_Button_flag = OFF;
bool Button_flag = OFF;

bool a_flag = false;
bool b_flag = false;
bool c_flag = false;
bool d_flag = false;
bool cycle_flag = false;
bool beep_flag = OFF;
bool bbeep_flag = OFF;


// ***** MY VARIABLES *****
int Tacho_Count = 0;
int Tacho_Count_Old =0;
byte Spindle_Count = 0;

int Enc_Pos = 0;                            // Encoder position counter

byte Ks_Count = 0;                          // Counter for "Feed", "Thread" the whole part
int Km_Count = 0;                           // Counter for "Thread" fractional part
byte Ks_Divisor = 0;                        // Divider for "Feed", "Thread" the whole part
byte tmp_Ks_Divisor = ACCEL;
int Km_Divisor = 0;                         // Divisor for "Thread" fractional part

byte Cs_Count = 0;                          // Counter for "Cone" the whole part
int Cm_Count = 0;                           // Counter for "Cone" fractional part
byte Cs_Divisor = 0;                        // Divisor for "Cone" the whole part
int Cm_Divisor = 0;                         // Divisor for "Cone" fractional part

byte tmp_Accel = ACCEL;
byte Repeat_Count = 0;

int Brake_Compens = 0;                      // Braking start compensation

byte Mode = Mode_Feed;                      // Boot mode
byte Sub_Mode_Thread = Sub_Mode_Thread_Man; // Submode at boot time
byte Feed_Step = 2;                         // Array sampling at boot (0.09mm)
byte Thread_Step = 11;                      // Array sampling at boot (1.5mm)
byte Cone_Step = 0;                         // Array sampling at boot (KM0)

long Motor_Z_Pos = 0;
long Motor_X_Pos = 0;

long Limit_Pos_Left = Limit_Pos_Max;
long Limit_Pos_Right = Limit_Pos_Min;
long Limit_Pos_Front = Limit_Pos_Max;
long Limit_Pos_Rear = Limit_Pos_Min;
long Limit_Pos = 0;

byte Start_Speed = Feed_Info[2].Div_Z;
byte max_OCR5A = Start_Speed;

byte Total_Tooth = 1;                  // Number of dividing teeth
byte Current_Tooth = 1;                // Current division tooth

byte Pass_Total = 0;                   // Total number of passes per cycle
byte Pass_Nr = 1;                      // Nr of the current pass for the cycle
long Null_X_Pos = 0;                   // Reference point for the cross-section of the cycle

float Step_mm = 0;
float Pass_Depth = 0;                  // The value of the current depth in whole steps, for the cycle
long Infeed_Value = 0;                 // The value of the current depth in microsteps, for the cycle


byte Beep_Count = 0;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("    ELS v.6c    ");
  delay(2500);
  TIMSK0 = 0;
    
  Encoder_Init();  
  Motor_Init();
  
  INT0_Init();
  
  Timer5_Init();
  OCR5A = max_OCR5A;
  Timer2_Init();
  OCR2A = MIN_RAPID_MOTION;
  Ena_INT_Z_Feed();
  
  Joy_Init();                                     
  LimitInit();

  Beeper_Init();
  Beeper_Off();
  
  Spindle_Dir = CW;
  Motor_Z_Dir = CW;
  Joy_Z_flag = OFF;
  Step_Z_flag = OFF;
  Motor_X_Dir = CW;
  Joy_X_flag = OFF;
  Step_X_flag = OFF;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  Spindle();
  Menu();
  Joystick();
  Limit_Button();
  Print();
  Beep();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Beeper ********** ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Beep()
{
   if (bbeep_flag == ON)
   { 
      Beep_Count++;
      if      (Beep_Count < 6)   { Beeper_On(); }
      else if (Beep_Count == 6)  { Beeper_Off(); }
      else if (Beep_Count == 24) { Beeper_On(); }
      else if (Beep_Count == 30) { Beeper_Off(); }
      else if (Beep_Count > 120) { Beep_Count = 0; }
      return;
   }

   if (beep_flag == ON)
   {
      if (++Beep_Count < 6) {Beeper_On();}
      else
      {
         Beeper_Off();
         beep_flag = OFF;
         Beep_Count = 254;
      }
   }
   else {Beep_Count = 0;}

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Spindle is running ********** ////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Spindle()
{
   if (++Spindle_Count > 25)
   {
      Spindle_Count = 0;
      if (Tacho_Count == Tacho_Count_Old) {Spindle_flag = OFF;}  //If the current spindle count is the same as the previous spindle count, set the spindle flag to off
      else Spindle_flag = ON;
      Tacho_Count_Old = Tacho_Count;  //If the spindle is spinning, then set the spindle flag to on
   }
   
   if (Spindle_flag == OFF) 
   {
      if (Mode == Mode_Feed)    //If in the feed mode and the spindle flag is off set all the motor outputs to off
      {
         Ks_Count = 0;
         Km_Count = 0;
         Feed_Z_flag = OFF;
         Feed_X_flag = OFF;
         Step_Z_flag = OFF;
         Step_X_flag = OFF;
         OCR5A = max_OCR5A;
         if (Button_Rapid) {Repeat_Count = 0;}
      }
      if (Mode == Mode_Cone_L || Mode == Mode_Cone_R) //If in the cone mode and the spindle flag is off set all the motor outputs to off
      {
         Ks_Count = 0;
         Km_Count = 0;
         Feed_Z_flag = OFF;
         Feed_X_flag = OFF;
         Step_Z_flag = OFF;
         OCR5A = max_OCR5A;
         if (Button_Rapid) {Repeat_Count = 0;}
      }
   }   
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Event processing function in the main menu ********** ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Menu()
{
  byte Pressed_Key;
  int ADC_value = analogRead(A0);
  if (ADC_value < 65)       Pressed_Key = Key_Right;
  else if (ADC_value < 220) Pressed_Key = Key_Up;
  else if (ADC_value < 390) Pressed_Key = Key_Down;
  else if (ADC_value < 600) Pressed_Key = Key_Left;
  else if (ADC_value < 870) Pressed_Key = Key_Select;
  else Pressed_Key = Key_None;
  
  if (Pressed_Key == Key_None) {Button_flag = false;}
  
  if (!Button_flag)
  {
      switch (Pressed_Key)
      {
        case Key_Left:
          MenuKeyLeftPressed();
          break;
        case Key_Right:
          MenuKeyRightPressed();
          break;
        case Key_Up:
          MenuKeyUpPressed();
          break;
        case Key_Down:
          MenuKeyDownPressed();
          break;       
        case Key_Select:
          MenuKeySelectPressed();    
          break;
      }
  }
}

// ********** Button click handler Select **********
void MenuKeySelectPressed()
{
  switch (Mode) 
  {
     case Mode_Feed:
        if (!Joy_Z_flag && !Joy_X_flag)
        {
           Ks_Count = 0;
           Km_Count = 0;
           Repeat_Count = 0;
           Mode++;
           beep_flag = ON;
           Ena_INT_Z_Feed();

           Step_Z_flag = OFF;
           Step_X_flag = OFF;
           Rapid_Step_Z_flag = OFF;
           Rapid_Step_X_flag = OFF;
        }
     break;
        
     case Mode_Cone_L:
        if (!Joy_Z_flag && !Joy_X_flag)
        {
           Ks_Count = 0;
           Km_Count = 0;
           Repeat_Count = 0;
           Mode++;
           beep_flag = ON;
           Ena_INT_Z_Feed();
            
           Step_Z_flag = OFF;
           Step_X_flag = OFF;
           Rapid_Step_Z_flag = OFF;
           Rapid_Step_X_flag = OFF;
        }
     break;
     
     case Mode_Cone_R:
        if (!Joy_Z_flag && !Joy_X_flag)
        {
           Ks_Count = 0;
           Km_Count = 0;
           Repeat_Count = 0;
           Mode++;
           beep_flag = ON;
           Sub_Mode_Thread = Sub_Mode_Thread_Man;
           Ena_INT_Thrd();
            
           Step_Z_flag = OFF;
           Step_X_flag = OFF;
           Rapid_Step_Z_flag = OFF;
           Rapid_Step_X_flag = OFF;
        }
     break;
    
     case Mode_Thread:
        if (!Joy_Z_flag && !Joy_X_flag)
        {
           Ks_Count = 0;
           Km_Count = 0;
           Repeat_Count = 0;
           Mode++;
           beep_flag = ON;
           Ena_INT_Thrd();
            
           Step_Z_flag = OFF;
           Step_X_flag = OFF;
           Rapid_Step_Z_flag = OFF;
           Rapid_Step_X_flag = OFF;
        }
     break;         
             
     case Mode_Divider:
        if (!Joy_Z_flag && !Joy_X_flag)
        {
           Ks_Count = 0;
           Km_Count = 0;
           Repeat_Count = 0;
           Mode = 1;
           beep_flag = ON;
           Ena_INT_Z_Feed();
            
           Step_Z_flag = OFF;
           Step_X_flag = OFF;
           Rapid_Step_Z_flag = OFF;
           Rapid_Step_X_flag = OFF;
        }
     break;
   }
   Button_flag = true;
}

// ********** Button click handler Up **********
void MenuKeyUpPressed()
{
  switch (Mode)
  {
     case Mode_Feed:
        if (Feed_Step < TOTAL_FEEDS - 1)
        {
           Feed_Step++;
           beep_flag = ON;
           if (Joy_Z_flag == ON)
           {
              b_flag = false;
              if (Motor_Z_Dir == CW) {Feed_Left(a_flag, b_flag);}
              else                   {Feed_Right(a_flag, b_flag);} 
           }
           else if (Joy_X_flag == ON)
           {
              b_flag = false;
              if (Motor_X_Dir == CW) {Feed_Front(a_flag, b_flag);}
              else                   {Feed_Rear(a_flag, b_flag);} 
           }
        }
     break;
    
     case Mode_Cone_L:
     case Mode_Cone_R:
        if (Feed_Step < TOTAL_FEEDS - 1)
        {
           Feed_Step++;
           beep_flag = ON;
           if (Joy_Z_flag == ON)
           {
              b_flag = false;
              if (Motor_Z_Dir == CW) {Cone_Left(a_flag, b_flag);}
              else                   {Cone_Right(a_flag, b_flag);} 
           }
           else if (Joy_X_flag == ON)
           {
              //
           }
        }
      break;
    
      case Mode_Thread:
        if (Thread_Step < TOTAL_THREADS - 1)
        {
           if (!Joy_Z_flag && !Joy_X_flag)
           {
              Ks_Count = 0;
              Km_Count = 0;
              Repeat_Count = 0;
              Thread_Step++;
              beep_flag = ON;

              Step_Z_flag = OFF;
              Step_X_flag = OFF;
              Rapid_Step_Z_flag = OFF;
              Rapid_Step_X_flag = OFF;
           }
        }
     break;        
        
     case Mode_Divider:
        if (Total_Tooth < 255)
        {
           Total_Tooth++;
           beep_flag = ON;
           Current_Tooth = 1;
        }
     break;
  }
  Button_flag = true;
}

// ********** Button click handler Down **********
void MenuKeyDownPressed()
{
   switch (Mode)
   {
      case Mode_Feed:
         if (Feed_Step > 0)
         {
            Feed_Step--;
            beep_flag = ON;
            if (Joy_Z_flag == ON)
            {
               b_flag = false;
               if (Motor_Z_Dir == CW) {Feed_Left(a_flag, b_flag);}
               else                   {Feed_Right(a_flag, b_flag);} 
            }
            else if (Joy_X_flag == ON)
            {
               b_flag = false;
               if (Motor_X_Dir == CW) {Feed_Front(a_flag, b_flag);}
               else                   {Feed_Rear(a_flag, b_flag);} 
            }
         }
      break;
      
      case Mode_Cone_L:
      case Mode_Cone_R:
         if (Feed_Step > 0)
         {
            Feed_Step--;
            beep_flag = ON;
            if (Joy_Z_flag == ON)
            {
               b_flag = false;
               if (Motor_Z_Dir == CW) {Cone_Left(a_flag, b_flag);}
               else                   {Cone_Right(a_flag, b_flag);} 
            }
            else if (Joy_X_flag == ON)
            {
               //
            }
         }
      break;    
     
      case Mode_Thread:
        if (Thread_Step > 0)
        {
           if (!Joy_Z_flag && !Joy_X_flag)
           {
              Ks_Count = 0;
              Km_Count = 0;
              Repeat_Count = 0;
              Thread_Step--;
              beep_flag = ON;

              Step_Z_flag = OFF;
              Step_X_flag = OFF;
              Rapid_Step_Z_flag = OFF;
              Rapid_Step_X_flag = OFF;
           }
        }
        break;        
        
      case Mode_Divider:
        if (Total_Tooth > 1)
        {
           Total_Tooth--;
           beep_flag = ON;
           Current_Tooth = 1;
        }
        break;
   }
   Button_flag = true;
}

// ********** Button click handler Left **********
void MenuKeyLeftPressed()
{
   switch (Mode)
   {
      case Mode_Cone_L:
      case Mode_Cone_R:
        if (Cone_Step > 0)
        {
           if (!Joy_Z_flag && !Joy_X_flag)
           {
              Ks_Count = 0;
              Km_Count = 0;
              Repeat_Count = 0;
              Cone_Step--;
              beep_flag = ON;

              Step_Z_flag = OFF;
              Step_X_flag = OFF;
              Rapid_Step_Z_flag = OFF;
              Rapid_Step_X_flag = OFF;
           }
        }
      break;
     
      case Mode_Divider:
         if (Current_Tooth > 1)
         {
            Current_Tooth--;
            beep_flag = ON;
         }
         else if (Current_Tooth == 1)
         {
            Current_Tooth = Total_Tooth;
            beep_flag = ON;
         }
      break;  

      case Mode_Thread:
      switch (Sub_Mode_Thread)
      {
         case Sub_Mode_Thread_Man:
         if ((!Joy_Z_flag && !Joy_X_flag) && (Limit_Pos_Left_flag == ON && Limit_Pos_Right_flag == ON))
         {
            Ks_Count = 0;
            Km_Count = 0;
            Repeat_Count = 0;
            Sub_Mode_Thread--;   // Innen
            beep_flag = ON;

            Step_Z_flag = OFF;
            Step_X_flag = OFF;
            Rapid_Step_Z_flag = OFF;
            Rapid_Step_X_flag = OFF;
         }
         break;
        
         case Sub_Mode_Thread_Ext:
         if (!Joy_Z_flag && !Joy_X_flag)
         {
            Ks_Count = 0;
            Km_Count = 0;
            Repeat_Count = 0;
            Sub_Mode_Thread--;   // Manual
            beep_flag = ON;

            Step_Z_flag = OFF;
            Step_X_flag = OFF;
            Rapid_Step_Z_flag = OFF;
            Rapid_Step_X_flag = OFF;
         }
         break;
      }
      break;
   }
   Button_flag = true;
}

// ********** Button click handler Right **********
void MenuKeyRightPressed()
{
   switch (Mode)
   { 
      case Mode_Cone_L:
      case Mode_Cone_R:
        if (Cone_Step < TOTAL_CONE - 1)
        {
           if (!Joy_Z_flag && !Joy_X_flag)
           {
              Ks_Count = 0;
              Km_Count = 0;
              Repeat_Count = 0;
              Cone_Step++;
              beep_flag = ON;

              Step_Z_flag = OFF;
              Step_X_flag = OFF;
              Rapid_Step_Z_flag = OFF;
              Rapid_Step_X_flag = OFF;
           }
        }
      break;
     
      case Mode_Divider:
         if (Current_Tooth < Total_Tooth)
         {
            Current_Tooth++;
            beep_flag = ON;
         }
         else if (Current_Tooth == Total_Tooth)
         {
            Current_Tooth = 1;
            beep_flag = ON;
         }
      break;

      case Mode_Thread:
      switch (Sub_Mode_Thread)
      {
         case Sub_Mode_Thread_Man:
         if ((!Joy_Z_flag && !Joy_X_flag) && (Limit_Pos_Left_flag == ON && Limit_Pos_Right_flag == ON))
         {
            Ks_Count = 0;
            Km_Count = 0;
            Repeat_Count = 0;   
            Sub_Mode_Thread++;   // Ext
            beep_flag = ON;
            
            Step_Z_flag = OFF;
            Step_X_flag = OFF;
            Rapid_Step_Z_flag = OFF;
            Rapid_Step_X_flag = OFF;
         }
         break;
        
         case Sub_Mode_Thread_Int:
         if (!Joy_Z_flag && !Joy_X_flag)
         {
            Ks_Count = 0;
            Km_Count = 0;
            Repeat_Count = 0;
            Sub_Mode_Thread++;  // Man
            beep_flag = ON;

            Step_Z_flag = OFF;
            Step_X_flag = OFF;
            Rapid_Step_Z_flag = OFF;
            Rapid_Step_X_flag = OFF;
         }
         break;
      } 
      break;
   }  
   Button_flag = true;
}
  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Processing limit buttons ********** /////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Limit_Button()
{
  // ********** Button click handler Limit_Left **********
  if (!Limit_Left_Button)
  {
     if (!Limit_Button_flag)
     {
        Limit_Button_flag = true;
        switch (Mode)
        { 
           case Mode_Feed:
           case Mode_Cone_L:
           case Mode_Cone_R:
           case Mode_Thread:
           if (!Joy_Z_flag)
           { 
              if (Limit_Pos_Left_flag == OFF)
              {
                 if (Motor_Z_Pos > (Limit_Pos_Right + ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
                 {
                    Limit_Pos_Left_flag = ON;
                    Limit_Pos_Left = ((Motor_Z_Pos + McSTEP_Z / 2) & ~(McSTEP_Z - 1));
                    Limit_Left_LED_On();
                    beep_flag = ON;
                 }
              }
              else
              {
                 Limit_Pos_Left_flag = OFF;
                 Limit_Pos_Left = Limit_Pos_Max;
                 Limit_Left_LED_Off();
                 beep_flag = ON;
              }
           }
        }
     }
  } 
   
  // ********** Button click handler Limit_Right **********
  else if (!Limit_Right_Button)
  {
     if (!Limit_Button_flag)
     {
        Limit_Button_flag = true;
        switch (Mode)
        {
           case Mode_Feed:
           case Mode_Cone_L:
           case Mode_Cone_R:
           case Mode_Thread:
           if (!Joy_Z_flag)
           { 
              if (Limit_Pos_Right_flag == OFF)
              {
                 if (Motor_Z_Pos < (Limit_Pos_Left - ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
                 {
                    Limit_Pos_Right_flag = ON;
                    Limit_Pos_Right = ((Motor_Z_Pos + McSTEP_Z / 2) & ~(McSTEP_Z - 1));
                    Limit_Right_LED_On();
                    beep_flag = ON;
                 }
              }
              else
              {
                 Limit_Pos_Right_flag = OFF;
                 Limit_Pos_Right = Limit_Pos_Min;
                 Limit_Right_LED_Off();
                 beep_flag = ON;
              }
           }
        }
     }
  }

   // ********** Button click handler Limit_Front **********
   else if (!Limit_Front_Button)
   {
      if (!Limit_Button_flag)
      {
         Limit_Button_flag = true;
         switch (Mode)
         { 
            case Mode_Thread:
            case Mode_Feed:
            if (!Joy_X_flag)
            { 
               if (Limit_Pos_Front_flag == OFF)
               {
                  if (Motor_X_Pos > (Limit_Pos_Rear + ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
                  {
                     Limit_Pos_Front_flag = ON;
                     Limit_Pos_Front = ((Motor_X_Pos + McSTEP_X / 2) & ~(McSTEP_X - 1));
                     Limit_Front_LED_On();
                     beep_flag = ON;
                  }
               }
               else
               {
                  Limit_Pos_Front_flag = OFF;
                  Limit_Pos_Front = Limit_Pos_Max;
                  Limit_Front_LED_Off();
                  beep_flag = ON;
               }
            }
         }
      }
   } 

   // ********** Button click handler Limit_Rear **********   
   else if (!Limit_Rear_Button)
   {
      if (!Limit_Button_flag)
      {
         Limit_Button_flag = true;
         switch (Mode)
         { 
            case Mode_Thread:
            case Mode_Feed:
            if (!Joy_X_flag)
            { 
               if (Limit_Pos_Rear_flag == OFF)
               {
                  if (Motor_X_Pos < (Limit_Pos_Front - ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
                  {
                     Limit_Pos_Rear_flag = ON;
                     Limit_Pos_Rear = ((Motor_X_Pos + McSTEP_X / 2) & ~(McSTEP_X - 1));
                     Limit_Rear_LED_On();
                     beep_flag = ON;
                  }
               }
               else
               {
                  Limit_Pos_Rear_flag = OFF;
                  Limit_Pos_Rear = Limit_Pos_Min;
                  Limit_Rear_LED_Off();
                  beep_flag = ON;
               }
            }
         }
      }
   }   
   
   else
   {
      Limit_Button_flag = false;
   }
   
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Joystick position processing ********** ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Joystick()
{
  if (!Joy_Left) {Joy_LeftPressed();}
  else if (!Joy_Right) {Joy_RightPressed();}
  else if (!Joy_Front)  {Joy_UpPressed();}
  else if (!Joy_Rear) {Joy_DownPressed();}
  else {Joy_NoPressed();}
}  
  

// ********** Handler Joystick Left **********
void Joy_LeftPressed()
{  
   if (Mode == Mode_Thread)
   {
      if (Sub_Mode_Thread == Sub_Mode_Thread_Man)
      {
         Joy_Z_flag = ON;
         Joy_X_flag = OFF;
         if (Spindle_Dir == CW) {Thread_Left(c_flag, d_flag);}    
         else
         {
            if (!Button_Rapid) {Limit_Pos = Limit_Pos_Min;}
            else               {Limit_Pos = Limit_Pos_Right + Brake_Compens;}
            Thread_Right(c_flag, d_flag);
         }
      }
      else if (Sub_Mode_Thread == Sub_Mode_Thread_Ext)
      {
         if (Spindle_Dir == CW) {Thread_Ext_Left();}    
         else                   {Thread_Ext_Right();}
      }
      else if (Sub_Mode_Thread == Sub_Mode_Thread_Int)
      {
         if (Spindle_Dir == CW) {Thread_Int_Left();}    
         else                   {Thread_Int_Right();}
      }
   }

   else if (Mode == Mode_Feed)
   {
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      if (!Button_Rapid)
      {  
         if (Motor_Z_Pos < (Limit_Pos_Left - ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
         {
            Feed_Z_flag = OFF;
            if (!Step_Z_flag) {Rapid_Feed_Left(a_flag, b_flag);}
         }
      }
      else
      {
         if (OCR5A == max_OCR5A)
         {
            Rapid_Z_flag = OFF;
            if (!Rapid_Step_Z_flag)
            {
               Feed_Z_flag = ON;
               Feed_Left(a_flag, b_flag);
            }
         }
      }
   }
   
   else if ((Mode == Mode_Cone_L) || (Mode == Mode_Cone_R))
   {
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      if (!Button_Rapid)
      {  
         if (Motor_Z_Pos < (Limit_Pos_Left - ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
         {
            Feed_Z_flag = OFF;
            if (!Step_Z_flag) {Rapid_Cone_Left(a_flag, b_flag);}
         }
      }
      else
      {
         if (OCR5A == max_OCR5A)
         {
            Rapid_Z_flag = OFF;
            if (!Rapid_Step_Z_flag)
            {
               Feed_Z_flag = ON;
               Cone_Left(a_flag, b_flag);
            }
         }
      }
   }
}  


// ********** Handler Joystick Right **********  
void Joy_RightPressed()
{
   if (Mode == Mode_Thread)
   {
      if (Sub_Mode_Thread == Sub_Mode_Thread_Man)
      {
         Joy_Z_flag = ON;
         Joy_X_flag = OFF;
         if (Spindle_Dir == CW)
         {
            if (!Button_Rapid) {Limit_Pos = Limit_Pos_Min;}
            else               {Limit_Pos = Limit_Pos_Right + Brake_Compens;}
            Thread_Right(c_flag, d_flag);
         }
         else {Thread_Left(c_flag, d_flag);}
      }
      else if (Sub_Mode_Thread == Sub_Mode_Thread_Ext)
      {
         if (Spindle_Dir == CW) {Thread_Ext_Right();}
         else                   {Thread_Ext_Left();}
      }
      else if (Sub_Mode_Thread == Sub_Mode_Thread_Int)
      {
         if (Spindle_Dir == CW) {Thread_Int_Right();}
         else                   {Thread_Int_Left();}
      }
   }
   
   else if (Mode == Mode_Feed)
   {
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      if (!Button_Rapid)
      {
         if (Motor_Z_Pos > (Limit_Pos_Right + ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
         {
            Feed_Z_flag = OFF;
            if (!Step_Z_flag) {Rapid_Feed_Right(a_flag, b_flag);}
         }
      }
      else
      {  
         if (OCR5A == max_OCR5A)
         {
            Rapid_Z_flag = OFF;
            if (!Rapid_Step_Z_flag)
            {
               Feed_Z_flag = ON;
               Feed_Right(a_flag, b_flag);
            }
         }
      }
   }
   
   else if (Mode == Mode_Cone_L || Mode == Mode_Cone_R)
   {
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      if (!Button_Rapid)
      {
         if (Motor_Z_Pos > (Limit_Pos_Right + ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
         {
            Feed_Z_flag = OFF;
            if (!Step_Z_flag) {Rapid_Cone_Right(a_flag, b_flag);}
         }
      }
      else
      {
         if (OCR5A == max_OCR5A)
         {
            Rapid_Z_flag = OFF;
            if (!Rapid_Step_Z_flag)
            {
               Feed_Z_flag = ON;
               Cone_Right(a_flag, b_flag);
            }
         }
      }
   }
}


// ********** Joystick Handler Up **********  
void Joy_UpPressed()
{
  Joy_Z_flag = OFF;
  Joy_X_flag = ON;
  
  if (Mode == Mode_Thread)
   {  
      if (Spindle_Dir == CW) {Thread_Front(c_flag, d_flag);}
      else                   {Thread_Rear(c_flag, d_flag);}
   }
      
   else if (Mode == Mode_Feed)
   {
      if (!Button_Rapid)
      {
         if (Motor_X_Pos < (Limit_Pos_Front - ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
         {  
            Feed_X_flag = OFF;
            if (!Step_X_flag) {Rapid_Feed_Front(a_flag, b_flag);}
         }
      }
      else
      {
         if (OCR5A == max_OCR5A)
         {
            Rapid_X_flag = OFF;
            if (!Rapid_Step_X_flag)
            {
               Feed_X_flag = ON;
               Feed_Front(a_flag, b_flag);
            }
         }
      }
   }
   
   else if (Mode == Mode_Cone_L)
   {
      //
   }
   
   else if (Mode == Mode_Cone_R)
   {
      //
   }
}


// ********** Processor Joystick Down **********  
void Joy_DownPressed()
{
  Joy_Z_flag = OFF;
  Joy_X_flag = ON;

  if (Mode == Mode_Thread)
   {
      if (Spindle_Dir == CW) {Thread_Rear(c_flag, d_flag);}
      else                   {Thread_Front(c_flag, d_flag);}
   }
   
   else if (Mode == Mode_Feed)
   {
      if (!Button_Rapid)
      {
         if (Motor_X_Pos > (Limit_Pos_Rear + ((MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt) * 2))
         {  
            Feed_X_flag = OFF;
            if (!Step_X_flag) {Rapid_Feed_Rear(a_flag, b_flag);}
         }
      }
      else
      {
         if (OCR5A == max_OCR5A)
         {
            Rapid_X_flag = OFF;
            if (!Rapid_Step_X_flag)
            {
               Feed_X_flag = ON;
               Feed_Rear(a_flag, b_flag);
            }
         }
      }
   }
   
   else if (Mode == Mode_Cone_L)
   {
      //
   }
   
   else if (Mode == Mode_Cone_R)
   {
      //
   }
}


// ********** Joystick in neutral **********  
void Joy_NoPressed()
{
   Joy_Z_flag = OFF;
   Joy_X_flag = OFF;
   Feed_Z_flag = OFF;
   Feed_X_flag = OFF;
   Rapid_Z_flag = OFF;
   Rapid_X_flag = OFF;
   
   b_flag = false;
   if (!Step_Z_flag && !Rapid_Step_Z_flag)
   {
      Motor_Z_Disable();
      a_flag = false;
      c_flag = false;
      d_flag = false;
   }
   if (!Step_X_flag && !Rapid_Step_X_flag)
   {
      Motor_X_Disable();
      a_flag = false;
      c_flag = false;
      d_flag = false;
   }
   
   if (Mode == Mode_Feed || Mode == Mode_Cone_L || Mode == Mode_Cone_R)
   {
      if (Spindle_flag == OFF)
      {
         Ks_Count = 0;
         Km_Count = 0;
         Repeat_Count = 0;
         Step_Z_flag = OFF;
         Step_X_flag = OFF;
         OCR5A = max_OCR5A;
      }
   }
   
   if (Mode == Mode_Thread)
   {
      if (Sub_Mode_Thread != Sub_Mode_Thread_Man)
      {
         Pass_Nr = 1;
         Null_X_Pos = Motor_X_Pos;
         
         Limit_Pos_Front = Limit_Pos_Max;
         Limit_Front_LED_Off();
         Limit_Pos_Rear = Limit_Pos_Min;
         Limit_Rear_LED_Off();

         Motor_Z_Disable();
         Motor_X_Disable();
         Ks_Count = 0;
         Km_Count = 0;
         Repeat_Count = 0;
         Step_Z_flag = OFF;
         Step_X_flag = OFF;
         cycle_flag = false;
         a_flag = false;
         c_flag = false;
         d_flag = false;
         bbeep_flag = OFF;
      }
   }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Thread mode ********** //////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Thread_Left(bool & c_flag, bool & d_flag)
{
   if (c_flag == true) return;
   d_flag = false;
   c_flag = true;
  
   Motor_Z_Enable();
   Motor_Z_CW();
   Motor_Z_Dir = CW;

   if (Motor_Z_Pos < (Limit_Pos_Left - (ACCEL * REPEAt )* 2)) // This section speeds up or slows down the leadscrew to stay on target for the thread.
   {
      Ks_Divisor = Thread_Info[Thread_Step].Ks_Div_Z;
      if (tmp_Ks_Divisor != Ks_Divisor)
      {
         tmp_Accel = ACCEL + Ks_Divisor;
         tmp_Ks_Divisor = ACCEL + Ks_Divisor;
      }
      Brake_Compens = ACCEL * REPEAt + 1;
   }
   else
   {
      Ks_Divisor = ACCEL + Thread_Info[0].Ks_Div_Z;
      tmp_Accel = Ks_Divisor;
      tmp_Ks_Divisor = Ks_Divisor;
      Brake_Compens = tmp_Accel - Ks_Divisor + 1;
   }
   
   Km_Divisor = Thread_Info[Thread_Step].Km_Div_Z;
   Ks_Count = 0;
   Km_Count = 0;
   Limit_Pos = Limit_Pos_Left - Brake_Compens;
}
  

void Thread_Right(bool & c_flag, bool & d_flag)
{
   if (d_flag == true) return;
   c_flag = false;
   d_flag = true;
  
   Motor_Z_Enable();
   Motor_Z_CCW();
   Motor_Z_Dir = CCW;

   if (Motor_Z_Pos > Limit_Pos_Right + ACCEL * REPEAt * 2 || Motor_Z_Pos <= Limit_Pos_Right)
   {
      Ks_Divisor = Thread_Info[Thread_Step].Ks_Div_Z;
      if (tmp_Ks_Divisor != Ks_Divisor)
      {
         tmp_Accel = ACCEL + Ks_Divisor;
         tmp_Ks_Divisor = ACCEL + Ks_Divisor;
      }  
      Brake_Compens = ACCEL * REPEAt + 1;
   }
   else
   {
      Ks_Divisor = ACCEL + Thread_Info[0].Ks_Div_Z;
      tmp_Accel = Ks_Divisor;
      tmp_Ks_Divisor = Ks_Divisor;
      Brake_Compens = tmp_Accel - Ks_Divisor + 1;
   }

    Km_Divisor = Thread_Info[Thread_Step].Km_Div_Z;
    Ks_Count = 0;
    Km_Count = 0;
    Limit_Pos = Limit_Pos_Right + Brake_Compens;  
}


void Thread_Front(bool & c_flag, bool & d_flag)
{
   if (c_flag == true) return;
   d_flag = false;
   c_flag = true;
  
   Motor_X_Enable();
   Motor_X_CW();
   Motor_X_Dir = CW;

   if (Motor_X_Pos < (Limit_Pos_Front - (ACCEL * REPEAt) * 2))
   {
      Ks_Divisor = Thread_Info[Thread_Step].Ks_Div_X;
      if (tmp_Ks_Divisor != Ks_Divisor)
      {
         tmp_Accel = ACCEL + Ks_Divisor;
         tmp_Ks_Divisor = ACCEL + Ks_Divisor;
      }   
      Brake_Compens = ACCEL * REPEAt + 1;
   }
   else
   {
      Ks_Divisor = ACCEL + Thread_Info[0].Ks_Div_X;
      tmp_Accel = Ks_Divisor;
      tmp_Ks_Divisor = Ks_Divisor;
      Brake_Compens = tmp_Accel - Ks_Divisor + 1;
   }
   
   Km_Divisor = Thread_Info[Thread_Step].Km_Div_X;
   Ks_Count = 0;
   Km_Count = 0;
   Limit_Pos = Limit_Pos_Front - Brake_Compens;
}


void Thread_Rear(bool & c_flag, bool & d_flag)
{
   if (d_flag == true) return;
   c_flag = false;
   d_flag = true;
  
   Motor_X_Enable();
   Motor_X_CCW();
   Motor_X_Dir = CCW;

   if (Motor_X_Pos > (Limit_Pos_Rear + (ACCEL * REPEAt) * 2))
   {
      Ks_Divisor = Thread_Info[Thread_Step].Ks_Div_X;
      if (tmp_Ks_Divisor != Ks_Divisor)
      {
         tmp_Accel = ACCEL + Ks_Divisor;
         tmp_Ks_Divisor = ACCEL + Ks_Divisor;
      }     
      Brake_Compens = ACCEL * REPEAt + 1;
   }
   else
   {
      Ks_Divisor = ACCEL + Thread_Info[0].Ks_Div_X;
      tmp_Accel = Ks_Divisor;
      tmp_Ks_Divisor = Ks_Divisor;
      Brake_Compens = tmp_Accel - Ks_Divisor + 1;
   }

    Km_Divisor = Thread_Info[Thread_Step].Km_Div_X;
    Ks_Count = 0;
    Km_Count = 0;
    Limit_Pos = Limit_Pos_Rear + Brake_Compens;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Mode "Cycle Thread" ********** /////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Translated by Google from http://www.chipmaker.ru/topic/97701/page__st__4400
 * On the formulas, they are all about the area of ​​an equilateral triangle.
(In the case of inch threads, correction is needed, there is an isosceles triangle)
S = H ^ 2 * (1 / sqrt (3)) is the area through the height (the height is the sinking)
S = A ^ 2 * sqrt (3) / 4 is the area across the side (side = thread pitch)
H = sqrt (sqrt (3) * S) - height through the area

The point is that the first pass specifies the depth parameter for the subsequent pass.
At the finish, two finishing passes are made for smoothing without penetration.
Depth ( depht ) for each pass is calculated so that the area of ​​the cut
Material (and hence the cutting force) was the same.

The initialization procedure takes into account the hardness of the material, but not to weight
The example in it does not take into account the thread pitch (should be done in the implementation).
So with a shallow step, the first penetration should be small.
All the constants in the expression {depht = 0.4 * 30.0 / hrc;} are called "from the lantern",
Those. Purely by experience, the first pass for a step of 1.5mm on a material of hardness of 30 units.
I would have done to a depth of 0.4mm, but for step 0.5 no more than 0.2.
That's what the correction for the thread pitch is for.
For large steps, correction should be vanishingly small (this is easy).

Correction, in order to turn the "theoretical" thread into a "practical" thread, you need
1. Add: #define CORRECTION 0.866025404
2. Edit: endDepht = (thread * SQRT_3 / 2) * CORRECTION;

The thread cutting algorythm is done in accordance to GOST 19258-73
 */
 
void Thread_Ext_Left()
{  
   if ((Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Rear) ||
       (Motor_X_Pos == Null_X_Pos && Motor_Z_Pos <= Limit_Pos_Right + McSTEP_Z/2) && (Motor_Z_Pos >= Limit_Pos_Right - McSTEP_Z/2))
   {  
      Pass_Total = Thread_Info[Thread_Step].Pass;
      if (cycle_flag == false && Pass_Nr <= Pass_Total)
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Step_mm = Thread_Info[Thread_Step].Step;
         if(Pass_Nr == 1) {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 6) - (Step_mm * 0.866 / 8)) / sqrt(Pass_Total-1) * sqrt(0.3);}  //Instead of using a thread pass table, the program uses this formula, see comment above
         else {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 6) - (Step_mm * 0.866 / 8)) / sqrt(Pass_Total-1) * sqrt(Pass_Nr-1);}
         Infeed_Value = long(Pass_Depth / (SCREW_X / MOTOR_X_STEP_PER_REV) + 0.5) * McSTEP_X;
         Limit_Pos_Front = (Null_X_Pos + Infeed_Value);
         Limit_Front_LED_On();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Pass_Nr++;
         Thread_Front(c_flag, d_flag);
         bbeep_flag = ON;
      }

      else if ((cycle_flag == false) && (Pass_Nr > Pass_Total && Pass_Nr <= Pass_Total + PASS_FINISH))
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Front(c_flag, d_flag);
         Pass_Nr++;
      }
      
      else if (cycle_flag == false && Pass_Nr > Pass_Total + PASS_FINISH)
      {
         c_flag = false;
         d_flag = false;
         Limit_Pos_Front = Null_X_Pos;
         Limit_Front_LED_Off();
         Limit_Rear_LED_Off();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Front(c_flag, d_flag);
         bbeep_flag = OFF;
      }
   }

   else if ((Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Front) ||
            (Motor_X_Pos == Limit_Pos_Front && Motor_Z_Pos <= Limit_Pos_Right + McSTEP_Z/2) && (Motor_Z_Pos >= Limit_Pos_Right - McSTEP_Z/2))
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Left(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Front)
   {
      cycle_flag = false;
      c_flag = false;
      d_flag = false;
      Limit_Pos_Rear = (Null_X_Pos - REBOUND_X);
      Limit_Rear_LED_On();
      Joy_Z_flag = OFF;
      Joy_X_flag = ON;
      Thread_Rear(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Rear)
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Right(c_flag, d_flag);
   } 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Thread_Ext_Right()  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
   if ((Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Rear) ||
       (Motor_X_Pos == Null_X_Pos && Motor_Z_Pos <= Limit_Pos_Left + McSTEP_Z/2) && (Motor_Z_Pos >= Limit_Pos_Left - McSTEP_Z/2))
   {  
      Pass_Total = Thread_Info[Thread_Step].Pass;
      if (cycle_flag == false && Pass_Nr <= Pass_Total)
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Step_mm = Thread_Info[Thread_Step].Step;
         if(Pass_Nr == 1) {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 6) - (Step_mm * 0.866 / 8)) / sqrt(Pass_Total-1) * sqrt(0.3);}
         else {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 6) - (Step_mm * 0.866 / 8)) / sqrt(Pass_Total-1) * sqrt(Pass_Nr-1);}
         Infeed_Value = long(Pass_Depth / (SCREW_X / MOTOR_X_STEP_PER_REV) + 0.5) * McSTEP_X;
         Limit_Pos_Front = (Null_X_Pos + Infeed_Value);
         Limit_Front_LED_On();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Pass_Nr++;
         Thread_Front(c_flag, d_flag);
         bbeep_flag = ON;
      }

      else if ((cycle_flag == false) && (Pass_Nr > Pass_Total && Pass_Nr <= Pass_Total + PASS_FINISH))
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Front(c_flag, d_flag);
         Pass_Nr++;
      }
      
      else if (cycle_flag == false && Pass_Nr > Pass_Total + PASS_FINISH)
      {
         c_flag = false;
         d_flag = false;
         Limit_Pos_Front = Null_X_Pos;
         Limit_Front_LED_Off();
         Limit_Rear_LED_Off();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Front(c_flag, d_flag);
         bbeep_flag = OFF;
      }
   }

   else if ((Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Front) ||
            (Motor_X_Pos == Limit_Pos_Front && Motor_Z_Pos <= Limit_Pos_Left + McSTEP_Z/2) && (Motor_Z_Pos >= Limit_Pos_Left - McSTEP_Z/2))
   
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Right(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Front)
   {
      cycle_flag = false;
      c_flag = false;
      d_flag = false;
      Limit_Pos_Rear = (Null_X_Pos - REBOUND_X);
      Limit_Rear_LED_On();
      Joy_Z_flag = OFF;
      Joy_X_flag = ON;
      Thread_Rear(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Rear)
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Left(c_flag, d_flag);
   }   
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Thread_Int_Left()  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
   if ((Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Front) ||
       (Motor_X_Pos == Null_X_Pos && Motor_Z_Pos <= Limit_Pos_Right + McSTEP_X/2) && (Motor_Z_Pos >= Limit_Pos_Right - McSTEP_Z/2))
   {  
      Pass_Total = Thread_Info[Thread_Step].Pass;
      if (cycle_flag == false && Pass_Nr <= Pass_Total)
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Step_mm = Thread_Info[Thread_Step].Step;
         if(Pass_Nr == 1) {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 4) - (Step_mm * 0.866 / 18)) / sqrt(Pass_Total-1) * sqrt(0.3);}
         else {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 4) - (Step_mm * 0.866 / 18)) / sqrt(Pass_Total-1) * sqrt(Pass_Nr-1);}
         Infeed_Value = long(Pass_Depth / (SCREW_X / MOTOR_X_STEP_PER_REV) + 0.5) * McSTEP_X;
         Limit_Pos_Rear = (Null_X_Pos - Infeed_Value);
         Limit_Rear_LED_On();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Pass_Nr++;
         Thread_Rear(c_flag, d_flag);
         bbeep_flag = ON;
      }

      else if ((cycle_flag == false) && (Pass_Nr > Pass_Total && Pass_Nr <= Pass_Total + PASS_FINISH))
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Rear(c_flag, d_flag);
         Pass_Nr++;
      }
      
      else if (cycle_flag == false && Pass_Nr > Pass_Total + PASS_FINISH)
      {
         c_flag = false;
         d_flag = false;
         Limit_Pos_Rear = Null_X_Pos;
         Limit_Front_LED_Off();
         Limit_Rear_LED_Off();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Rear(c_flag, d_flag);
         bbeep_flag = OFF;
      }
   }

   else if ((Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Rear) ||
            (Motor_X_Pos == Limit_Pos_Rear && Motor_Z_Pos <= Limit_Pos_Right + McSTEP_X/2) && (Motor_Z_Pos >= Limit_Pos_Right - McSTEP_Z/2))
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Left(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Rear)
   {
      cycle_flag = false;
      c_flag = false;
      d_flag = false;
      Limit_Pos_Front = (Null_X_Pos + REBOUND_X);
      Limit_Front_LED_On();
      Joy_Z_flag = OFF;
      Joy_X_flag = ON;
      Thread_Front(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Front)
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Right(c_flag, d_flag);
   } 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Thread_Int_Right()  //////////////////////////////////////////////////////////////////////////////////////////////////////////
{
   if ((Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Front) ||
       (Motor_X_Pos == Null_X_Pos && Motor_Z_Pos <= Limit_Pos_Left + McSTEP_Z/2) && (Motor_Z_Pos >= Limit_Pos_Left - McSTEP_Z/2))
   {  
      Pass_Total = Thread_Info[Thread_Step].Pass ;
      if (cycle_flag == false && Pass_Nr <= Pass_Total)
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Step_mm = Thread_Info[Thread_Step].Step;
         if(Pass_Nr == 1) {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 4) - (Step_mm * 0.866 / 18)) / sqrt(Pass_Total-1) * sqrt(0.3);}
         else {Pass_Depth = ((Step_mm * 0.866) - (Step_mm * 0.866 / 4) - (Step_mm * 0.866 / 18)) / sqrt(Pass_Total-1) * sqrt(Pass_Nr-1);}
         Infeed_Value = long(Pass_Depth / (SCREW_X / MOTOR_X_STEP_PER_REV) + 0.5) * McSTEP_X;
         Limit_Pos_Rear = (Null_X_Pos - Infeed_Value);
         Limit_Rear_LED_On();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Pass_Nr++;
         Thread_Rear(c_flag, d_flag);
         bbeep_flag = ON;
      }

      else if ((cycle_flag == false) && (Pass_Nr > Pass_Total && Pass_Nr <= Pass_Total + PASS_FINISH))
      {
         cycle_flag = true;
         c_flag = false;
         d_flag = false;
         
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Rear(c_flag, d_flag);
         Pass_Nr++;
      }
      
      else if (cycle_flag == false && Pass_Nr > Pass_Total + PASS_FINISH)
      {
         c_flag = false;
         d_flag = false;
         Limit_Pos_Rear = Null_X_Pos;
         Limit_Front_LED_Off();
         Limit_Rear_LED_Off();
         Joy_Z_flag = OFF;
         Joy_X_flag = ON;
         Thread_Rear(c_flag, d_flag);
         bbeep_flag = OFF;
      }
   }

   else if ((Motor_Z_Pos == Limit_Pos_Left && Motor_X_Pos == Limit_Pos_Rear) ||
            (Motor_X_Pos == Limit_Pos_Rear && Motor_Z_Pos <= Limit_Pos_Left + McSTEP_Z/2) && (Motor_Z_Pos >= Limit_Pos_Left - McSTEP_Z/2))
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Right(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Rear)
   {
      cycle_flag = false;
      c_flag = false;
      d_flag = false;
      Limit_Pos_Front = (Null_X_Pos + REBOUND_X);
      Limit_Front_LED_On();
      Joy_Z_flag = OFF;
      Joy_X_flag = ON;
      Thread_Front(c_flag, d_flag);
   }

   else if (Motor_Z_Pos == Limit_Pos_Right && Motor_X_Pos == Limit_Pos_Front)
   {
      c_flag = false;
      d_flag = false;
      Joy_Z_flag = ON;
      Joy_X_flag = OFF;
      Thread_Left(c_flag, d_flag);
   } 
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Feed Mode ********** //////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Feed_Left(bool & a_flag, bool & b_flag)
{
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CW();
   Motor_Z_Dir = CW;
   Feed_Z_flag = ON;
   
   if (Motor_Z_Pos < Limit_Pos_Left - Feed_Info[0].Div_Z * 2)
   {
      Ks_Divisor = Feed_Info[Feed_Step].Div_Z;
      if (Ks_Divisor < Start_Speed)
      {
         max_OCR5A = Start_Speed;
         OCR5A = Start_Speed;
      }
      else
      {
         max_OCR5A = Ks_Divisor;
         OCR5A = Ks_Divisor;
      }
   }
   else
   {
      Ks_Divisor = Feed_Info[0].Div_Z;
      max_OCR5A = Feed_Info[0].Div_Z;
      OCR5A = Feed_Info[0].Div_Z;
   }
   
   Brake_Compens = max_OCR5A - Ks_Divisor + 1;
   Limit_Pos = Limit_Pos_Left - Brake_Compens;
   
   Ena_INT_Z_Feed();
}

void Feed_Right(bool & a_flag, bool & b_flag)
{
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CCW();
   Motor_Z_Dir = CCW;
   Feed_Z_flag = ON;
   
   if (Motor_Z_Pos > Limit_Pos_Right + Feed_Info[0].Div_Z * 2)
   {
      Ks_Divisor = Feed_Info[Feed_Step].Div_Z;
      if (Ks_Divisor < Start_Speed)
      {
         max_OCR5A = Start_Speed;
         OCR5A = Start_Speed;
      }
      else
      {
         max_OCR5A = Ks_Divisor;
         OCR5A = Ks_Divisor;
      }
   }
   else
   {
      Ks_Divisor = Feed_Info[0].Div_Z;
      max_OCR5A = Feed_Info[0].Div_Z;
      OCR5A = Feed_Info[0].Div_Z;
   }
   
   Brake_Compens = max_OCR5A - Ks_Divisor + 1;
   Limit_Pos = Limit_Pos_Right + Brake_Compens;

   Ena_INT_Z_Feed();
}

void Feed_Front(bool & a_flag, bool & b_flag)
{  
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;
   
   Motor_X_Enable();
   Motor_X_CW();
   Motor_X_Dir = CW;
   Feed_X_flag = ON;
   
   if (Motor_X_Pos < Limit_Pos_Front - Feed_Info[0].Div_X * 2)
   {
      Ks_Divisor = Feed_Info[Feed_Step].Div_X;
      if (Ks_Divisor < Start_Speed)
      {
         max_OCR5A = Start_Speed;
         OCR5A = Start_Speed;
      }
      else
      {
         max_OCR5A = Ks_Divisor;
         OCR5A = Ks_Divisor;
      }
   }
   else
   {
      Ks_Divisor = Feed_Info[0].Div_X;
      max_OCR5A = Feed_Info[0].Div_X;
      OCR5A = Feed_Info[0].Div_X;
   }
   
   Brake_Compens = max_OCR5A - Ks_Divisor + 1;
   Limit_Pos = Limit_Pos_Front - Brake_Compens;

   Ena_INT_X_Feed();
}

void Feed_Rear(bool & a_flag, bool & b_flag)
{
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;
   
   Motor_X_Enable();
   Motor_X_CCW();
   Motor_X_Dir = CCW;
   Feed_X_flag = ON;
   
   if (Motor_X_Pos > Limit_Pos_Rear + Feed_Info[0].Div_X * 2)
   {
      Ks_Divisor = Feed_Info[Feed_Step].Div_X;
      if (Ks_Divisor < Start_Speed)
      {
         max_OCR5A = Start_Speed;
         OCR5A = Start_Speed;
      }
      else
      {
         max_OCR5A = Ks_Divisor;
         OCR5A = Ks_Divisor;
      }
   }
   else
   {
      Ks_Divisor = Feed_Info[0].Div_X;
      max_OCR5A = Feed_Info[0].Div_X;
      OCR5A = Feed_Info[0].Div_X;
   }

   Brake_Compens = max_OCR5A - Ks_Divisor + 1;
   Limit_Pos = Limit_Pos_Rear + Brake_Compens;

   Ena_INT_X_Feed();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Mode "Cone" ********** //////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Cone_Left(bool & a_flag, bool & b_flag)
{  
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CW();
   Motor_Z_Dir = CW;
   Motor_X_Enable();
   
   if (Mode == Mode_Cone_L)
   {
      Motor_X_CW();
      Motor_X_Dir = CW;
   }
   else if (Mode == Mode_Cone_R)
   {
      Motor_X_CCW();
      Motor_X_Dir = CCW;
   }
   
   Feed_Z_flag = ON;
   Step_X_flag = ON;
   
   Cs_Divisor = Cone_Info[Cone_Step].Cs_Div;
   Cm_Divisor = Cone_Info[Cone_Step].Cm_Div;
   Cs_Count = 0;
   Cm_Count = 0;
   
   if (Motor_Z_Pos < Limit_Pos_Left - Feed_Info[0].Div_Z * 2)
   {
      Ks_Divisor = Feed_Info[Feed_Step].Div_Z;
      if (Ks_Divisor < Start_Speed)
      {
         max_OCR5A = Start_Speed;
         OCR5A = Start_Speed;
      }
      else
      {
         max_OCR5A = Ks_Divisor;
         OCR5A = Ks_Divisor;
      }
   }
   else
   {
      Ks_Divisor = Feed_Info[0].Div_Z;
      max_OCR5A = Ks_Divisor;
      OCR5A = Ks_Divisor;
   }
   
   Brake_Compens = max_OCR5A - Ks_Divisor + 1;
   Limit_Pos = Limit_Pos_Left - Brake_Compens;

   Ena_INT_Z_Feed();
}

void Cone_Right(bool & a_flag, bool & b_flag)
{
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true; 
   
   Motor_Z_Enable();
   Motor_Z_CCW();
   Motor_Z_Dir = CCW;
   Motor_X_Enable();
   
   if (Mode == Mode_Cone_L)
   {
      Motor_X_CCW();
      Motor_X_Dir = CCW;
   }
   else if (Mode == Mode_Cone_R)
   {
      Motor_X_CW();
      Motor_X_Dir = CW;
   }
   
   Feed_Z_flag = ON;
   Step_X_flag = ON;
   
   Cs_Divisor = Cone_Info[Cone_Step].Cs_Div;
   Cm_Divisor = Cone_Info[Cone_Step].Cm_Div;
   Cs_Count = 0;
   Cm_Count = 0;
   
   if (Motor_Z_Pos > Limit_Pos_Right + Feed_Info[0].Div_Z * 2)
   {
      Ks_Divisor = Feed_Info[Feed_Step].Div_Z;
      if (Ks_Divisor < Start_Speed)
      {
         max_OCR5A = Start_Speed;
         OCR5A = Start_Speed;
      }
      else
      {
         max_OCR5A = Ks_Divisor;
         OCR5A = Ks_Divisor;
      }
   }
   else
   {
      Ks_Divisor = Feed_Info[0].Div_Z;
      max_OCR5A = Ks_Divisor;
      OCR5A = Ks_Divisor;
   }
   
   Brake_Compens = max_OCR5A - Ks_Divisor + 1;
   Limit_Pos = Limit_Pos_Right + Brake_Compens;

   Ena_INT_Z_Feed();
}

void Cone_Front(bool & a_flag, bool & b_flag)
{
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;
}

void Cone_Rear(bool & a_flag, bool & b_flag)
{
   if (b_flag == true) return;
   a_flag = false;
   b_flag = true;  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Fast Feed Mode ********** //////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Rapid_Feed_Left(bool & a_flag, bool & b_flag)
{
   if (a_flag == true) return;
   b_flag = false;
   a_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CW();
   Motor_Z_Dir = CW;
   
   Brake_Compens = (MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt / 2 + 1;
   Limit_Pos = Limit_Pos_Left - Brake_Compens;
                
   Rapid_Z_flag = ON;
   Ena_INT_Z_Rapid();
}

void Rapid_Feed_Right(bool & a_flag, bool & b_flag)
{
   if (a_flag == true) return;
   b_flag = false;
   a_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CCW();
   Motor_Z_Dir = CCW;
   
   Brake_Compens = (MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt / 2 + 1;
   Limit_Pos = Limit_Pos_Right + Brake_Compens;
                
   Rapid_Z_flag = ON;
   Ena_INT_Z_Rapid();  
}

void Rapid_Feed_Front(bool & a_flag, bool & b_flag)
{
   if (a_flag == true) return;
   b_flag = false;
   a_flag = true;
   
   Motor_X_Enable();
   Motor_X_CW();
   Motor_X_Dir = CW;
   
   Brake_Compens = (MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt / 2 + 1;
   Limit_Pos = Limit_Pos_Front - Brake_Compens;
               
   Rapid_X_flag = ON;
   Ena_INT_X_Rapid();
}

void Rapid_Feed_Rear(bool & a_flag, bool & b_flag)
{
   if (a_flag == true) return;
   b_flag = false;
   a_flag = true;
   
   Motor_X_Enable();
   Motor_X_CCW();
   Motor_X_Dir = CCW;
   
   Brake_Compens = (MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt / 2 + 1;
   Limit_Pos = Limit_Pos_Rear + Brake_Compens;
               
   Rapid_X_flag = ON;
   Ena_INT_X_Rapid();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ********** Mode "Cone! Fast Feed" ********** ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Rapid_Cone_Left(bool & a_flag, bool & b_flag)
{
   if (a_flag == true) return;
   b_flag = false;
   a_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CW();
   Motor_Z_Dir = CW;
   Motor_X_Enable();
   
   if (Mode == Mode_Cone_L)
   {
      Motor_X_CW();
      Motor_X_Dir = CW;
   }
   else if (Mode == Mode_Cone_R)
   {
      Motor_X_CCW();
      Motor_X_Dir = CCW;
   }
   
   Brake_Compens = (MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt / 2 + 1;
   Limit_Pos = Limit_Pos_Left - Brake_Compens;
   
   Cs_Count = 0;
   Cm_Count = 0;            
   Rapid_Z_flag = ON;
   Step_X_flag = ON;
   Ena_INT_Z_Rapid();
}

void Rapid_Cone_Right(bool & a_flag, bool & b_flag)
{
   if (a_flag == true) return;
   b_flag = false;
   a_flag = true;
   
   Motor_Z_Enable();
   Motor_Z_CCW();
   Motor_Z_Dir = CCW;
   Motor_X_Enable();
   
   if (Mode == Mode_Cone_L)
   {
      Motor_X_CCW();
      Motor_X_Dir = CCW;
   }
   else if (Mode == Mode_Cone_R)
   {
      Motor_X_CW();
      Motor_X_Dir = CW;
   }
   
   Brake_Compens = (MIN_RAPID_MOTION - MAX_RAPID_MOTION) * REPEAt / 2 + 1;
   Limit_Pos = Limit_Pos_Right + Brake_Compens;
   
   Cs_Count = 0;
   Cm_Count = 0;            
   Rapid_Z_flag = ON;
   Step_X_flag = ON;
   Ena_INT_Z_Rapid();  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ***** Print ***** /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Print()
{
   if (Mode == Mode_Feed)
   { 
      snprintf(LCD_Row_1, 17, "Feed:     %s", Feed_Info[Feed_Step].Feed_Print);
      snprintf(LCD_Row_2, 17, "Max:     %s", Feed_Info[Feed_Step].Limit_Print);
   }
   
   else if (Mode == Mode_Cone_L)
   {
      snprintf(LCD_Row_1, 17, "Feed:     %s", Feed_Info[Feed_Step].Feed_Print);
      snprintf(LCD_Row_2, 17, "Cone:     <%s", Cone_Info[Cone_Step].Cone_Print);
   }
   
   else if (Mode == Mode_Cone_R)
   {
      snprintf(LCD_Row_1, 17, "Feed:     %s", Feed_Info[Feed_Step].Feed_Print);
      snprintf(LCD_Row_2, 17, "Cone:     >%s", Cone_Info[Cone_Step].Cone_Print);
   } 
   
   else if (Mode == Mode_Thread)
   {
      if (Sub_Mode_Thread == Sub_Mode_Thread_Int)
      {
         snprintf(LCD_Row_1, 17, "Thrd: Int %s", Thread_Info[Thread_Step].Thread_Print);
      }
      else if (Sub_Mode_Thread == Sub_Mode_Thread_Man)
      {
         snprintf(LCD_Row_1, 17, "Thrd: Man %s", Thread_Info[Thread_Step].Thread_Print);
      }
      else if (Sub_Mode_Thread == Sub_Mode_Thread_Ext)
      {
         snprintf(LCD_Row_1, 17, "Thrd: Ext %s", Thread_Info[Thread_Step].Thread_Print);
      }
      snprintf(LCD_Row_2, 17, "Max:     %s", Thread_Info[Thread_Step].Limit_Print);
   }  
   
   else if (Mode == Mode_Divider)
   { 
      long Spindle_Angle = Enc_Pos * 36000 / ENC_TICK;
      long Required_Angle = 36000 * (Current_Tooth - 1) / Total_Tooth;
      snprintf(LCD_Row_1, 17, "Req:%3ld.%02ld z:%3d", Required_Angle/100, Required_Angle%100, Total_Tooth);
      snprintf(LCD_Row_2, 17, "Rea:%3ld.%02ld a:%3d", Spindle_Angle/100, Spindle_Angle%100, Current_Tooth);
   }
 
   lcd.setCursor(0, 0);
   lcd.print(LCD_Row_1);
   lcd.print("    ");

   lcd.setCursor(0, 1);
   lcd.print(LCD_Row_2);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// *****  Thread ***** ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR(INT0_vect)  //If ther is a change in PIN 21, the Encoder step either up or down then
{
   TachoRemovePulse();
   Motor_Z_RemovePulse();
   Motor_X_RemovePulse();
   
   if (!Enc_Read)   // If the Encoder read is 0 on PIN 20 then the spindle goes in a CW direction
   {
      Spindle_Dir = CW;
      if (++Enc_Pos == ENC_TICK)   //If the Enc_Pos is greater than the number of lines for the encoder - output a pulse to the tachometer and reset the Enc_Pos
      {                                           
         Enc_Pos = 0;
         TachoSetPulse();
         if (Joy_Z_flag == ON) {Step_Z_flag = ON;}
         else if (Joy_X_flag == ON) {Step_X_flag = ON;}
      }
   } 
   else
   {
      Spindle_Dir = CCW;  // If the Encoder read is 1 on PIN 21 then the spindle goes in a CCW direcetion
      if (--Enc_Pos < 0)
      { 
         Enc_Pos = ENC_TICK - 1;
         TachoSetPulse();
         if (Joy_Z_flag == ON) {Step_Z_flag = ON;}
         else if (Joy_X_flag == ON) {Step_X_flag = ON;}
      }
   }
   
   //////////////////////////////////////////// Longitudinal, can be broken down into INT0 INT1
   
   if (Step_Z_flag == ON)
   {
      if ( (Motor_Z_Dir == CW && Motor_Z_Pos > Limit_Pos) || (Motor_Z_Dir == CCW && Motor_Z_Pos < Limit_Pos) || (!Joy_Z_flag) )
      {
         if (tmp_Ks_Divisor < tmp_Accel)
         {
            Ks_Count++;
            if (Ks_Count > tmp_Ks_Divisor)
            {
               Motor_Z_SetPulse();
               if (Motor_Z_Dir == CW) {Motor_Z_Pos ++;}
               else {Motor_Z_Pos --;}
               Ks_Count = 0;
               if (++Repeat_Count == REPEAt)
               {
                  Repeat_Count = 0;
                  tmp_Ks_Divisor ++;
               }
            }
         }  
         else {Step_Z_flag = OFF;}
      }

      else
      {
         Ks_Count++;
         if (Ks_Count > tmp_Ks_Divisor)
         {
            Motor_Z_SetPulse();
            if (Motor_Z_Dir == CW) {Motor_Z_Pos ++;}
            else {Motor_Z_Pos --;}
         
            if (tmp_Ks_Divisor > Ks_Divisor)
            {
               Ks_Count = 0;
               if (++Repeat_Count == REPEAt)
               {
                  Repeat_Count = 0;
                  tmp_Ks_Divisor --;
               }
            }
            else
            {  
               Km_Count = Km_Count + Km_Divisor;
               if (Km_Count > Km_Divisor)
               {
                  Km_Count = Km_Count - 10000;
                  Ks_Count = 0;
               }
               else {Ks_Count = 1;}
            }
         }
      }
   }
   
   /////////////////////////////////////////////////////////////  Transverse, can be divided into INT0 INT1

   if (Step_X_flag == ON)
   {
      if ( (Motor_X_Dir == CW && Motor_X_Pos > Limit_Pos) || (Motor_X_Dir == CCW && Motor_X_Pos < Limit_Pos) || (!Joy_X_flag) )
      {
         if (tmp_Ks_Divisor < tmp_Accel)
         {
            Ks_Count++;
            if (Ks_Count > tmp_Ks_Divisor)
            {
               Motor_X_SetPulse();
               if (Motor_X_Dir == CW) {Motor_X_Pos ++;}
               else {Motor_X_Pos --;}
               Ks_Count = 0;
               if (++Repeat_Count == REPEAt)
               {
                  Repeat_Count = 0;
                  tmp_Ks_Divisor ++;
               }
            }
         }  
         else {Step_X_flag = OFF;}
      }

      else
      {
         Ks_Count++;
         if (Ks_Count > tmp_Ks_Divisor)
         {
            Motor_X_SetPulse();
            if (Motor_X_Dir == CW) {Motor_X_Pos ++;}
            else {Motor_X_Pos --;}
         
            if (tmp_Ks_Divisor > Ks_Divisor)
            {
               Ks_Count = 0;
               if (++Repeat_Count == REPEAt)
               {
                  Repeat_Count = 0;
                  tmp_Ks_Divisor --;
               }
            }
            else
            {  
               Km_Count = Km_Count + Km_Divisor;
               if (Km_Count > Km_Divisor)
               {
                  Km_Count = Km_Count - 10000;
                  Ks_Count = 0;
               }
               else {Ks_Count = 1;}
            }
         }
      }
   }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// *****  Feed & Cone ***** //////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR (TIMER5_COMPA_vect)
{
   TachoRemovePulse();
   Motor_Z_RemovePulse();
   Motor_X_RemovePulse();
   Tacho_Count = Tacho_Count + (OCR5A+1);
   if (Tacho_Count > ENC_LINE_PER_REV)
   {
      TachoSetPulse();
      Tacho_Count = Tacho_Count - ENC_LINE_PER_REV;
   }
   
   if ( (Motor_Z_Dir == CW && Motor_Z_Pos > Limit_Pos) || (Motor_Z_Dir == CCW && Motor_Z_Pos < Limit_Pos) || (!Feed_Z_flag) )
   {
      if (OCR5A < max_OCR5A)
      {
         OCR5A++;
         Motor_Z_SetPulse();
         if (Motor_Z_Dir == CW) {Motor_Z_Pos ++;}
         else {Motor_Z_Pos --;}
      }
      else
      {             
         Step_Z_flag = OFF;
         Step_X_flag = OFF;
      }
   }
   
   else
   {
      Step_Z_flag = ON;
      Motor_Z_SetPulse();
      if (Motor_Z_Dir == CW) {Motor_Z_Pos ++;}
      else {Motor_Z_Pos --;}
      
      if (OCR5A > Ks_Divisor) {OCR5A--;}
      else if (OCR5A < Ks_Divisor) {OCR5A ++;}
   }
   
   ///////////////////////////////////////////////////////
   if (Step_X_flag == ON)
   {
      if (++Cs_Count > Cs_Divisor)
      {
         Motor_X_SetPulse();
         if (Motor_X_Dir == CW) {Motor_X_Pos ++;}
         else {Motor_X_Pos --;}
  
         Cm_Count = Cm_Count + Cm_Divisor;
         if (Cm_Count > Cm_Divisor)
         {
            Cm_Count = Cm_Count - 10000;
            Cs_Count = 0;
         }
         else {Cs_Count = 1;}
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR (TIMER5_COMPB_vect)
{
   TachoRemovePulse();
   Motor_X_RemovePulse();
   
   Tacho_Count = Tacho_Count + (OCR5A+1);
   if (Tacho_Count > ENC_LINE_PER_REV)
   {
      TachoSetPulse();
      Tacho_Count = Tacho_Count - ENC_LINE_PER_REV;
   }
  
   if ( (Motor_X_Dir == CW && Motor_X_Pos > Limit_Pos) || (Motor_X_Dir == CCW && Motor_X_Pos < Limit_Pos) || (!Feed_X_flag) )
   {
      if (OCR5A < max_OCR5A)
      {
         OCR5A++;
         Motor_X_SetPulse();
         if (Motor_X_Dir == CW) {Motor_X_Pos ++;}
         else {Motor_X_Pos --;}
      }
      else {Step_X_flag = OFF;}
   }
   
   else 
   {
      Step_X_flag = ON;
      Motor_X_SetPulse();
      if (Motor_X_Dir == CW) {Motor_X_Pos ++;}
      else {Motor_X_Pos --;}
      
      if (OCR5A > Ks_Divisor) {OCR5A--;}
      else if (OCR5A < Ks_Divisor) {OCR5A ++;}
   }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ***** Rapid Feed & Rapid Cone ***** ///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR (TIMER2_COMPA_vect)         // Longitudinal
{
   if ( (Motor_Z_Dir == CW && Motor_Z_Pos > Limit_Pos) || (Motor_Z_Dir == CCW && Motor_Z_Pos < Limit_Pos) || (!Rapid_Z_flag) )
   {
      if (OCR2A < MIN_RAPID_MOTION)
      {
         Motor_Z_InvertPulse();
         if (!Read_Z_State)
         {
            if (Motor_Z_Dir == CW) { Motor_Z_Pos ++; }
            else { Motor_Z_Pos --; }
         }
         if (++Repeat_Count == REPEAt)
         {
            Repeat_Count = 0;
            OCR2A ++;
         }
      }
      else
      {
         Rapid_Step_Z_flag = OFF;
         Step_X_flag = OFF;
     }
   }
  
   else
   {
      Rapid_Step_Z_flag = ON;
      Motor_Z_InvertPulse();
      if (!Read_Z_State)
      {
         if (Motor_Z_Dir == CW) { Motor_Z_Pos ++; }
         else { Motor_Z_Pos --; }
      }

      if (OCR2A > MAX_RAPID_MOTION)
      {
         if (++Repeat_Count == REPEAt)
         {
            Repeat_Count = 0;
            OCR2A --;
         }
      }
   }
   
   ///////////////////////////////////////////////////////
   if (Step_X_flag == ON)
   {
      if (++Cs_Count > Cs_Divisor)
      {
         Motor_X_InvertPulse();
         if (!Read_X_State)
         {
            if (Motor_X_Dir == CW) { Motor_X_Pos ++; }
            else { Motor_X_Pos --; }
         }
  
         Cm_Count = Cm_Count + Cm_Divisor;
         if (Cm_Count > Cm_Divisor)
         {
            Cm_Count = Cm_Count - 10000;
            Cs_Count = 0;
         }
         else {Cs_Count = 1;}
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR (TIMER2_COMPB_vect)
{
   if ( (Motor_X_Dir == CW && Motor_X_Pos > Limit_Pos) || (Motor_X_Dir == CCW && Motor_X_Pos < Limit_Pos) || (!Rapid_X_flag) )
   {
      if (OCR2A < MIN_RAPID_MOTION)
      {
         Motor_X_InvertPulse();
         if (!Read_X_State)
         {
            if (Motor_X_Dir == CW) { Motor_X_Pos ++; }
            else { Motor_X_Pos --; }
         }

         if (++Repeat_Count == REPEAt)
         {
            Repeat_Count = 0;
            OCR2A ++;
         }
      }
      else  {Rapid_Step_X_flag = OFF;}
   }
  
   else
   {
      Rapid_Step_X_flag = ON;
      Motor_X_InvertPulse();
      if (!Read_X_State)
      {
         if (Motor_X_Dir == CW) { Motor_X_Pos ++; }
         else { Motor_X_Pos --; }
      }

      if (OCR2A > MAX_RAPID_MOTION)
      {
         if (++Repeat_Count == REPEAt)
         {
            Repeat_Count = 0;
            OCR2A --;
         }
      }
   }
}
// ***** End ***** ///////////////////////////////////////////////////
