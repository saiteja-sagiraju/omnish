#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "string.h"
#include "tokenizer.h"
#include "parser.h"

char* read_input() {
    int pos = 0;
    int capacity = INIT_CAPACITY;

    char *buffer = malloc(INIT_CAPACITY * sizeof(char));

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

        char *input = read_input();

        Token *tokens = tokenize(input);
        Token *current_token = tokens;
        ASTNode *ast = parse_expression(&current_token);
        char *round_trip = ast_to_string(ast);

        if (round_trip) {
            printf("%s\n", round_trip);
        }

        free(round_trip);
        free_ast(ast);
        free_tokens(tokens);
        free(input);
    }

    return 0;
}
