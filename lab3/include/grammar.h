#ifndef GRAMMAR_H
#define GRAMMAR_H

#define MAX_PRODUCTIONS 100
#define MAX_SYMBOLS 20
#define MAX_SYMBOL_LEN 32
#define MAX_LINE_LEN 256

typedef struct {
    char left[MAX_SYMBOL_LEN];
    char right[MAX_SYMBOLS][MAX_SYMBOL_LEN];
    int right_count;
} Production;

typedef struct {
    Production productions[MAX_PRODUCTIONS];
    int production_count;

    char start_symbol[MAX_SYMBOL_LEN];
    char augmented_start[MAX_SYMBOL_LEN];
} Grammar;

void init_grammar(Grammar *g);
void read_grammar_from_file(Grammar *g, const char *filename);
void augment_grammar(Grammar *g);
void print_grammar(const Grammar *g);
int is_nonterminal(const Grammar *g, const char *symbol);
void collect_symbols(const Grammar *g, char symbols[][MAX_SYMBOL_LEN], int *count);

#endif