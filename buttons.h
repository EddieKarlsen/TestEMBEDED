#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

typedef enum {
    BUTTON_RED = 0,
    BUTTON_GREEN,
    BUTTON_BLUE,
    NUM_BUTTONS
} Button;

// Funktionsdeklarationer
void initButtons(void);
uint8_t readButton(Button button);
void handleButton(Button button);
uint8_t isResetButtonPressed(void);

// Knapptillstånd
extern volatile uint8_t buttonStates[NUM_BUTTONS];
extern volatile uint8_t lastButtonStates[NUM_BUTTONS];
extern volatile uint8_t buttonPressed[NUM_BUTTONS];
extern volatile uint8_t buttonEnabled[NUM_BUTTONS];

#endif /* BUTTONS_H */