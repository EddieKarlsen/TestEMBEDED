#include <avr/io.h>
#include "leds.h"
#include "buttons.h"
#include "adc.h"
#include "timmers.h"
#include "main.h"

volatile LEDState ledStates[NUM_BUTTONS] = {LED_BLINKING, LED_BLINKING, LED_BLINKING};
volatile uint8_t blueIntensity = 255;
volatile uint8_t potControlEnabled = 1;

void handleLED(Button button) {
    if (button == BUTTON_BLUE) {
        switch (ledStates[button]) {
            case LED_OFF:
                PORTC &= ~(1 << LED_BLUE_PIN);
                break;
            case LED_ON:
                if (potControlEnabled) {
                    uint16_t potValue = readADC(); 
                    blueIntensity = potValue >> 2;
                    OCR0A = blueIntensity;
                    PORTC |= (1 << LED_BLUE_PIN);
                } else {
                    PORTC |= (1 << LED_BLUE_PIN);
                }
                break;
            case LED_BLINKING:
                if (millis() % 2000 < 1000) {
                    if (potControlEnabled) {
                        uint16_t potValue = readADC();
                        blueIntensity = potValue >> 2;
                        OCR0A = blueIntensity;
                    }
                    PORTC |= (1 << LED_BLUE_PIN);
                } else {
                    PORTC &= ~(1 << LED_BLUE_PIN);
                }
                break;
        }
    } 
    else {
        switch (ledStates[button]) {
            case LED_OFF:
                if (button == BUTTON_RED) PORTB &= ~(1 << LED_RED_PIN);
                else if (button == BUTTON_GREEN) PORTB &= ~(1 << LED_GREEN_PIN);
                break;
            case LED_ON:
                if (button == BUTTON_RED) PORTB |= (1 << LED_RED_PIN);
                else if (button == BUTTON_GREEN) PORTB |= (1 << LED_GREEN_PIN);
                break;
            case LED_BLINKING:
                if (millis() % 2000 < 1000) {
                    if (button == BUTTON_RED) PORTB |= (1 << LED_RED_PIN);
                    else if (button == BUTTON_GREEN) PORTB |= (1 << LED_GREEN_PIN);
                } else {
                    if (button == BUTTON_RED) PORTB &= ~(1 << LED_RED_PIN);
                    else if (button == BUTTON_GREEN) PORTB &= ~(1 << LED_GREEN_PIN);
                }
                break;
        }
    }
}

void handleRGB(void) {
    uint16_t redValue = readADCChannel(POT_RED_CHANNEL);
    uint16_t greenValue = readADCChannel(POT_GREEN_CHANNEL);
    uint16_t blueValue = readADCChannel(POT_BLUE_CHANNEL);
    
    uint8_t redPWM = redValue >> 2; 
    uint8_t greenPWM = greenValue >> 2;
    uint8_t bluePWM = blueValue >> 2;
    
    OCR2A = redPWM;
    OCR1B = greenPWM;
    OCR1A = bluePWM;
}