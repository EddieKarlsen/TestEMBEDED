#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include "buttons.h"

typedef enum {
    LED_OFF = 0,
    LED_ON,
    LED_BLINKING
} LEDState;

// LED funktioner
void handleLED(Button button);
void handleRGB(void);

// LED tillstånd
extern volatile LEDState ledStates[NUM_BUTTONS];
extern volatile uint8_t blueIntensity;
extern volatile uint8_t potControlEnabled;

#endif /* LEDS_H */