/*******************************************************************************
 * LCD.ino
 * 2015.10.24
 * Mike Quigley
 * 
 * Binary clock using 5x7 LED matrix. Matrix is connected to I/O ports B and D
 * (Binary pins 1 -> 12). Pin 0 is not used because a resistor on the Arduino
 * board interferes and makes the lights dim, so values in port D must be
 * shifted accordingly.
 * 
 * Select and Set buttons are connected to analog pins 1 and 0,
 * aka digital pins 15 and 14, using internal pullup resistors.
 * Pressing Set button when clock is running switches between binary
 * and BCD output modes.
 ******************************************************************************/

#include <QClock.h>

enum ClockMode {SET_HOUR, SET_MINUTE, RUN};

bool select = false;
bool set = false;
bool bcd = false;



void setup()
{
    for(unsigned char i = 1; i<13; i++)
    {
        pinMode(i, OUTPUT);
    }
    pinMode(14, INPUT_PULLUP);
    pinMode(15, INPUT_PULLUP);
    pinMode(16, INPUT_PULLUP);
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
            m = (ClockMode)runState();
    }
    if(bcd)
        dispBcdTime();
    else
        dispBinaryTime();
}

//Reads the current state of both buttons, and sets the global variables accordingly
//previous state of each button is stored, so we only get TRUE when a button is first pressed.
//If a button is held down, set its variable to TRUE once, then FALSE until it's released and pressed again.
void readButtons()
{
    static uint8_t select_p = false;
    static uint8_t set_p = false;
    uint8_t select_c = digitalRead(15);
    uint8_t set_c = digitalRead(14);

    select = (!select_p && select_c);
    set = (!set_p && set_c);

    select_p = select_c;
    set_p = set_c;
}

//This function is called during the RUN state.
//Transition to SET_HOUR state when select button is pressed
//ignore set button
uint8_t runState()
{
    readButtons();
    
    if(select)
    {
        //Stop the timer while setting
        stopTimer();
        resetSeconds();
        return SET_HOUR;
    }
    if(set)
    {
        bcd = !bcd;
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

//Display time in pure binary format
void dispBinaryTime()
{
    copySeconds();
    uint8_t hours = getHour();
    uint8_t minutes = getMinute();
    uint8_t secs = getSecond();
    
    //Print time in HH:MM:SS format
    PORTB = 0b00001;
    PORTD = ~(hours << 1);
    delay(1);

    PORTB = 0b00100;
    PORTD = ~(minutes << 1);
    delay(1);

    PORTB = 0b10000;
    PORTD = ~(secs << 1);
    delay(1);

    if(!(secs % 2))
    {
        PORTB = 0b01010;
        PORTD = 0b11010111;
        delay(1);
    }
}

//Display time in BCD format
void dispBcdTime()
{
    copySeconds();
    uint8_t hours = getHour();
    uint8_t minutes = getMinute();
    uint8_t secs = getSecond();

    uint8_t timeArr[6];
    timeArr[0] = hours / 10;
    timeArr[1] = hours % 10;
    timeArr[2] = minutes / 10;
    timeArr[3] = minutes % 10;
    timeArr[4] = secs / 10;
    timeArr[5] = secs % 10;

    uint8_t scanArr[6] =
    {
        0b10111111,
        0b11011111,
        0b11101111,
        0b11110111,
        0b11111011,
        0b11111101
    };

    for(uint8_t i = 0; i < 6; i++)
    {
        PORTB = timeArr[i];
        PORTD = scanArr[i];
        delay(1);
        
    }
}

