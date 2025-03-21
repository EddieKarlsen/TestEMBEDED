#include <avr/io.h>
#include "pwm.h"

void initPWM(void) {
    
    TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << CS22);
    
    
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12);
    
   
    TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00);
    OCR0A = 255;
}