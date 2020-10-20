#include <Arduino.h>
#include "motion.h"
#include "config.h"


volatile float calculated_stepper_pulses=0;

void IRAM_ATTR calcDelta(){

  

  // calculate the current spindle position in motor steps.
  calculated_stepper_pulses = factor * spindlePos;

  // calculate the delta in motor steps between the current spindle position and the current motor position aka toolPos
  delta = toolPos - calculated_stepper_pulses;

  
  if (delta > 1){
    Serial.print(z_moving);
    Serial.print(",");
    Serial.print(z_pause);
    Serial.print(",");
    Serial.print(delay_ticks);
    Serial.print(",");
    Serial.println(delta);
  }
  

  // TODO: Calculate acceleration here, 
  // how much ramp is needed and when is it too much to stay in sync?

  /*
  if delta is increasing we should reduce the step delay

  if delta is decreasing we should increase the step delay

  if delta is somewhere close to 0 the step delay should stay the same.
  */
  
}

void init_motion(){
  
  // motor_steps = (microsteps * native_steps) /lead_screw_pitch;
  // factor is motor steps per spindle tick
  factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution); 
}