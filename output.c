#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "set_of_parse.h"

char* opprint[]  = {
    " ", ".", "+", "-", "*", "/",
    "<", "<=", "=", "<>", ">", ">=", ":=",
    "and", "or", "not", "div", "mod", "^",
    "in", "if", "goto", "progn", "label", "funcall",
    "aref", "program", "float", "fix", "fundcaller",
};

int opsize[] = {
    1, 1, 1, 1, 1, 1,
    1, 2, 1, 2, 1, 2, 2,
    3, 2, 3, 3, 3, 1,
    2, 2, 4, 5, 5, 7,
    4, 7, 5, 3, 6,
};

/* find length of a string */
int strlength(char str[]) {
    int i, n;
    n = 16;
    for (i = 0; i < 16; i++)
        if ( str[i] == '\0' && n == 16 ) n = i;
    return n;
}

void ppexpr(TK tok)
{
    printexpr(tok, 0);
    printf("\n");
}

void printexpr(TK tok, int col)
{
  TK opnds;
  int nextcol, start, i;
  if (strcmp(tok->stringval, "function") == 0)
  {
    printf ("[->function ");
    nextcol = col + 10;
    opnds = tok->operands;
    start = 0;
    while (opnds != NULL)
    {
      if (start < 3)
        printf(" ");
      else 
      {
        printf("\n");
        for (i = 0; i < nextcol; i++)
          printf(" ");
      }
      if (start == 0)
        nextcol += 1 + strlength(opnds->stringval);
      printexpr(opnds, nextcol);
      start++;
      opnds = opnds->link;
    }
    printf ("]");
  }
  else if (strcmp(tok->stringval, "procedure") == 0)
  {
    printf ("[->procedure");
    nextcol = col + 11;
    opnds = tok->operands;
    start = 0;
    while (opnds != NULL)
    {
      if (start < 3)
        printf(" ");
      else
      {
        printf("\n");
        for (i = 0; i < nextcol; i++)
          printf(" ");
      }
      if (start == 0)
        nextcol += 1 + strlength(opnds->stringval);
      printexpr(opnds, nextcol);
      start++;
      opnds = opnds->link;
    }
    printf ("]");
  }
  else if (tok->tType == OPERATOR)
  {
    printf ("[->%s", opprint[tok->idval]);
    nextcol = col + 2 + opsize[tok->idval];
    opnds = tok->operands;
    start = 0;
    while (opnds != NULL)
    {
      if (start == 0)
        printf(" ");
      else
      {
        printf(" \n");
        for (i = 0; i < nextcol; i++)
          printf(" ");
      }
      printexpr(opnds, nextcol);
      if ( opnds->tType == TOKEN_ID && nextcol < 60 )
        nextcol = nextcol + 1 + strlength(opnds->stringval);
      else
        start = 1;
      opnds = opnds->link;
    }
    printf ("]");
  }
  else
    printtok(tok);
}

void printtok(TK tok)
{
  switch (tok->tType)
  {
    case TOKEN_ID:
      printf ("%s", tok->stringval);
      break;
    case TOKEN_STR:
      printf ("'%s'", tok->stringval);
      break;
    case RESERVED:
      printf ("%s", tok->stringval);
      break;
    case TOKEN_CHAR:
      printf ("'%c'", tok->charval);
      break;
    case TOKEN_NUM:
      switch (tok->dType)
      {
        case TYPE_INT:
          printf ("%d", tok->intval);
          break;
        case  TYPE_REAL:
          printf ("%e", tok->realval);
          break;
      }
      break;
    case DELIMITER:
    case OPERATOR:
      break;
  }
}
