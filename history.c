#include "history.h"

History* init_history(void) {
    History *history = malloc(sizeof(History));

    if (!history) {
        perror("Failed to allocate History struct");
        return NULL;
    }

    history->capacity = MAX_HIST_COUNT;
    history->head = 0;
    history->count = 0;
    history->nav_offset = 0;
    history->draft = NULL;

    history->entries = calloc(history->capacity, sizeof(char*));

    if (!history->entries) {
        perror("Failed to allocate history entries array");
        free(history);
        return NULL;
    }

    return history;
}

void history_reset_nav(History *history) {
    if (!history) return;
    history->nav_offset = 0;
    if (history->draft) {
        free(history->draft);
        history->draft = NULL;
    }
}

void push_history(History *history, const char *command) {

    if (!history || !command || *command == '\0') return;

    if (history->count > 0) {
        size_t last_idx = (history->head + history->count - 1) % history->capacity;
        if (history->entries[last_idx] && str_compare(history->entries[last_idx], command) == true) return;
    }

    char *dup = str_dup(command);
    if (!dup) return;

    if (history->count == history->capacity) {
        free(history->entries[history->head]);
        history->entries[history->head] = dup;
        history->head = (history->head + 1) % history->capacity;
    } else {
        size_t insert_idx = (history->head + history->count) % history->capacity;
        history->entries[insert_idx] = dup;
        history->count++;
    }

    history_reset_nav(history);

}

void print_history(History *history) {

    if (!history) return;

    for (size_t i = 0; i < history->count; i++) {
        size_t idx = (history->head + i) % history->capacity;
        printf("%s\n", history->entries[idx]);
    }
}

const char* history_prev(History *history, const char *current_input) {
    if (!history) return NULL;

    if (history->nav_offset == 0) {
        free(history->draft);
        history->draft = str_dup(current_input ? current_input : "");
        if (!history->draft) return "";
    }

    if (history->nav_offset >= (int)history->count) {
        if (history->count == 0) {
            history->nav_offset = 1;
            return history->draft;
        }

        return history->entries[history->head];
    }

    history->nav_offset++;

    size_t offset = history->count - history->nav_offset;
    size_t idx = (history->head + offset) % history->capacity;

    return history->entries[idx];
}

const char* history_next(History *history, const char *current_input) {
    if (!history) return NULL;

    if (history->nav_offset == 0) {
        return history->draft ? history->draft : "";
    }

    history->nav_offset--;

    if (history->nav_offset == 0) {
        return history->draft ? history->draft : "";
    }

    size_t offset = history->count - history->nav_offset;
    size_t idx = (history->head + offset) % history->capacity;

    return history->entries[idx];
}

void free_history(History *history) {
    if (!history) return;

    if (history->entries) {
        for (size_t i = 0; i < history->count; i++) {
            size_t idx = (history->head + i) % history->capacity;
            free(history->entries[idx]);
        }
        free(history->entries);
    }

    free(history->draft);
    free(history);
}
