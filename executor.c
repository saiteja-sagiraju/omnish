#include "executor.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int execute_ast(ASTNode *node) {
    if (!node) return 0;

    switch (node->type) {

        case NODE_COMMAND: {

            if (str_compare(node->command.argv[0], "cd")) {
                int changed = chdir(node->command.argv[1]);
                if (changed < 0) perror(node->command.argv[1]);
                return 0;
            }

            pid_t pid = fork();
            if (pid == 0) {
                execvp(node->command.argv[0], node->command.argv);
                perror(node->command.argv[0]);
                exit(1);
            }

            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }

        case NODE_PIPE: {
            int fd[2];
            pipe(fd);

            // write to pipe
            if (fork() == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                exit(execute_ast(node->pipe.left));
            }

            // read from pipe
            if (fork() == 0) {
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                exit(execute_ast(node->pipe.right));
            }

            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
            return 0;
        }

        case NODE_REDIRECT: {
            int flags;

            if (node->redirect.mode == REDIR_OUT) {
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            } else if (node->redirect.mode == REDIR_IN) {
                flags = O_RDONLY;
            } else if (node->redirect.mode == APPEND) {
                flags = O_WRONLY | O_CREAT | O_APPEND;
            } else {
                return -1;
            }

            int target_std = (node->redirect.mode == REDIR_IN) ? STDIN_FILENO : STDOUT_FILENO;
            int saved_std = dup(target_std);

            int fd = open(node->redirect.file, flags, 0644);
            if (fd < 0) {
                perror(node->redirect.file);
                close(saved_std);
                return 1;
            }

            dup2(fd, target_std);
            close(fd);

            int status = execute_ast(node->redirect.child);

            dup2(saved_std, target_std);
            close(saved_std);

            return status;

        }

        case NODE_AND: {
            int left_status = execute_ast(node->logical.left);

            if (left_status == 0) {
                return execute_ast(node->logical.right);
            }

            return left_status;
        }

        case NODE_OR: {
            int left_status = execute_ast(node->logical.left);

            if (left_status == 0) return 0;

            return execute_ast(node->logical.right);
        }

    }

    return -1;

}
