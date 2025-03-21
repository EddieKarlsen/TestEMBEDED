#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <avr/io.h>

#define F_CPU 16000000UL


#define LED_RED_PIN        PB5
#define LED_GREEN_PIN      PB4
#define LED_BLUE_PIN       PC1
#define LED_RGB_RED_PIN    PB3
#define LED_RGB_GREEN_PIN  PB2
#define LED_RGB_BLUE_PIN   PB1


#define POT_A0_CHANNEL     0  // A0
#define POT_RED_CHANNEL    2  // A2
#define POT_GREEN_CHANNEL  3  // A3
#define POT_BLUE_CHANNEL   4  // A4


void setup(void);
void softwareReset(void);

#endif 