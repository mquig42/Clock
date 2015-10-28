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

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
char select = 0;
char set = 0;
volatile uint32_t global_seconds = 0;

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
    dispTime();
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

