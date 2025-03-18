#ifndef __MAIN_H
#define __MAIN_H
#include <stddef.h>
void initUSART();
void USART_Transmit(unsigned char data);
void USART_PrintString(const char* str);
void initMillisTimer();
void initPWM();
void itoa_custom(int num, char* str, int base);
int atoi_custom(const char *str);
int strncasecmp_custom(const char *s1, const char *s2, size_t n);

void processCommand(char *command);
void setup();


int strncasecmp_custom(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    while (*s1 && *s2 && i < n) {
        char c1 = (*s1 >= 'a' && *s1 <= 'z') ? (*s1 - 32) : *s1;
        char c2 = (*s2 >= 'a' && *s2 <= 'z') ? (*s2 - 32) : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++; i++;
    }
    if (i == n) return 0;
    return *s1 - *s2;
}

#endif