/*******************************************************************************
 * LCD.ino
 * 2015.10.24
 * Mike Quigley
 * 
 * Digital clock using the Hitachi LCD screen, connected to the same pins as
 * the other LCD examples. The select and set buttons for the clock are
 * connected to digital pins 9 and 10 using internal pullup resistors.
 ******************************************************************************/

#include <LiquidCrystal.h>
#include <QClock.h>

enum ClockMode {SET_HOUR, SET_MINUTE, RUN};

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
bool select = false;
bool set = false;



void setup()
{
    lcd.begin(16, 2);
    pinMode(9, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);
    pinMode(13, OUTPUT);
    digitalWrite(13, 0);
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
    uint8_t select_c = !digitalRead(9);
    uint8_t set_c = !digitalRead(10);

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
    lcd.setCursor(0, 0);
    if(hours<10)
        lcd.print("0");
    lcd.print(hours);
    lcd.print(":");
    if(minutes<10)
        lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");
    if(secs<10)
        lcd.print("0");
    lcd.print(secs);
}
