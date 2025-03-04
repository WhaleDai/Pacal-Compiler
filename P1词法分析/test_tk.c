#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "set_of_lex.h"

extern TK yylval;

int main()
  {
    int res, flag = 0;
    printf("Started scanner test.\n");
    while (!flag)
    {
        if ((res = yylex()) != 0)
        {
          printf("yylex() = %4d   ", res);
          outputTK(yylval);
        }
        else
          flag = 1;
    }
    return 0;
  }
