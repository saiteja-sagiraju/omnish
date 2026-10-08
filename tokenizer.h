#include "string.h"

#define INIT_CAPACITY 16

typedef enum {
    WORD,
    SEQ, // &&
    PIPE, // |
    REDIR_IN, // <
    REDIR_OUT, // >
    APPEND, // >>
    END
} TokenType;


typedef struct Token {
    TokenType type;
    string argv;
} Token;

typedef struct Tokens {
    Token **tokens;
    int count;
} Tokens;

Tokens tokenize(string command);

const char* token_type_to_string(TokenType type);

void free_tokens(Tokens *tokens);
