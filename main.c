#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "string.h"
#include "tokenizer.h"

string read_input() {
    int pos = 0;
    int capacity = INIT_CAPACITY;

    string buffer = malloc(INIT_CAPACITY * sizeof(char));

    if (!buffer) {
        perror("Allocation error");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int c = fgetc(stdin);

        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        }

        buffer[pos++] = (char)c;

        if (pos >= capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity * sizeof(char));

            if (!buffer) {
                perror("Reallocation error");
                exit(EXIT_FAILURE);
            }
        }

    }

}

int main() {

    char cwd[PATH_MAX];

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("\033[1;34m%s\033[0m> ", cwd);
        }

        string input = read_input();

        Tokens tokens = tokenize(input);

        for (int i = 0; i < tokens.count; i++) {
            printf("%s: %s\n", token_type_to_string(tokens.tokens[i]->type), tokens.tokens[i]->argv);
        }

        free_tokens(&tokens);
    }

    return 0;
}
