#include <stdbool.h>

typedef char* string;

bool str_compare(string s1, string s2);
void str_copy(string dest, const string src);
int str_len(string str);
string str_dup(string src);