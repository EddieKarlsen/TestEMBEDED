#include <avr/io.h>
#include <util/delay.h>
#include "buttons.h"
#include "leds.h"
#include "uart.h"
#include "main.h"

volatile uint8_t buttonStates[NUM_BUTTONS] = {0};
volatile uint8_t lastButtonStates[NUM_BUTTONS] = {0};
volatile uint8_t buttonPressed[NUM_BUTTONS] = {0};
volatile uint8_t buttonEnabled[NUM_BUTTONS] = {1, 1, 1};

void initButtons(void) {
    DDRB &= ~(1 << PB0); // Röd Knapp
    PORTB |= (1 << PB0);
    DDRB &= ~(1 << PD7); // Grön Knapp
    PORTD |= (1 << PD7);
    DDRB &= ~(1 << PD6); // Blå Knapp
    PORTD |= (1 << PD6);
    DDRB &= ~(1 << PD2); // Gul ResetKnapp
    PORTD |= (1 << PD2);
}

uint8_t readButton(Button button) {
    switch (button) {
        case BUTTON_RED:
            return (PINB & (1 << PB0)) == 0;
        case BUTTON_GREEN:
            return (PIND & (1 << PD7)) == 0;
        case BUTTON_BLUE:
            return (PIND & (1 << PD6)) == 0;
        default:
            return 0;
    }
}

void handleButton(Button button) {
    if (!buttonEnabled[button]) return;
    uint8_t buttonStateNow = readButton(button);

    if (buttonStateNow != lastButtonStates[button]) {
        _delay_ms(50);  // Debouncer

        if (buttonStateNow == 0) {
            buttonPressed[button] = 1;
        }

        lastButtonStates[button] = buttonStateNow;
    }

    if (buttonPressed[button]) {
        if (ledStates[button] == LED_BLINKING) {
            ledStates[button] = LED_ON;
        } else if (ledStates[button] == LED_ON) {
            ledStates[button] = LED_OFF;
        } else if (ledStates[button] == LED_OFF) {
            ledStates[button] = LED_ON;
        }
        buttonPressed[button] = 0;  
    }
}

uint8_t isResetButtonPressed(void) {
    if ((PIND & (1 << PD2)) == 0) { 
        _delay_ms(50);
        return (PIND & (1 << PD2)) == 0;
    }
    return 0;
}

ISR(INT0_vect) {
    _delay_ms(50);
    if ((PIND & (1 << PD2)) == 0) {
        // Resetar alla states
        ledStates[BUTTON_RED] = LED_BLINKING;
        ledStates[BUTTON_GREEN] = LED_BLINKING;
        ledStates[BUTTON_BLUE] = LED_BLINKING;
        buttonEnabled[0] = 1;
        buttonEnabled[1] = 1;
        buttonEnabled[2] = 1;
        potControlEnabled = 1;
        commandIndex = 0;
        commandReady = 0;
        USART_PrintString("System reset\r\n");
    }
}