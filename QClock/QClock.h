/*******************************************************************************
 * QClock.h
 * 2015.11.04
 * Mike Quigley
 * 
 * Clock library for AVR, using timer1
 ******************************************************************************/

//Timer calibration adjustment.
//Enter a negative number if the clock runs too slow, or positive if it's too fast.
const int timer_ofs = -41;

void configureTimer();
void stopTimer();
void startTimer();
void incHour();
void incMinute();
void resetSeconds();
void copySeconds();
unsigned char getHour();
unsigned char getMinute();
unsigned char getSecond();