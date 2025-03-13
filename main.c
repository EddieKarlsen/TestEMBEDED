#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "main.h"
#include <string.h>

typedef enum {
    BUTTON_RED = 0,   
    BUTTON_GREEN,      
    BUTTON_BLUE,       
    NUM_BUTTONS       
} Button;

typedef enum {
    LED_OFF = 0,    
    LED_ON,           
    LED_BLINKING      
} LEDState;

volatile uint32_t timerMillis = 0;
volatile uint8_t buttonStates[NUM_BUTTONS] = {0};
volatile uint8_t lastButtonStates[NUM_BUTTONS] = {0}; 
volatile uint8_t buttonPressed[NUM_BUTTONS] = {0};
volatile LEDState ledStates[NUM_BUTTONS] = {LED_BLINKING, LED_BLINKING, LED_BLINKING};



#define LED_RED_PIN    PB5
#define LED_GREEN_PIN  PB4
#define LED_BLUE_PIN   PC1
#define LED_RGB_PIN    PB3


void setup(){
    DDRB |= (1 << LED_RED_PIN);
    DDRB |= (1 << LED_GREEN_PIN);
    DDRB |= (1 << LED_RGB_PIN);



    ADMUX = (1 << REFS0); //blå potentiometer
    DDRC |= (1 << LED_BLUE_PIN); // blå LED  
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

    DDRB &= ~(1 << PB0); //Röd Knapp
    PORTB |= (1 << PB0);
    DDRB &= ~(1 << PD7); //Grön Knapp
    PORTD |= (1 << PD7);
    DDRB &= ~(1 << PD6); //Blå Knapp
    PORTD |= (1 << PD6);
    DDRB &= ~(1 << PD2); //Gul ResetKnapp
    PORTD |= (1 << PD2); 

    return;
}

void initMillisTimer() {
    TCCR0A |= (1 << WGM01);  
    TCCR0B |= (1 << CS01) | (1 << CS00);  
    OCR0A = 249;
    TIMSK0 |= (1 << OCIE0A); 
    sei(); 
}

ISR(TIMER0_COMPA_vect) {
    timerMillis++;
}

uint32_t millis() {
    uint32_t ms;
    cli();
    ms = timerMillis;
    sei();
    return ms;
}

uint16_t readADC() {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC; 
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
    static uint8_t lastButtonStates[NUM_BUTTONS] = {0};
    static uint8_t buttonPressed[NUM_BUTTONS] = {0};

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

uint8_t isResetButtonPressed() {
    if ((PIND & (1 << PD2)) == 0) { 
        _delay_ms(50);
        return (PIND & (1 << PD2)) == 0;
    }
    return 0;
}

void initUSART() {

    unsigned int ubrr = F_CPU / 16 / 9600 - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr; 
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void USART_PrintString(char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}
void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data; 
}

void USART_ReadString(char *buffer, uint8_t maxLength) {
    
    uint8_t index = 0;
    while (index < maxLength - 1) {
        if (UCSR0A & (1 << RXC0)) {
    buffer[index++] = UDR0;
}        char received = UDR0;
        if (received == '\n' || received == '\r') {
            break;
        }
        buffer[index++] = received;
    }
    buffer[index] = '\0';
}

void initPWM() {
    DDRC |= (1 << PC1);
    TCCR1A |= (1 << COM1A1) | (1 << WGM10); 
    TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12); 

}

void itoa_custom(int num, char* str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int digits = 0;
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }


    if (num < 0 && base == 10) {
        *ptr++ = '-';
        num = -num;
    }
    while (num) {
        int rem = num % base;
        *ptr++ = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
        digits++;
    }
    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr = *ptr1;
        *ptr1 = tmp_char;
        ptr--;
        ptr1++;
    }
}

