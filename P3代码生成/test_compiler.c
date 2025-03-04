#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "set_of_parse.h"
#include "output.c"
#include "coding.h"
#include "genAssemble.h"

extern TK pres;
extern int labelnumber;

int main()
{
	int res;
	initsyms();
	res = yyparse();
	gencode(pres, blockoffs[blocknumber], labelnumber-1);
	return 0;
}
