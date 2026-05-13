#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lr0.h"

static int item_equal(Item a, Item b) {
    return a.production_id == b.production_id && a.dot_pos == b.dot_pos;
}

static int item_exists(const ItemSet *set, Item item) {
    for (int i = 0; i < set->item_count; i++) {
        if (item_equal(set->items[i], item)) {
            return 1;
        }
    }
    return 0;
}

static void add_item(ItemSet *set, Item item) {
    if (!item_exists(set, item)) {
        set->items[set->item_count++] = item;
    }
}

static int itemset_equal(const ItemSet *a, const ItemSet *b) {
    if (a->item_count != b->item_count) {
        return 0;
    }

    for (int i = 0; i < a->item_count; i++) {
        if (!item_exists(b, a->items[i])) {
            return 0;
        }
    }

    return 1;
}

static int state_exists(const LR0Collection *c, const ItemSet *set) {
    for (int i = 0; i < c->state_count; i++) {
        if (itemset_equal(&c->states[i], set)) {
            return i;
        }
    }
    return -1;
}

static void print_item(FILE *out, const Grammar *g, Item item) {
    Production p = g->productions[item.production_id];

    fprintf(out, "%s ->", p.left);

    for (int i = 0; i < p.right_count; i++) {
        if (i == item.dot_pos) {
            fprintf(out, " ·");
        }
        fprintf(out, " %s", p.right[i]);
    }

    if (item.dot_pos == p.right_count) {
        fprintf(out, " ·");
    }
}

static void closure(const Grammar *g, ItemSet *set) {
    int changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < set->item_count; i++) {
            Item item = set->items[i];
            Production p = g->productions[item.production_id];

            if (item.dot_pos >= p.right_count) {
                continue;
            }

            char *symbol_after_dot = p.right[item.dot_pos];

            if (is_nonterminal(g, symbol_after_dot)) {
                for (int k = 0; k < g->production_count; k++) {
                    if (strcmp(g->productions[k].left, symbol_after_dot) == 0) {
                        Item new_item;
                        new_item.production_id = k;
                        new_item.dot_pos = 0;

                        if (!item_exists(set, new_item)) {
                            add_item(set, new_item);
                            changed = 1;
                        }
                    }
                }
            }
        }
    }
}

static ItemSet goto_set(const Grammar *g, const ItemSet *set, const char *symbol) {
    ItemSet result;
    result.item_count = 0;

    for (int i = 0; i < set->item_count; i++) {
        Item item = set->items[i];
        Production p = g->productions[item.production_id];

        if (item.dot_pos < p.right_count &&
            strcmp(p.right[item.dot_pos], symbol) == 0) {
            Item moved;
            moved.production_id = item.production_id;
            moved.dot_pos = item.dot_pos + 1;
            add_item(&result, moved);
        }
    }

    if (result.item_count > 0) {
        closure(g, &result);
    }

    return result;
}

static void add_transition(LR0Collection *c, int from, const char *symbol, int to) {
    Transition *t = &c->transitions[c->transition_count++];
    t->from_state = from;
    strcpy(t->symbol, symbol);
    t->to_state = to;
}

void build_lr0_collection(const Grammar *g, LR0Collection *c) {
    c->state_count = 0;
    c->transition_count = 0;

    ItemSet start_set;
    start_set.item_count = 0;

    Item start_item;
    start_item.production_id = 0;
    start_item.dot_pos = 0;

    add_item(&start_set, start_item);
    closure(g, &start_set);

    c->states[c->state_count++] = start_set;

    char symbols[200][MAX_SYMBOL_LEN];
    int symbol_count = 0;
    collect_symbols(g, symbols, &symbol_count);

    int processed = 0;

    while (processed < c->state_count) {
        ItemSet current = c->states[processed];

        for (int i = 0; i < symbol_count; i++) {
            ItemSet next = goto_set(g, &current, symbols[i]);

            if (next.item_count == 0) {
                continue;
            }

            int index = state_exists(c, &next);

            if (index == -1) {
                index = c->state_count;
                c->states[c->state_count++] = next;
            }

            add_transition(c, processed, symbols[i], index);
        }

        processed++;
    }
}

static int is_kernel_item(Item item) {
    if (item.production_id == 0) {
        return 1;
    }
    return item.dot_pos > 0;
}

