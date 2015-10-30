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
#include <util/atomic.h>

enum ClockMode {SET_HOUR, SET_MINUTE, RUN};

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
volatile uint32_t global_seconds = 0;
bool select = false;
bool set = false;

//Timer interrupt is fired once per second.
//Increment seconds counter, toggle pin 13 LED, reset to 0 at midnight.
ISR(TIMER1_COMPA_vect)
{
    global_seconds++;
    digitalWrite(13, !digitalRead(13));
    //Reset to 0 after 24 hours
    if(global_seconds >= 86400)
        global_seconds = 0;
}

void setup()
{
    lcd.begin(16, 2);
    pinMode(9, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);
    pinMode(13, OUTPUT);
    configureTimer();
}

//Configure timer1 to fire an interrupt once every second
void configureTimer()
{
    noInterrupts();
    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = 62499;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
    interrupts();
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

    if(!select_p && select_c)
        select = true;
    else
        select = false;
    
    if(!set_p && set_c)
        set = true;
    else
        set = false;

    select_p = select_c;
    set_p = set_c;
}

//This function is called during the RUN state.
//Transition to SET_HOUR state when select button is pressed
//ignore set button
uint8_t checkSelectButton()
{
    readButtons();
    
    //We're using a pullup, so buttons are 0 when pressed
    if(select)
    {
        //Disable timer interrupt to pause clock when setting
        TIMSK1 &= ~(1 << OCIE1A);
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
    {
        global_seconds += 3600;
        
        //wrap hours at midnight
        if(global_seconds >= 86400)
            global_seconds -= 86400;
    }
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
        TIMSK1 |= (1 << OCIE1A);
        //Timer interrupts may be queued, causing the clock to skip ahead
        //by a couple of seconds. Reset again to prevent this.
        resetSeconds();
        return RUN;
    }
    if(set)
    {
        global_seconds += 60;

        //wrap minutes
        if((global_seconds % 3600) / 60 == 0)
            global_seconds -= 3600;
    }
    return SET_MINUTE;
}

//Display time on the first line of the LCD in HH:MM:SS format
void dispTime()
{
    uint32_t local_seconds;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        local_seconds = global_seconds;
    }
    uint8_t hours = local_seconds / 3600;
    uint8_t minutes = (local_seconds % 3600) / 60;
    uint8_t secs = local_seconds % 60;
    
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

//Resets the seconds portion of the current time to 0, while preserving hours and minutes.
void resetSeconds()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        uint8_t secs = global_seconds % 60;
        global_seconds -= secs;
    }
}

