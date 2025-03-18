#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>

#include "main.h"
#include "buttons.h"
#include "leds.h"
#include "uart.h"
#include "adc.h"
#include "pwm.h"
#include "timmers.h"
#include "commands.h"

void setup(void) {
    // Initiera GPIO för LEDs
    DDRB |= (1 << LED_RED_PIN);
    DDRB |= (1 << LED_GREEN_PIN);
    DDRC |= (1 << LED_BLUE_PIN);
    DDRB |= (1 << LED_RGB_RED_PIN);
    DDRB |= (1 << LED_RGB_GREEN_PIN);
    DDRB |= (1 << LED_RGB_BLUE_PIN);

    // Interrupt för reset-knapp
    EICRA |= (1 << ISC01);    
    EIMSK |= (1 << INT0);     
    sei();                    
}

void softwareReset(void) {
    wdt_enable(WDTO_15MS);
    while (1);
}

int main(void) {
    // Initiera alla subsystem
    initMillisTimer();
    initUSART();
    initPWM();
    initADC();
    initButtons();
    setup();
       
    while (1) {
        handleButton(BUTTON_RED);
        handleButton(BUTTON_GREEN);
        handleButton(BUTTON_BLUE);
        
        handleLED(BUTTON_RED);
        handleLED(BUTTON_GREEN);
        handleLED(BUTTON_BLUE);  

        handleRGB();

        if (commandReady) {
            char tempCommand[COMMAND_BUFFER_SIZE];
            strcpy(tempCommand, (char*)commandBuffer); 
            commandReady = 0;
            processCommand(tempCommand);
        }
    }
}