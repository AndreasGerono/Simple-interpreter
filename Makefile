# Simple makefile to compile bison with flex
CC=gcc

calculator: parser.y lexer.l calculator.h
			bison -d -Wcounterexamples parser.y 
			flex lexer.l
			$(CC) -o $@ parser.tab.c lex.yy.c calculator.c

clean:
		rm -r calculator *.tab.* *.yy.c
