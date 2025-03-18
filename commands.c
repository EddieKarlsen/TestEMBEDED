#include <string.h>
#include "commands.h"
#include "uart.h"
#include "main.h"
#include "buttons.h"
#include "leds.h"
#include "adc.h"

int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'a' && *s1 <= 'z') ? (*s1 - 32) : *s1;
        char c2 = (*s2 >= 'a' && *s2 <= 'z') ? (*s2 - 32) : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return *s1 - *s2;
}

int strncasecmp_custom(const char *s1, const char *s2, size_t n) {
    while (n > 0 && *s1 && *s2) {
        char c1 = (*s1 >= 'a' && *s1 <= 'z') ? (*s1 - 32) : *s1;
        char c2 = (*s2 >= 'a' && *s2 <= 'z') ? (*s2 - 32) : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
        n--;
    }
    if (n == 0) return 0;
    return *s1 - *s2;
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
    else if (strncasecmp_custom(command, "disable button ", 15) == 0) {
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