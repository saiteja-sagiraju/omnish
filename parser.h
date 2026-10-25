#ifndef PARSER_H
#define PARSER_H

#include "string.h"
#include "tokenizer.h"

#include <stdlib.h>

#define MAX_ARGS 64

typedef enum {
    NODE_COMMAND,
    NODE_PIPE,
    NODE_REDIRECT,
    NODE_AND,
    NODE_OR
} NodeType;

typedef struct ASTNode {
    NodeType type;

    union {
        struct {
            char **argv;
        } command;

        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } pipe;

        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } logical;


        struct {
            struct ASTNode *child;
            char *file;
            TokenType mode;
        } redirect;
    };
} ASTNode;

ASTNode *parse_expression(Token **tokens);
ASTNode *parse_pipe(Token **tokens);
ASTNode *parse_command(Token **tokens);
char *ast_to_string(const ASTNode *node);
void free_ast(ASTNode *node);

#endif