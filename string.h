#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool str_compare(const char *s1, const char *s2);
void str_copy(char *dest, const char *src);
int str_len(const char *str);
char* str_dup(const char *src);

#endif