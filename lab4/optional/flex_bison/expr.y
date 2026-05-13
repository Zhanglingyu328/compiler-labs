%{
#include <stdio.h>
#include <stdlib.h>
int yylex(void);
void yyerror(const char *s);
%}

%token ID
%left '+'
%left '*'

%%
input
    : expr { printf("语法分析成功\n"); }
    ;

expr
    : expr '+' term
    | term
    ;

term
    : term '*' factor
    | factor
    ;

factor
    : '(' expr ')'
    | ID
    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "语法错误: %s\n", s);
}

int main(void) {
    printf("请输入表达式，例如: id + id * id\n");
    return yyparse();
}
