#ifndef LR0_H
#define LR0_H

#include "grammar.h"

#define MAX_ITEMS 300
#define MAX_STATES 100
#define MAX_TRANSITIONS 300

typedef struct {
    int production_id;
    int dot_pos;
} Item;

typedef struct {
    Item items[MAX_ITEMS];
    int item_count;
} ItemSet;

typedef struct {
    int from_state;
    char symbol[MAX_SYMBOL_LEN];
    int to_state;
} Transition;

typedef struct {
    ItemSet states[MAX_STATES];
    int state_count;

    Transition transitions[MAX_TRANSITIONS];
    int transition_count;
} LR0Collection;

void build_lr0_collection(const Grammar *g, LR0Collection *c);
void print_lr0_collection(const Grammar *g, const LR0Collection *c);
void check_lr0_conflicts(const Grammar *g, const LR0Collection *c);
void export_dot(const Grammar *g, const LR0Collection *c, const char *filename);
void export_json(const Grammar *g, const LR0Collection *c, const char *filename);

#endif