void processCommand(char *command) {
    USART_PrintString(command);
    if (strncmp(command, "LedOn RED", (size_t)9) == 0) {
        ledStates[BUTTON_RED] = LED_ON;
    } 
    if (strncmp(command, "a", 1)==0){
        USART_PrintString("HEJ");
    }
    else if (strncmp(command, "LedOn GREEN", 11) == 0) {
        ledStates[BUTTON_GREEN] = LED_ON;
    } else if (strncmp(command, "LedOn BLUE", 10) == 0) {
        ledStates[BUTTON_BLUE] = LED_ON;
    } else if (strncmp(command, "LedOff RED", 10) == 0) {
        ledStates[BUTTON_RED] = LED_OFF;
    } else if (strncmp(command, "LedOff GREEN", 12) == 0) {
        ledStates[BUTTON_GREEN] = LED_OFF;
    } else if (strncmp(command, "LedOff BLUE", 11) == 0) {
        ledStates[BUTTON_BLUE] = LED_OFF;
    } else if (strncmp(command, "LedBlink RED", 12) == 0) {
        ledStates[BUTTON_RED] = LED_BLINKING;
    } else if (strncmp(command, "LedBlink GREEN", 14) == 0) {
        ledStates[BUTTON_GREEN] = LED_BLINKING;
    } else if (strncmp(command, "LedBlink BLUE", 13) == 0) {
        ledStates[BUTTON_BLUE] = LED_BLINKING;
    } else if (strncmp(command, "GetADC", 6) == 0) {
        uint16_t adcValue = readADC(); 
        char buffer[10];
        itoa_custom(adcValue, buffer, 10);  
        USART_PrintString("ADC Value: "); 
        USART_PrintString(buffer);        
        USART_Transmit('\n');             
        USART_Transmit('\r');              
    } else {
        USART_PrintString("Unknown Command\r\n");
    }
}


void handleLED(Button button) {
    switch (ledStates[button]) {
        case LED_OFF:
            if (button == BUTTON_RED) PORTB &= ~(1 << LED_RED_PIN);
            else if (button == BUTTON_GREEN) PORTB &= ~(1 << LED_GREEN_PIN);
            else if (button == BUTTON_BLUE) PORTC &= ~(1 << LED_BLUE_PIN); 
            break;
        case LED_ON:
            if (button == BUTTON_RED) PORTB |= (1 << LED_RED_PIN);
            else if (button == BUTTON_GREEN) PORTB |= (1 << LED_GREEN_PIN);
            else if (button == BUTTON_BLUE) PORTC |= (1 << LED_BLUE_PIN);
            break;
        case LED_BLINKING:
            if (millis() % 2000 < 1000) {
                if (button == BUTTON_RED) PORTB |= (1 << LED_RED_PIN);
                else if (button == BUTTON_GREEN) PORTB |= (1 << LED_GREEN_PIN);
                else if (button == BUTTON_BLUE) PORTC |= (1 << LED_BLUE_PIN);
            } else {
                if (button == BUTTON_RED) PORTB &= ~(1 << LED_RED_PIN);
                else if (button == BUTTON_GREEN) PORTB &= ~(1 << LED_GREEN_PIN);
                else if (button == BUTTON_BLUE) PORTC &= ~(1 << LED_BLUE_PIN);
            }
            break;
    }
}

int main() 
{ 
    char commandBuffer[20];
    initMillisTimer(); 
    initUSART();
    initPWM(); 
    setup();
    USART_PrintString("ADC Value: ");
    while (1) {
        handleButton(BUTTON_RED);
        handleButton(BUTTON_GREEN);
        handleButton(BUTTON_BLUE);
        
        handleLED(BUTTON_RED);
        handleLED(BUTTON_GREEN);
        handleLED(BUTTON_BLUE);

        if (isResetButtonPressed()) {
            ledStates[BUTTON_RED] = LED_BLINKING;
            ledStates[BUTTON_GREEN] = LED_BLINKING;
            ledStates[BUTTON_BLUE] = LED_BLINKING;
        }
        
        _delay_ms(100);
        USART_ReadString(commandBuffer, sizeof(commandBuffer));
        if (commandBuffer[0] != '\0') {
            processCommand(commandBuffer);  
            
        }
        USART_PrintString("test");
    }
}

