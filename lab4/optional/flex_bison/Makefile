.PHONY: all run clean

all: expr_parser

expr.tab.c expr.tab.h: expr.y
	bison -d expr.y

lex.yy.c: expr.l expr.tab.h
	flex expr.l

expr_parser: expr.tab.c lex.yy.c
	gcc -o expr_parser expr.tab.c lex.yy.c

run: expr_parser
	echo "id + id * id" | ./expr_parser

clean:
	rm -f expr_parser expr.tab.c expr.tab.h lex.yy.c
