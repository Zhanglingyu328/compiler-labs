%{
#include <stdio.h>
%}

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTNUM      [+-]?{DIGIT}+
FLOATNUM    [+-]?{DIGIT}*"."{DIGIT}+|[+-]?{DIGIT}+"."{DIGIT}*

%%

"int"       { printf("(INT, int)\n"); }
"float"     { printf("(FLOAT, float)\n"); }
"void"      { printf("(VOID, void)\n"); }
"if"        { printf("(IF, if)\n"); }
"else"      { printf("(ELSE, else)\n"); }
"while"     { printf("(WHILE, while)\n"); }
"return"    { printf("(RETURN, return)\n"); }
"input"     { printf("(INPUT, input)\n"); }
"print"     { printf("(PRINT, print)\n"); }

"++"        { printf("(AAA, ++)\n"); }
"--"        { printf("(AAS, --)\n"); }

"<="        { printf("(ROP, <=)\n"); }
">="        { printf("(ROP, >=)\n"); }
"=="        { printf("(ROP, ==)\n"); }
"!="        { printf("(ROP, !=)\n"); }
"<"         { printf("(ROP, <)\n"); }
">"         { printf("(ROP, >)\n"); }

"="         { printf("(ASG, =)\n"); }

"+"         { printf("(ADD, +)\n"); }
"-"         { printf("(ADD, -)\n"); }
"*"         { printf("(MUL, *)\n"); }
"/"         { printf("(MUL, /)\n"); }

"("         { printf("(LPA, ()\n"); }
")"         { printf("(RPA, ))\n"); }
"["         { printf("(LBK, [)\n"); }
"]"         { printf("(RBK, ])\n"); }
"{"         { printf("(LBR, {)\n"); }
"}"         { printf("(RBR, })\n"); }
","         { printf("(CMA, ,)\n"); }
";"         { printf("(SCO, ;)\n"); }

{FLOATNUM}  { printf("(FLO, %s)\n", yytext); }
{INTNUM}    { printf("(INT, %s)\n", yytext); }
{ID}        { printf("(ID, %s)\n", yytext); }

[ \t\r\n]+  { /* skip whitespace */ }

"//".*      { /* skip line comment */ }

.           { printf("(ERROR, %s)\n", yytext); }

%%

int yywrap() {
    return 1;
}

int main() {
    yylex();
    return 0;
}