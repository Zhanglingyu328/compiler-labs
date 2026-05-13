#include <stdio.h>
#include <string.h>
#include "../include/grammar.h"
#include "../include/lr0.h"
#include "../include/scanner.h"

int main(int argc, char *argv[]) {
    const char *grammar_file = "data/grammar.txt";
    const char *source_file = "data/test.src";
    const char *token_file = "data/token.txt";

    printf("=========================================\n");
    printf(" Compiler Lab 3: LR(0) Item Collection\n");
    printf(" Author: Mohammad Maimun\n");
    printf("=========================================\n\n");

    if (argc >= 2 && strcmp(argv[1], "--scan") == 0) {
        printf("[Optional Part] Lab1 + Lab2 + Lab3 Pipeline\n");
        scan_source_to_tokens(source_file, token_file);
        printf("Token stream generated. You can use it as syntax input.\n\n");
    }

    Grammar grammar;
    LR0Collection collection;

    init_grammar(&grammar);
    read_grammar_from_file(&grammar, grammar_file);
    augment_grammar(&grammar);

    print_grammar(&grammar);

    build_lr0_collection(&grammar, &collection);
    print_lr0_collection(&grammar, &collection);
    check_lr0_conflicts(&grammar, &collection);

    export_json(&grammar, &collection, "data/lr0_output.json");
    export_dot(&grammar, &collection, "data/lr0_graph.dot");

    printf("\nExperiment finished successfully.\n");

    return 0;
}