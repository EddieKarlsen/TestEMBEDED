#ifndef COMMANDS_H
#define COMMANDS_H


void processCommand(char *command);


int strcasecmp_custom(const char *s1, const char *s2);
int strncasecmp_custom(const char *s1, const char *s2, size_t n);
int atoi_custom(const char *str);
void itoa_custom(int num, char* str, int base);

#endif 