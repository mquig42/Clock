/*******************************************************************************
 * QClock.c
 * 2015.11.04
 * Mike Quigley
 * 
 * Clock library for AVR, using timer1
 ******************************************************************************/

#include <QClock.h>
#include <util/atomic.h>

volatile uint32_t global_seconds = 0;       //Stores the current time, in seconds since midnight.
uint32_t local_seconds;                     //Stores a temporary copy of global_seconds, for HH:MM:SS calculations

//Timer interrupt is fired once per second.
//Increment seconds counter, reset to 0 at midnight.
ISR(TIMER1_COMPA_vect)
{
    global_seconds = (global_seconds + 1) % 86400;
}

//Set up timer1 to throw an interrupt once per second
void configureTimer()
{
    cli();
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = 62499 + timer_ofs;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

//Disable timer interrupt to stop clock from running
void stopTimer()
{
    TIMSK1 &= ~(1 << OCIE1A);
}

//Enable timer interrupt to start clock running
void startTimer()
{
    TIMSK1 |= (1 << OCIE1A);
}

//Increment clock by one hour, wrapping around at midnight
//Do not call when clock is running
void incHour()
{
    global_seconds = (global_seconds + 3600) % 86400;
}

//Increment clock by one minute, wrapping at 60 (eg 6:59 -> 6:00, not 7:00)
//Do not call when clock is running
void incMinute()
{
    global_seconds += 60;

    //wrap minutes
    if((global_seconds % 3600) / 60 == 0)
        global_seconds -= 3600;
}

//Resets the seconds component of the current time to 0, preserving hours and minutes
//Do not call when clock is running
void resetSeconds()
{
    global_seconds -= global_seconds % 60;
}

//Make a temp copy of global_seconds that won't be affected by interrupts
void copySeconds()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        local_seconds = global_seconds;
    }
}

//Returns current hour
unsigned char getHour()
{
    return local_seconds / 3600;
}

//Returns current minute
unsigned char getMinute()
{
    return (local_seconds % 3600) / 60;
}

//Returns current second
unsigned char getSecond()
{
    return local_seconds % 60;
}
