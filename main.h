#ifndef __MAIN_H
#define __MAIN_H

void initUSART();
void USART_Transmit(unsigned char data);
void USART_PrintString(const char* str);
void initMillisTimer();
void initPWM();
void itoa_custom(int num, char* str, int base);
void setup();
#endif