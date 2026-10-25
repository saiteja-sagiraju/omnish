#ifndef HISTORY_H
#define HISTORY_H

#include <stdlib.h>
#include <stdio.h>
#include "string.h"

#define MAX_HIST_COUNT 1000

typedef struct History {
    char **entries;
    size_t capacity;
    size_t head;
    size_t count;

    int nav_offset;
    char *draft;
} History;

History* init_history(void);
void history_reset_nav(History *history);
void push_history(History *history, const char *command);
void print_history(History *history);
const char* history_prev(History *history, const char *current_input);
const char* history_next(History *history, const char *current_input);
void free_history(History *history);

#endif
