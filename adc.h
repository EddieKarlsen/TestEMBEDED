#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// ADC funktioner
void initADC(void);
uint16_t readADC(void);
uint16_t readADCChannel(uint8_t channel);

#endif /* ADC_H */