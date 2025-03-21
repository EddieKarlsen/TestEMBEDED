#include <avr/io.h>
#include "adc.h"
#include "main.h"

void initADC(void) {
    ADMUX = (1 << REFS0); 
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

uint16_t readADC(void) {
    return readADCChannel(POT_A0_CHANNEL);
}

uint16_t readADCChannel(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); 
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}