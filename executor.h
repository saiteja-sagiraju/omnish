#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "tokenizer.h"
#include "parser.h"
#include "history.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int execute_ast(ASTNode *node, History *history);

#endif