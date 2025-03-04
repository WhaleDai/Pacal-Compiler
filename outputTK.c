#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include<linux/module.h>
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
            printf("  %10s      %4d     \t%2d\n",table2[tok->idval], tok->idval, tok->tType);
            break;
        case DELIMITER:
            printf("  %10s      %4d     \t%2d\n",table1[tok->idval], tok->idval, tok->tType);
           break;
        case RESERVED:
            printf("  %10s      %4d     \t%2d\n",table3[tok->idval], tok->idval, tok->tType);
            break;
        case TOKEN_ID:
        case TOKEN_STR:
           printf("  %10s        None      \t%2d\n", tok->stringval, tok->tType);
           break;
        case TOKEN_CHAR:
           printf("  %10s        None      \t%2d\n", tok->stringval, tok->tType);
           break;
        case TOKEN_NUM:
            switch (tok->dType)
            {
              case TYPE_INT:
                printf("  %12d      %4d     \t%2d\n", tok->intval, tok->dType, tok->tType);
                break;
              case  TYPE_REAL:
                printf("  %12e      %4d     \t%2d\n", tok->intval, tok->dType, tok->tType);
                break;
            }
          break;
     }
  }
