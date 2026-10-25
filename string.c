#include "string.h"

bool str_compare(const char *s1, const char *s2) {
    int i = 0;

    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return false;
        }

        i++;
    }

    return s2[i] == '\0' && s2[i] == '\0';
}

void str_copy(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int str_len(const char *str) {
    int i = 0;

    while (str[i] != '\0') {
        i++;
    };

    return i;
}

char* str_dup(const char *src) {
    int len = str_len(src);
    char *dest = malloc((len + 1) * sizeof(char));
    if (!dest) return NULL;

    str_copy(dest, src);
    return dest;
}
