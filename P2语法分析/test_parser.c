#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "set_of_parse.h"
#include "output.c"


extern TK pres;

int main() {

    int res;
    initsyms();
    res = yyparse();
    output_symbol_table();
    printf("yyparse result = %8d\n", res);
    ppexpr(pres);

    return 0;
}
