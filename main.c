#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "string.h"
#include "tokenizer.h"
#include "parser.h"
#include "executor.h"
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include "history.h"

#define KEY_ENTER     13
#define KEY_CTRL_C    3
#define KEY_CTRL_D    4
#define KEY_BACKSPACE 127
#define KEY_UP        1001
#define KEY_DOWN      1002
#define KEY_INTERRUPT 1003

static struct termios orig_termios;
static int raw_mode_active = 0;
static volatile sig_atomic_t stop_requested = 0;

void disable_raw_mode(void) {
    if (raw_mode_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_active = 0;
    }
}

void enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr failure");
        exit(EXIT_FAILURE);
    }

    atexit(disable_raw_mode);

    struct termios raw = orig_termios;

    /* Input flags: Disable break, CR-to-NL translation, parity, flow control */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /* Control flags: Set 8-bit chars */
    raw.c_cflag |= (CS8);

    /* Local flags: Disable echo, canonical mode, extended input processing.
     * Keeping ISIG enabled allows Ctrl+C to still generate SIGINT. */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    /* Control characters: read returns immediately per byte (no timeout) */
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr failure");
        exit(EXIT_FAILURE);
    }

    raw_mode_active = 1;

}

void handle_sigint(int sig) {
    (void)sig;
    stop_requested = 1;
    write(STDOUT_FILENO, "\n", 1);
}

void setup_signal_handlers(void) {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM failed");
        exit(EXIT_FAILURE);
    }
}

void redraw_line(char *buffer, int *pos, int capacity, const char *new_text) {
    if (!new_text) return;

    str_copy(buffer, new_text);
    buffer[capacity - 1] = '\0';
    *pos = (int)str_len(buffer);

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\r\033[K\033[1;34m%s\033[0m> %s", cwd, new_text);
        fflush(stdout);
    }

}

int read_key(void) {
    char c;
    ssize_t nread;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno == EINTR && stop_requested) {
            return KEY_INTERRUPT;
        }
        if (nread == -1 && errno != EAGAIN && errno != EINTR) {
            perror("read error");
            exit(EXIT_FAILURE);
        }
    }

    if (c == '\033') {
        char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
            }
        }
        return '\033';
    }

    return (unsigned char)c;
}
char* read_input(History *history) {
    int pos = 0;
    int capacity = INIT_CAPACITY;

    char *buffer = malloc(INIT_CAPACITY * sizeof(char));

    if (!buffer) {
        perror("Allocation error");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0';

    while (1) {
        int key = read_key();

        if (key == KEY_INTERRUPT) {
            free(buffer);
            return NULL;
        }

        if (key == KEY_ENTER || key == '\n' || key == '\r') {
            buffer[pos] = '\0';
            write(STDOUT_FILENO, "\r\n", 2);
            return buffer;
        }

        if (key == KEY_CTRL_D) {
            if (pos == 0) {
                free(buffer);
                return NULL;
            }
            continue;
        }

        if (key == KEY_UP) {

            const char *prev_command = history_prev(history, buffer);

            if (str_len(prev_command) >= capacity) {
                capacity *= 2;
                char *new_buffer = realloc(buffer, capacity * sizeof(char));

                if (!new_buffer) {
                    free(buffer);
                    perror("Reallocation error");
                    exit(EXIT_FAILURE);
                }
                buffer = new_buffer;
            }

            redraw_line(buffer, &pos, capacity, prev_command);

            continue;
        }

        if (key == KEY_DOWN) {

            const char *next_command = history_next(history, buffer);

            if (next_command && str_len(next_command) >= capacity) {
                capacity *= 2;
                char *new_buffer = realloc(buffer, capacity * sizeof(char));

                if (!new_buffer) {
                    free(buffer);
                    perror("Reallocation error");
                    exit(EXIT_FAILURE);
                }
                buffer = new_buffer;
            }

            redraw_line(buffer, &pos, capacity, next_command);

            continue;
        }

        if (key == KEY_BACKSPACE || key == 8) {
            if (pos > 0) {
                pos--;
                write(STDOUT_FILENO, "\b \b", 3);
            }
            continue;
        }

        if (key >= 32 && key <= 126) {
            buffer[pos++] = (char)key;
            write(STDOUT_FILENO, &buffer[pos - 1], 1);

            if (pos >= capacity) {
                capacity *= 2;
                char *new_buffer = realloc(buffer, capacity * sizeof(char));

                if (!new_buffer) {
                    free(buffer);
                    perror("Reallocation error");
                    exit(EXIT_FAILURE);
                }
                buffer = new_buffer;
            }
        }

    }

}

int main() {
    setup_signal_handlers();
    enable_raw_mode();

    History *history = init_history();
    if (!history) {
        disable_raw_mode();
        return EXIT_FAILURE;
    }

    char cwd[PATH_MAX];

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("\033[1;34m%s\033[0m> ", cwd);
            fflush(stdout);
        }

        char *input = read_input(history);
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

        push_history(history, input);
        int exec_status = execute_ast(ast, history);

        if (exec_status < 0) {
            perror("execute_ast");
        }

        free_ast(ast);
        free_tokens(tokens);
        free(input);

        if (stop_requested) {
            break;
        }
    }

    free_history(history);
    disable_raw_mode();

    if (stop_requested) {
        return 130;
    }

    return 0;
}
