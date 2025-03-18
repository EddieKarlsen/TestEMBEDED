#include <avr/io.h>
#include <avr/interrupt.h>
#include "timmers.h"

volatile uint32_t timerMillis = 0;

void initMillisTimer(void) {
    TCCR0A |= (1 << WGM01);  
    TCCR0B |= (1 << CS01) | (1 << CS00);  
    OCR0A = 249;
    TIMSK0 |= (1 << OCIE0A); 
    sei(); 
}

uint32_t millis(void) {
    uint32_t ms;
    cli();
    ms = timerMillis;
    sei();
    return ms;
}

ISR(TIMER0_COMPA_vect) {
    timerMillis++;
}