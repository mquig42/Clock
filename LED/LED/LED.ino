/*******************************************************************************
 * LCD.ino
 * 2015.10.24
 * Mike Quigley
 * 
 * Test for binary LED clock. LEDs are connected to pins 8 through 13. 
 * The select and set buttons for the clock are connected to digital pins
 * 6 and 7 using internal pullup resistors.
 ******************************************************************************/

#include <QClock.h>

enum ClockMode {SET_HOUR, SET_MINUTE, RUN};

bool select = false;
bool set = false;



void setup()
{
    pinMode(6,  INPUT_PULLUP);
    pinMode(7,  INPUT_PULLUP);
    pinMode(8,  OUTPUT);
    pinMode(9,  OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    configureTimer();
}

void loop()
{
    //Finite state machine
    static ClockMode m = RUN;
    switch(m)
    {
        case SET_HOUR:
            m = (ClockMode)setHour();
            break;
        case SET_MINUTE:
            m = (ClockMode)setMinute();
            break;
        case RUN:
            m = (ClockMode)checkSelectButton();
    }
    dispTime();
}

//Reads the current state of both buttons, and sets the global variables accordingly
//previous state of each button is stored, so we only get TRUE when a button is first pressed.
//If a button is held down, set its variable to TRUE once, then FALSE until it's released and pressed again.
void readButtons()
{
    static uint8_t select_p = false;
    static uint8_t set_p = false;
    uint8_t select_c = !digitalRead(6);
    uint8_t set_c = !digitalRead(7);

    select = (!select_p && select_c);
    set = (!set_p && set_c);

    select_p = select_c;
    set_p = set_c;
}

//This function is called during the RUN state.
//Transition to SET_HOUR state when select button is pressed
//ignore set button
uint8_t checkSelectButton()
{
    readButtons();
    
    if(select)
    {
        //Stop the timer while setting
        stopTimer();
        resetSeconds();
        return SET_HOUR;
    }
    return RUN;
}

//This function is called during the SET_HOUR state
//Transition to SET_MINUTE state when select button is pressed
//Increment by one hour when set button is pressed
uint8_t setHour()
{
    readButtons();

    if(select)
        return SET_MINUTE;
    if(set)
        incHour();
    
    return SET_HOUR;
}

//This function is called duting the SET_MINUTE state
//transition to RUN state when select button is pressed
//increment clock by one minute when set button is pressed
uint8_t setMinute()
{
    readButtons();

    if(select)
    {
        //Finished setting clock, so turn timer interrupt back on
        startTimer();
        //Timer interrupts may be queued, causing the clock to skip ahead
        //by a couple of seconds. Reset again to prevent this.
        resetSeconds();
        return RUN;
    }
    if(set)
        incMinute();

    return SET_MINUTE;
}

//Display time on the first line of the LCD in HH:MM:SS format
void dispTime()
{
    copySeconds();
    uint8_t hours = getHour();
    uint8_t minutes = getMinute();
    uint8_t secs = getSecond();
    
    //Print time in HH:MM:SS format
    PORTB = secs;
}
