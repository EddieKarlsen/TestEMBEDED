#ifndef UART_H
#define UART_H

#include <stdint.h>

#define COMMAND_BUFFER_SIZE 20

// UART funktioner
void initUSART(void);
void USART_Transmit(unsigned char data);
void USART_PrintString(const char* str);

// Command buffer
extern volatile char commandBuffer[COMMAND_BUFFER_SIZE];
extern volatile uint8_t commandIndex;
extern volatile uint8_t commandReady;

#endif /* UART_H */