#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "set_of_lex.h"

extern TK yylval;

int main()
  {
    int res, flag = 0;
    printf("Output lexer result:\n");
    printf("    TypeName      TypeId      TypeValue\n\n");
    while (!flag)
    {
        if ((res = yylex()) != 0)
        {
          outputTK(yylval);
        }
        else
          flag = 1;
    }
    return 0;
  }
