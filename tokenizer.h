#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "string.h"

#define INIT_CAPACITY 16

typedef enum {
    WORD,
    SEQ,
    AND,
    OR,
    PIPE, // |
    REDIR_IN, // <
    REDIR_OUT, // >
    APPEND, // >>
    END
} TokenType;


typedef struct Token {
    TokenType type;
    char *argv;
} Token;

Token *tokenize(char *command);

const char* token_type_to_string(TokenType type);

void free_tokens(Token *tokens);

#endif
