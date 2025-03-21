#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
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
volatile uint8_t buttonEnabled[NUM_BUTTONS] = {1, 1, 1};
volatile LEDState ledStates[NUM_BUTTONS] = {LED_BLINKING, LED_BLINKING, LED_BLINKING};
volatile uint8_t blueIntensity = 255;
volatile uint8_t potControlEnabled = 1; 




#define LED_RED_PIN    PB5
#define LED_GREEN_PIN  PB4
#define LED_BLUE_PIN   PC1
#define LED_RGB_RED_PIN   PB3
#define LED_RGB_GREEN_PIN PB2
#define LED_RGB_BLUE_PIN  PB1
#define POT_A0_CHANNEL    0  // A0
#define POT_RED_CHANNEL   2  // A2
#define POT_GREEN_CHANNEL 3  // A3
#define POT_BLUE_CHANNEL  4  // A4






void setup(){
    DDRB |= (1 << LED_RED_PIN);
    DDRB |= (1 << LED_GREEN_PIN);
    DDRB |= (1 << LED_RGB_RED_PIN);
    DDRB |= (1 << LED_RGB_GREEN_PIN);
    DDRB |= (1 << LED_RGB_BLUE_PIN);



    ADMUX = (1 << REFS0); 
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

    EICRA |= (1 << ISC01);    
    EIMSK |= (1 << INT0);     
    sei();                    

    return;
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

void softwareReset() {
    wdt_enable(WDTO_15MS);
    while (1);
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
    ADMUX = (ADMUX & 0xF8) | (POT_A0_CHANNEL & 0x07);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC; 
}
uint16_t readADCChannel(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); 
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

void initPWM() {
    // Setup timer 2 för PB3
    TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B |= (1 << CS22);
    
    // Setup timer 2 för PB2 
    TCCR1A |= (1 << COM1B1) | (1 << WGM10);
    TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12);
    
    // Setup timer 1 för PB1
    TCCR1A |= (1 << COM1A1); 
    // Setup Timer0 för den Blåa LED PWM
    TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00);
    OCR0A = 255;
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
        USART_PrintString("5. Reset - Restart the software\r\n");
        USART_PrintString("6. Disable button <number> - Disable a button by its number\r\n");
        USART_PrintString("7. Enable button <number> - Enable a button by its number\r\n");
        USART_PrintString("8. LedPower <0-255|-1> - Set LED intensity (0-255) or enable potentiometer control (-1)\r\n");
        USART_PrintString("\r\n");
    }

    if (strncasecmp_custom(command, "disable button ", 15) == 0) {
        char *buttonNum = command + 15;
        int btnIdx = atoi_custom(buttonNum) - 1;
        
        if (btnIdx >= 0 && btnIdx < NUM_BUTTONS) {
            buttonEnabled[btnIdx] = 0;
            USART_PrintString("Button ");
            USART_Transmit('1' + btnIdx);
            USART_PrintString(" disabled\r\n");
        } else {
            USART_PrintString("Invalid button number\r\n");
        }
    }
    else if (strncasecmp_custom(command, "enable button ", 14) == 0) {
        char *buttonNum = command + 14;
        int btnIdx = atoi_custom(buttonNum) - 1;
        
        if (btnIdx >= 0 && btnIdx < NUM_BUTTONS) {
            buttonEnabled[btnIdx] = 1;
            USART_PrintString("Button ");
            USART_Transmit('1' + btnIdx);
            USART_PrintString(" enabled\r\n");
        } else {
            USART_PrintString("Invalid button number\r\n");
        }
    }
    else if (strncasecmp_custom(command, "ledpower ", 9) == 0) {
        char *powerLevel = command + 9;
        int level = atoi_custom(powerLevel);
        
        if (level == -1) {
            potControlEnabled = 1;
            USART_PrintString("Potentiometer control enabled\r\n");
        } else if (level >= 0 && level <= 255) {
            potControlEnabled = 0;
            blueIntensity = level;
            USART_PrintString("Blue LED intensity set to: ");
            char buffer[10];
            itoa_custom(level, buffer, 10);
            USART_PrintString(buffer);
            USART_PrintString("\r\n");
        } else {
            USART_PrintString("Invalid power level (use 0-255 or -1)\r\n");
        }
    }

    else if (strcasecmp_custom(command, "getadc") == 0) {
        uint16_t adcValue = readADCChannel(POT_A0_CHANNEL); 
        char buffer[10];
        itoa_custom(adcValue, buffer, 10);
        USART_PrintString("A0 Potentiometer Value: ");
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
        softwareReset();
    } 
    else {
        USART_PrintString("Unknown Command\r\n");
    }
}

int atoi_custom(const char *str) {
    int res = 0;
    int sign = 1;
    
 
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    
    return sign * res;
}

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
}
void handleRGB(){
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

        if (commandReady) {
            char tempCommand[COMMAND_BUFFER_SIZE];
            strcpy(tempCommand, (char*)commandBuffer); 
            commandReady = 0;
            processCommand(tempCommand);
        }
    }
}