void print_lr0_collection(const Grammar *g, const LR0Collection *c) {
    printf("========== Canonical Collection of LR(0) Items ==========\n");

    for (int i = 0; i < c->state_count; i++) {
        printf("I%d:\n", i);

        printf("  Kernel Items:\n");
        for (int j = 0; j < c->states[i].item_count; j++) {
            Item item = c->states[i].items[j];

            if (is_kernel_item(item)) {
                printf("    ");
                print_item(stdout, g, item);
                printf("\n");
            }
        }

        printf("  Closure Items:\n");
        for (int j = 0; j < c->states[i].item_count; j++) {
            printf("    ");
            print_item(stdout, g, c->states[i].items[j]);
            printf("\n");
        }

        printf("  Goto:\n");
        int has_goto = 0;

        for (int j = 0; j < c->transition_count; j++) {
            Transition t = c->transitions[j];

            if (t.from_state == i) {
                printf("    Goto(I%d, %s) = I%d\n", i, t.symbol, t.to_state);
                has_goto = 1;
            }
        }

        if (!has_goto) {
            printf("    None\n");
        }

        printf("\n");
    }
}

void check_lr0_conflicts(const Grammar *g, const LR0Collection *c) {
    printf("========== LR(0) Conflict Check ==========\n");

    int has_conflict = 0;

    for (int i = 0; i < c->state_count; i++) {
        int reduce_count = 0;
        int shift_count = 0;

        for (int j = 0; j < c->states[i].item_count; j++) {
            Item item = c->states[i].items[j];
            Production p = g->productions[item.production_id];

            if (item.dot_pos == p.right_count) {
                reduce_count++;
            } else {
                shift_count++;
            }
        }

        if (reduce_count >= 2) {
            printf("[Reduce-Reduce Conflict] State I%d has %d reduce items.\n",
                   i, reduce_count);
            has_conflict = 1;
        }

        if (reduce_count >= 1 && shift_count >= 1) {
            printf("[Shift-Reduce Conflict] State I%d has reduce item and shift item.\n",
                   i);
            has_conflict = 1;
        }
    }

    if (!has_conflict) {
        printf("No LR(0) conflict found. The grammar is LR(0) under current detection.\n");
    } else {
        printf("Conflict detected. The grammar is not pure LR(0), or needs SLR(1) follow-set filtering.\n");
    }

    printf("\n");
}

void export_dot(const Grammar *g, const LR0Collection *c, const char *filename) {
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        printf("Error: cannot write DOT file: %s\n", filename);
        return;
    }

    fprintf(fp, "digraph LR0 {\n");
    fprintf(fp, "    rankdir=LR;\n");
    fprintf(fp, "    node [shape=box];\n");

    for (int i = 0; i < c->state_count; i++) {
        fprintf(fp, "    I%d [label=\"I%d", i, i);

        for (int j = 0; j < c->states[i].item_count; j++) {
            Production p = g->productions[c->states[i].items[j].production_id];
            Item item = c->states[i].items[j];

            fprintf(fp, "\\n%s ->", p.left);

            for (int k = 0; k < p.right_count; k++) {
                if (k == item.dot_pos) {
                    fprintf(fp, " ·");
                }
                fprintf(fp, " %s", p.right[k]);
            }

            if (item.dot_pos == p.right_count) {
                fprintf(fp, " ·");
            }
        }

        fprintf(fp, "\"];\n");
    }

    for (int i = 0; i < c->transition_count; i++) {
        Transition t = c->transitions[i];
        fprintf(fp, "    I%d -> I%d [label=\"%s\"];\n",
                t.from_state, t.to_state, t.symbol);
    }

    fprintf(fp, "}\n");
    fclose(fp);

    printf("DOT graph exported to: %s\n", filename);
}

void export_json(const Grammar *g, const LR0Collection *c, const char *filename) {
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        printf("Error: cannot write JSON file: %s\n", filename);
        return;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"states\": [\n");

    for (int i = 0; i < c->state_count; i++) {
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"id\": \"I%d\",\n", i);

        fprintf(fp, "      \"items\": [\n");
        for (int j = 0; j < c->states[i].item_count; j++) {
            Item item = c->states[i].items[j];
            Production p = g->productions[item.production_id];

            fprintf(fp, "        \"");
            fprintf(fp, "%s ->", p.left);

            for (int k = 0; k < p.right_count; k++) {
                if (k == item.dot_pos) {
                    fprintf(fp, " ·");
                }
                fprintf(fp, " %s", p.right[k]);
            }

            if (item.dot_pos == p.right_count) {
                fprintf(fp, " ·");
            }

            fprintf(fp, "\"");

            if (j != c->states[i].item_count - 1) {
                fprintf(fp, ",");
            }

            fprintf(fp, "\n");
        }
        fprintf(fp, "      ],\n");

        fprintf(fp, "      \"goto\": {");
        int first = 1;

        for (int j = 0; j < c->transition_count; j++) {
            Transition t = c->transitions[j];

            if (t.from_state == i) {
                if (!first) {
                    fprintf(fp, ", ");
                }
                fprintf(fp, "\"%s\": \"I%d\"", t.symbol, t.to_state);
                first = 0;
            }
        }

        fprintf(fp, "}\n");
        fprintf(fp, "    }");

        if (i != c->state_count - 1) {
            fprintf(fp, ",");
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);

    printf("JSON result exported to: %s\n", filename);
}