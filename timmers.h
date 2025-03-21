#ifndef TIMMERS_H
#define TIMMERS_H

#include <stdint.h>


void initMillisTimer(void);
uint32_t millis(void);


extern volatile uint32_t timerMillis;

#endif