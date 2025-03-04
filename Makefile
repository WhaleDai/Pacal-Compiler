# Makefile for SPL-compiler
clean:
	rm -rf *.o
	rm lex.yy.c
	rm y.tab.c

# To compile your file set_of_lex.l --> lexer
#
lexer:  lex.yy.o test_tk.o outputTK.o set_of_lex.h
	gcc -o lexer lex.yy.o test_tk.o outputTK.o

test_tk.o: test_tk.c set_of_lex.h
	gcc -c test_tk.c

lex.yy.c: set_of_lex.l set_of_lex.h
	lex set_of_lex.l

lex.yy.o: lex.yy.c
	gcc -c lex.yy.c

outputTK.o: outputTK.c set_of_lex.h
	gcc -c outputTK.c


parser: test_parser.o func_of_parse.o y.tab.o lex.yy.o outputTK.o output.o symbol_table.o
	cc -o parser test_parser.o func_of_parse.o y.tab.o lex.yy.o outputTK.o output.o symbol_table.o -ll

test_parser.o: test_parser.c set_of_lex.h set_of_parse.h symbol_table.h output.c
	cc -c test_parser.c

func_of_parse.o: func_of_parse.c set_of_lex.h set_of_parse.h symbol_table.h output.c
	cc -c func_of_parse.c

y.tab.o: y.tab.c
	cc -c y.tab.c

y.tab.c: set_of_parse.y set_of_lex.h set_of_parse.h symbol_table.h
	yacc set_of_parse.y

output.o: output.c set_of_lex.h
	cc -c output.c

symbol_table.o: symbol_table.c set_of_lex.h symbol_table.h
	cc -c symbol_table.c


compiler: test_compiler.o func_of_parse.o y.tab.o lex.yy.o outputTK.o output.o symbol_table.o coding.o genAssemble.o
	cc -o compiler test_compiler.o func_of_parse.o y.tab.o lex.yy.o outputTK.o output.o symbol_table.o \
             coding.o genAssemble.o

test_compiler.o: test_compiler.c set_of_lex.h set_of_parse.h symbol_table.h genAssemble.h
	cc  -c test_compiler.c

genAssemble.o: genAssemble.c set_of_lex.h symbol_table.h genAssemble.h
	cc -c genAssemble.c

coding.o: coding.c set_of_lex.h symbol_table.h genAssemble.h
	cc -c coding.c

