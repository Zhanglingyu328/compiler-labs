#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../include/scanner.h"

void scan_source_to_tokens(const char *source_file, const char *token_file) {
    FILE *in = fopen(source_file, "r");
    FILE *out = fopen(token_file, "w");

    if (!in) {
        printf("Error: cannot open source file: %s\n", source_file);
        exit(1);
    }

    if (!out) {
        printf("Error: cannot open token file: %s\n", token_file);
        fclose(in);
        exit(1);
    }

    int ch;

    while ((ch = fgetc(in)) != EOF) {
        if (ch == '\n') {
            fprintf(out, "\n");
            continue;
        }

        if (isspace(ch)) {
            continue;
        }

        if (isalpha(ch) || ch == '_') {
            while ((ch = fgetc(in)) != EOF && (isalnum(ch) || ch == '_')) {
            }

            if (ch != EOF) {
                ungetc(ch, in);
            }

            fprintf(out, "id ");
        } else if (isdigit(ch)) {
            while ((ch = fgetc(in)) != EOF && isdigit(ch)) {
            }

            if (ch != EOF) {
                ungetc(ch, in);
            }

            fprintf(out, "num ");
        } else if (ch == '+') {
            fprintf(out, "+ ");
        } else if (ch == '*') {
            fprintf(out, "* ");
        } else if (ch == '(') {
            fprintf(out, "( ");
        } else if (ch == ')') {
            fprintf(out, ") ");
        } else {
            printf("Scanner warning: unknown character '%c'\n", ch);
        }
    }

    fprintf(out, "\n");

    fclose(in);
    fclose(out);

    printf("Scanner output saved to: %s\n", token_file);
}