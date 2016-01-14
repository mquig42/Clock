void setup()
{
    for(unsigned char i = 1; i<13; i++)
    {
        pinMode(i, OUTPUT);
    }
}

void loop()
{
    unsigned char interval = 250;
    PORTB = 0b10000;
    PORTD = 0b01111111;
    delay(interval);
    PORTD = 0b10111111;
    delay(interval);
    PORTD = 0b11011111;
    delay(interval);
    PORTD = 0b11101111;
    delay(interval);
    PORTD = 0b11110111;
    delay(interval);
    PORTD = 0b11111011;
    delay(interval);
    PORTD = 0b11111101;
    delay(interval);
}
