#include "tokenizer.h"

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case WORD:      return "WORD";
        case SEQ:       return "SEQ";
        case PIPE:      return "PIPE";
        case REDIR_IN:  return "REDIR_IN";
        case REDIR_OUT: return "REDIR_OUT";
        case APPEND:    return "APPEND";
        case END:       return "END";
        default:        return "UNKNOWN";
    }
}

static bool is_metachar(char c) {
    return c == '|' || c == '<' || c == '>' || c == '&' || c == ';';
}

Token *tokenize(char *command) {
    int index = 0;
    int token_capacity = INIT_CAPACITY;
    Token *tokens = malloc(token_capacity * sizeof(Token));

    int capacity = INIT_CAPACITY;
    char *buffer = malloc(capacity * sizeof(char));

    int i = 0;

    while (command[i] != '\0') {
        while (command[i] == ' ' || command[i] == '\t' || command[i] == '\n') {
            i++;
        }

        if (command[i] == '\0') break;

        Token token = {0};
        int pos = 0;

        if (is_metachar(command[i])) {
            if (command[i] == '|') {
                if (command[i + 1] == '|') {
                    token.type = OR;
                    token.argv = str_dup("||");
                    i += 2;
                } else {
                    token.type = PIPE;
                    token.argv = str_dup("|");
                    i++;
                }
            } else if (command[i] == '&' && command[i + 1] == '&') {
                token.type = AND;
                token.argv = str_dup("&&");
                i += 2;
            } else if (command[i] == '>') {
                if (command[i + 1] == '>') {
                    token.type = APPEND;
                    token.argv = str_dup(">>");
                    i += 2;
                } else {
                    token.type = REDIR_OUT;
                    token.argv = str_dup(">");
                    i++;
                }
            } else if (command[i] == '<') {
                token.type = REDIR_IN;
                token.argv = str_dup("<");
                i++;
            } else {
                token.type = SEQ;
                buffer[0] = command[i++];
                buffer[1] = '\0';
                token.argv = str_dup(buffer);
            }
        }

        else {
            token.type = WORD;
            bool in_single_quote = false;
            bool in_double_quote = false;

            while (command[i] != '\0') {
                char c = command[i];

                if (c == '\'' && !in_double_quote) {
                    in_single_quote = !in_single_quote;
                    i++;
                    continue;
                } else if (c == '"' && !in_single_quote) {
                    in_double_quote = !in_double_quote;
                    i++;
                    continue;
                }

                if (!in_single_quote && !in_double_quote) {
                    if (c == ' ' || c == '\t' || c == '\n' || is_metachar(c)) break;
                }

                if (pos >= capacity - 1) {
                    capacity *= 2;
                    buffer = realloc(buffer, capacity * sizeof(char));
                }

                buffer[pos++] = c;
                i++;
            }

            buffer[pos++] = '\0';
            token.argv = str_dup(buffer);
        }

        if (index >= token_capacity) {
            token_capacity *= 2;
            tokens = realloc(tokens, token_capacity * sizeof(Token));
        }

        tokens[index++] = token;
    }

    Token end_token = {0};
    end_token.type = END;
    end_token.argv = str_dup("");

    if (index >= token_capacity) {
        token_capacity += 1;
        tokens = realloc(tokens, token_capacity * sizeof(Token));
    }
    tokens[index++] = end_token;

    free(buffer);

    return tokens;
}

void free_tokens(Token *tokens) {
    if (!tokens) return;

    int i = 0;
    while (tokens[i].type != END) {
        free(tokens[i].argv);
        i++;
    }

    free(tokens[i].argv);
    free(tokens);
}
