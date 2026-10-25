#include "parser.h"

ASTNode *parse_expression(Token **tokens) {
    ASTNode *left = parse_pipe(tokens);

    while ((*tokens)->type == AND || (*tokens)->type == OR) {
        TokenType type = (*tokens)->type;
        (*tokens)++;

        ASTNode *right = parse_pipe(tokens);
        ASTNode *logical_node = malloc(sizeof(ASTNode));
        if (!logical_node) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }

        logical_node->type = type == AND ? NODE_AND : NODE_OR;
        logical_node->logical.left = left;
        logical_node->logical.right = right;
        left = logical_node;
    }

    return left;
}

ASTNode *parse_pipe(Token **tokens) {
    ASTNode *left = parse_command(tokens);

    while ((*tokens)->type == PIPE) {
        (*tokens)++;

        ASTNode *right = parse_command(tokens);
        ASTNode *pipe_node = malloc(sizeof(ASTNode));
        if (!pipe_node) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }

        pipe_node->type = NODE_PIPE;
        pipe_node->pipe.left = left;
        pipe_node->pipe.right = right;
        left = pipe_node;
    }

    return left;
}

ASTNode *parse_command(Token **tokens) {
    ASTNode *command_node = malloc(sizeof(ASTNode));
    if (!command_node) return NULL;

    command_node->type = NODE_COMMAND;
    command_node->command.argv = calloc(MAX_ARGS, sizeof(char *));
    if (!command_node->command.argv) {
        free(command_node);
        return NULL;
    }

    ASTNode *base_command = command_node;
    int argc = 0;
    while ((*tokens)->type != PIPE && (*tokens)->type != AND && (*tokens)->type != OR && (*tokens)->type != END && (*tokens)->type != SEQ) {

        if ((*tokens)->type == REDIR_IN || (*tokens)->type == REDIR_OUT || (*tokens)->type == APPEND) {
            TokenType redir_type = (*tokens)->type;
            (*tokens)++;

            if ((*tokens)->type != WORD) {
                free_ast(command_node);
                return NULL;
            }

            ASTNode *redirect_node = malloc(sizeof(ASTNode));
            if (!redirect_node) {
                free_ast(command_node);
                return NULL;
            }

            redirect_node->type = NODE_REDIRECT;
            redirect_node->redirect.file = str_dup((*tokens)->argv);
            redirect_node->redirect.child = command_node;
            redirect_node->redirect.mode = redir_type;
            if (!redirect_node->redirect.file) {
                free(redirect_node);
                free_ast(command_node);
                return NULL;
            }

            (*tokens)++;
            command_node = redirect_node;
        } else {
            if (argc >= MAX_ARGS - 1) {
                free_ast(command_node);
                return NULL;
            }

            base_command->command.argv[argc] = str_dup((*tokens)->argv);
            if (!base_command->command.argv[argc]) {
                free_ast(command_node);
                return NULL;
            }
            argc++;
            (*tokens)++;
        }
    }

    return command_node;
}

void free_ast(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_COMMAND:
            for (int i = 0; node->command.argv[i]; i++) {
                free(node->command.argv[i]);
            }
            free(node->command.argv);
            break;

        case NODE_PIPE:
            free_ast(node->pipe.left);
            free_ast(node->pipe.right);
            break;

        case NODE_AND:
        case NODE_OR:
            free_ast(node->logical.left);
            free_ast(node->logical.right);
            break;

        case NODE_REDIRECT:
            free_ast(node->redirect.child);
            free(node->redirect.file);
            break;
    }

    free(node);
}
