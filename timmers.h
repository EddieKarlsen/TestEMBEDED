#ifndef TIMMERS_H
#define TIMMERS_H

#include <stdint.h>

// Timer funktioner
void initMillisTimer(void);
uint32_t millis(void);

// Globala tidräknare
extern volatile uint32_t timerMillis;

#endif /* TIMER_H */