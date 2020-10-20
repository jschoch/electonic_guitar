#ifndef _Controls_h
#define _Controls_h

#include <Arduino.h>
#include "config.h"

//#include <neotimer.h>

// btn pins


// Left Button
#define LBP 15
// Right Button
#define RBP 2
// Menu switch button
#define SBP 14
// Up Button
#define UBP 16
// Down Button
#define DBP 4 // SVN



void init_controls(void);
void read_buttons(void);

void handleLBP(void);
void handleRBP(void);
void handleSBP(void);
void handleUBP(void);
void handleDBP(void);
void setFactor(void);
void thread_parameters(void);
void feed_parameters(void);
#endif
