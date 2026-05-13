#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/grammar.h"

static void trim(char *str) {
    int start = 0;
    int end = (int)strlen(str) - 1;

    while (isspace((unsigned char)str[start])) {
        start++;
    }

    while (end >= start && isspace((unsigned char)str[end])) {
        str[end] = '\0';
        end--;
    }

    if (start > 0) {
        memmove(str, str + start, strlen(str + start) + 1);
    }
}

void init_grammar(Grammar *g) {
    g->production_count = 0;
    g->start_symbol[0] = '\0';
    g->augmented_start[0] = '\0';
}

static void add_production(Grammar *g, const char *left, const char *right_text) {
    if (g->production_count >= MAX_PRODUCTIONS) {
        printf("Error: too many productions.\n");
        exit(1);
    }

    Production *p = &g->productions[g->production_count++];
    strcpy(p->left, left);
    p->right_count = 0;

    char buffer[MAX_LINE_LEN];
    strcpy(buffer, right_text);
    trim(buffer);

    char *token = strtok(buffer, " \t\r\n");

    while (token != NULL) {
        if (p->right_count >= MAX_SYMBOLS) {
            printf("Error: too many symbols in production: %s -> %s\n", left, right_text);
            exit(1);
        }

        strcpy(p->right[p->right_count++], token);
        token = strtok(NULL, " \t\r\n");
    }
}

void read_grammar_from_file(Grammar *g, const char *filename) {
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        printf("Error: cannot open grammar file: %s\n", filename);
        exit(1);
    }

    char line[MAX_LINE_LEN];
    int first_rule = 1;

    while (fgets(line, sizeof(line), fp)) {
        trim(line);

        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        char *arrow = strstr(line, "->");

        if (!arrow) {
            printf("Error: invalid grammar line: %s\n", line);
            exit(1);
        }

        char left[MAX_SYMBOL_LEN];
        int left_len = (int)(arrow - line);

        if (left_len >= MAX_SYMBOL_LEN) {
            printf("Error: left symbol is too long: %s\n", line);
            exit(1);
        }

        strncpy(left, line, left_len);
        left[left_len] = '\0';
        trim(left);

        if (strlen(left) == 0) {
            printf("Error: empty left symbol in line: %s\n", line);
            exit(1);
        }

        if (first_rule) {
            strcpy(g->start_symbol, left);
            first_rule = 0;
        }

        char right_part[MAX_LINE_LEN];
        strcpy(right_part, arrow + 2);
        trim(right_part);

        if (strlen(right_part) == 0) {
            printf("Error: empty right part in line: %s\n", line);
            exit(1);
        }

        /*
         * Important:
         * Do not use nested strtok here.
         * add_production() already uses strtok to split symbols.
         * If read_grammar_from_file() also uses strtok to split '|',
         * the outer strtok state will be overwritten.
         *
         * Therefore, use strchr() manually to split alternatives.
         */
        char *start = right_part;

        while (1) {
            char *bar = strchr(start, '|');
            char alternative[MAX_LINE_LEN];

            if (bar != NULL) {
                int len = (int)(bar - start);

                if (len >= MAX_LINE_LEN) {
                    printf("Error: alternative is too long.\n");
                    exit(1);
                }

                strncpy(alternative, start, len);
                alternative[len] = '\0';
            } else {
                strcpy(alternative, start);
            }

            trim(alternative);

            if (strlen(alternative) > 0) {
                add_production(g, left, alternative);
            }

            if (bar == NULL) {
                break;
            }

            start = bar + 1;
        }
    }

    fclose(fp);

    if (g->production_count == 0) {
        printf("Error: grammar file is empty.\n");
        exit(1);
    }
}

void augment_grammar(Grammar *g) {
    Grammar old = *g;

    g->production_count = 0;

    /*
     * Build augmented start symbol.
     * Example:
     * E  -> E'
     * S  -> S'
     */
    strncpy(g->augmented_start, old.start_symbol, MAX_SYMBOL_LEN - 2);
    g->augmented_start[MAX_SYMBOL_LEN - 2] = '\0';
    strcat(g->augmented_start, "'");

    Production *p = &g->productions[g->production_count++];
    strcpy(p->left, g->augmented_start);
    strcpy(p->right[0], old.start_symbol);
    p->right_count = 1;

    for (int i = 0; i < old.production_count; i++) {
        if (g->production_count >= MAX_PRODUCTIONS) {
            printf("Error: too many productions after augmentation.\n");
            exit(1);
        }

        g->productions[g->production_count++] = old.productions[i];
    }

    strcpy(g->start_symbol, old.start_symbol);
}

void print_grammar(const Grammar *g) {
    printf("========== Augmented Grammar ==========\n");

    for (int i = 0; i < g->production_count; i++) {
        const Production *p = &g->productions[i];

        printf("%d. %s ->", i, p->left);

        for (int j = 0; j < p->right_count; j++) {
            printf(" %s", p->right[j]);
        }

        printf("\n");
    }

    printf("\n");
}

int is_nonterminal(const Grammar *g, const char *symbol) {
    for (int i = 0; i < g->production_count; i++) {
        if (strcmp(g->productions[i].left, symbol) == 0) {
            return 1;
        }
    }

    return 0;
}

static int symbol_exists(char symbols[][MAX_SYMBOL_LEN], int count, const char *symbol) {
    for (int i = 0; i < count; i++) {
        if (strcmp(symbols[i], symbol) == 0) {
            return 1;
        }
    }

    return 0;
}

void collect_symbols(const Grammar *g, char symbols[][MAX_SYMBOL_LEN], int *count) {
    *count = 0;

    for (int i = 0; i < g->production_count; i++) {
        const Production *p = &g->productions[i];

        if (!symbol_exists(symbols, *count, p->left)) {
            strcpy(symbols[*count], p->left);
            (*count)++;
        }

        for (int j = 0; j < p->right_count; j++) {
            if (!symbol_exists(symbols, *count, p->right[j])) {
                strcpy(symbols[*count], p->right[j]);
                (*count)++;
            }
        }
    }
}