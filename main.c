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

#define COMMAND_BUFFER_SIZE 20

volatile uint32_t timerMillis = 0;

volatile char commandBuffer[COMMAND_BUFFER_SIZE];
volatile uint8_t commandIndex = 0;
volatile uint8_t commandReady = 0;

volatile uint8_t buttonStates[NUM_BUTTONS] = {0};
volatile uint8_t lastButtonStates[NUM_BUTTONS] = {0}; 
volatile uint8_t buttonPressed[NUM_BUTTONS] = {0};
volatile LEDState ledStates[NUM_BUTTONS] = {LED_BLINKING, LED_BLINKING, LED_BLINKING};



#define LED_RED_PIN    PB5
#define LED_GREEN_PIN  PB4
#define LED_BLUE_PIN   PC1
#define LED_RGB_RED_PIN   PB3
#define LED_RGB_GREEN_PIN PB2
#define LED_RGB_BLUE_PIN  PB1
#define POT_RED_CHANNEL   2  // A2
#define POT_GREEN_CHANNEL 3  // A3
#define POT_BLUE_CHANNEL  4  // A4






void setup(){
    DDRB |= (1 << LED_RED_PIN);
    DDRB |= (1 << LED_GREEN_PIN);
    DDRB |= (1 << LED_RGB_RED_PIN);
    DDRB |= (1 << LED_RGB_GREEN_PIN);
    DDRB |= (1 << LED_RGB_BLUE_PIN);



    ADMUX = (1 << REFS0); //blå potentiometer
    DDRC |= (1 << LED_BLUE_PIN);
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
uint16_t readADCChannel(uint8_t channel) {
    // Set the ADC channel (0-7)
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // Clear the bottom 3 bits and set channel
    
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));
    
    // Return result
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
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    sei(); 
}

int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'a' && *s1 <= 'z') ? (*s1 - 32) : *s1;
        char c2 = (*s2 >= 'a' && *s2 <= 'z') ? (*s2 - 32) : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return *s1 - *s2;
}

void USART_PrintString(const char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}

void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

ISR(USART_RX_vect) {
    char received = UDR0;

    if (received == '\r' || received == '\n') {  
        commandBuffer[commandIndex] = '\0';
        commandReady = 1;
        commandIndex = 0;
        USART_PrintString("\r\n");
    } 
    else if (received == '\b' || received == 127) { 
        if (commandIndex > 0) {
            commandIndex--;
            USART_PrintString("\b \b");
        }
    } 
    else if (commandIndex < COMMAND_BUFFER_SIZE - 1) {  
        commandBuffer[commandIndex++] = received;
        USART_Transmit(received);
    }
}



// void initPWM() {
//     DDRC |= (1 << PC1);
//     TCCR1A |= (1 << COM1A1) | (1 << WGM10); 
//     TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12); 

// }

