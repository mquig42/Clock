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
volatile uint32_t seconds = 0;
volatile char ledState = 0;

void setup()
{
    lcd.begin(16, 2);
    pinMode(9, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);
    pinMode(13, OUTPUT);

    //Set up timer1 to count seconds
    //This is specific to the Atmel 328
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

ISR(TIMER1_COMPA_vect)
{
    seconds++;
    ledState = !ledState;
    digitalWrite(13, ledState);
    //Reset to 0 after 24 hours
    if(seconds >= 86400)
        seconds = 0;
}

void loop()
{
    uint32_t local_seconds;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        local_seconds = seconds;
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

    //Print millis on bottom line of lcd
    lcd.setCursor(0, 1);
    lcd.print(millis() / 1000);
}

