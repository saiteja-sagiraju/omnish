#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "string.h"
#include "tokenizer.h"
#include "parser.h"
#include "executor.h"

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

        if (c == EOF) {
            if (pos == 0) {
                free(buffer);
                return NULL;
            }
            buffer[pos] = '\0';
            return buffer;
        }

        if (c == '\n') {
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
        if (!input) {
            putchar('\n');
            break;
        }

        if (input[0] == '\0') {
            free(input);
            continue;
        }

        Token *tokens = tokenize(input);
        Token *current_token = tokens;
        ASTNode *ast = parse_expression(&current_token);

        int exec_status = execute_ast(ast);

        if (exec_status < 0) {
            perror("execute_ast");
        }

        free_ast(ast);
        free_tokens(tokens);
        free(input);
    }

    return 0;
}
