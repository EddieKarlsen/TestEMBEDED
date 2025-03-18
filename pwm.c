#include <avr/io.h>
#include "pwm.h"

void initPWM(void) {
    // Setup timer 2 för PB3 - RED
    TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << CS22);
    
    // Setup timer 1 för PB2 - GREEN (1B) och PB1 - BLUE (1A)
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12);
    
    // Setup Timer0 för blue LED PWM
    TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00);
    OCR0A = 255;
}