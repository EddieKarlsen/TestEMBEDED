#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#include "main.h"

volatile char commandBuffer[COMMAND_BUFFER_SIZE];
volatile uint8_t commandIndex = 0;
volatile uint8_t commandReady = 0;

void initUSART(void) {
    unsigned int ubrr = F_CPU / 16 / 9600 - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    sei(); 
}

void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void USART_PrintString(const char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
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