#include <stdio.h>
#include "string.h"
#include <stdlib.h>

void str_copy(string dest, const string src) {
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int str_len(string str) {
    int i = 0;

    while (str[i] != '\0') {
        i++;
    };

    return i;
}

string str_dup(const string src) {
    int len = str_len(src);
    string dest = malloc((len + 1) * sizeof(char));
    if (!dest) return NULL;

    str_copy(dest, src);
    return dest;
}
