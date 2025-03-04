#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "set_of_lex.h"


static char* table1[] = { " ", "(", ")", "[", "]", "..", ",", ":", ";"} ;

static char* table2[]  = {" ", ".", "+", "-", "*", "/", "<", "<=", "=",
                          "<>", ">", ">=", ":=","and", "or", "not", "div",
                          "mod"};

static char* table3[] = { " ", "array", "begin", "case", "const", "do",
                           "downto", "else", "end", "for","function",
                           "goto", "if", "in","of", "packed", "procedure",
                           "program", "read","record","repeat", "set",
                           "then", "to", "type","until", "var", "while", "with",
                          "SYS_CON", "SYS_FUNCT", "SYS_PROC", "SYS_TYPE"
                        };

TK initilize()
{
    TK tok = (TK) calloc(1,sizeof(struct tk));
    if ( tok != NULL )
      return (tok);
    else
    {
        printf("initilize failed!\n");
        exit(-1);
    }
}

void outputTK(TK tok)
{
    switch (tok->tType)
    {
        case OPERATOR:
            printf ("tType: %2d  id: %4d   %10s\n",tok->tType, tok->idval,table2[tok->idval] );
            break;
        case DELIMITER:
            printf ("tType: %2d  id: %4d   %10s\n",tok->tType, tok->idval,table1[tok->idval] );
           break;
        case RESERVED:
           printf ("tType: %2d  id: %4d   %10s\n",tok->tType, tok->idval,table3[tok->idval] );
           break;
        case TOKEN_ID:
        case TOKEN_STR:
           printf ("tType: %2d  value:  %16s\n",tok->tType, tok->stringval);
           break;
        case TOKEN_CHAR:
           printf ("tType: %2d  value:  %16c\n",tok->tType, tok->charval);
           break;
        case TOKEN_NUM:
            switch (tok->dType)
            {
              case TYPE_INT:
                printf ("tType: %2d  type:  %4d %12d\n",tok->tType, tok->dType, tok->intval);
                break;
              case  TYPE_REAL:
                printf ("tType: %2d  type:  %4d %12e\n",tok->tType, tok->dType, tok->realval);
                break;
            }
          break;
     }
  }