void initPWM() {
    // Setup timer 2 for PB3 (OC2A) - RED
    TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20); // Fast PWM, non-inverting
    TCCR2B |= (1 << CS22);  // Prescaler = 64
    
    // Setup timer 1 for PB2 (OC1B) - GREEN
    TCCR1A |= (1 << COM1B1) | (1 << WGM10); // 8-bit PWM, non-inverting
    TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12); // Prescaler = 64
    
    // Setup timer 1 for PB1 (OC1A) - BLUE
    TCCR1A |= (1 << COM1A1); // Already setup above, just enable output compare
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
    char *newline = strpbrk(command, "\r\n");
    if (newline) *newline = '\0';

    USART_PrintString("Processing: ");
    USART_PrintString(command);
    USART_PrintString("\r\n");


    USART_PrintString("Processed Command: ");
    USART_PrintString(command);
    USART_PrintString("\r\n");

 
    if (strcasecmp_custom(command, "help") == 0) {
        USART_PrintString("Available commands:\r\n");
        USART_PrintString("1. GetADC - Get the ADC value from the potentiometer\r\n");
        USART_PrintString("2. LedOn <color> - Turn on the LED for the specified color (red, green, blue)\r\n");
        USART_PrintString("3. LedOff <color> - Turn off the LED for the specified color (red, green, blue)\r\n");
        USART_PrintString("4. LedBlink <color> - Make the LED blink for the specified color (red, green, blue)\r\n");
        USART_PrintString("5. Reset - Reset the system (press the reset button)\r\n");
        USART_PrintString("\r\n");
    }

    else if (strcasecmp_custom(command, "getadc") == 0) {
        uint16_t adcValue = readADC(); 
        char buffer[10];
        itoa_custom(adcValue, buffer, 10);
        USART_PrintString("ADC Value: ");
        USART_PrintString(buffer);
        USART_PrintString("\r\n");
    } 
    else if (strcasecmp_custom(command, "ledon red") == 0) {
        ledStates[BUTTON_RED] = LED_ON;
    } 
    else if (strcasecmp_custom(command, "ledon green") == 0) {
        ledStates[BUTTON_GREEN] = LED_ON;
    } 
    else if (strcasecmp_custom(command, "ledon blue") == 0) {
        ledStates[BUTTON_BLUE] = LED_ON;
    } 
    else if (strcasecmp_custom(command, "ledoff red") == 0) {
        ledStates[BUTTON_RED] = LED_OFF;
    } 
    else if (strcasecmp_custom(command, "ledoff green") == 0) {
        ledStates[BUTTON_GREEN] = LED_OFF;
    } 
    else if (strcasecmp_custom(command, "ledoff blue") == 0) {
        ledStates[BUTTON_BLUE] = LED_OFF;
    } 
    else if (strcasecmp_custom(command, "ledblink red") == 0) {
        ledStates[BUTTON_RED] = LED_BLINKING;
    } 
    else if (strcasecmp_custom(command, "ledblink green") == 0) {
        ledStates[BUTTON_GREEN] = LED_BLINKING;
    } 
    else if (strcasecmp_custom(command, "ledblink blue") == 0) {
        ledStates[BUTTON_BLUE] = LED_BLINKING;
    } 
    else if (strcasecmp_custom(command, "reset") == 0) {
        ledStates[BUTTON_RED] = LED_BLINKING;
        ledStates[BUTTON_GREEN] = LED_BLINKING;
        ledStates[BUTTON_BLUE] = LED_BLINKING;
    } 
    else {
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
void handleRGB(){
    // Read potentiometer values
    uint16_t redValue = readADCChannel(POT_RED_CHANNEL);
    uint16_t greenValue = readADCChannel(POT_GREEN_CHANNEL);
    uint16_t blueValue = readADCChannel(POT_BLUE_CHANNEL);
    
    // Convert 10-bit ADC value (0-1023) to 8-bit PWM value (0-255)
    uint8_t redPWM = redValue >> 2;   // Divide by 4
    uint8_t greenPWM = greenValue >> 2;
    uint8_t bluePWM = blueValue >> 2;
    
    // Update PWM values for each color
    OCR2A = redPWM;     // Set RED PWM duty cycle (PB3)
    OCR1B = greenPWM;   // Set GREEN PWM duty cycle (PB2)
    OCR1A = bluePWM;    // Set BLUE PWM duty cycle (PB1)
}
int main() 
{   

    initMillisTimer();
    initUSART();
    initPWM();
    setup();
    

    
    while (1) {
        

        

        handleButton(BUTTON_RED);
        handleButton(BUTTON_GREEN);
        handleButton(BUTTON_BLUE);
        
        handleLED(BUTTON_RED);
        handleLED(BUTTON_GREEN);
        handleLED(BUTTON_BLUE);  

        handleRGB();

        if (isResetButtonPressed()) {
            ledStates[BUTTON_RED] = LED_BLINKING;
            ledStates[BUTTON_GREEN] = LED_BLINKING;
            ledStates[BUTTON_BLUE] = LED_BLINKING;
        }
        
        if (commandReady) {
            char tempCommand[COMMAND_BUFFER_SIZE];
            strcpy(tempCommand, (char*)commandBuffer); 
            commandReady = 0;
            processCommand(tempCommand);
        }
    }
}