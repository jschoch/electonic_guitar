#include <Arduino.h>
#include "motion.h"
#include "config.h"


volatile float calculated_stepper_pulses=0;

void IRAM_ATTR calcDelta(){
  calculated_stepper_pulses = factor * spindlePos;
  delta = toolPos - calculated_stepper_pulses;
}

void init_motion(){
  //initial factor when turning on the ELS to deliver a turning operation of "medium" pitch
  factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution); 
}