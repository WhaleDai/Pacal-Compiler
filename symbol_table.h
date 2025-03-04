#ifndef SYMTAB_H
#define SYMTAB_H

#include "set_of_lex.h"

#define MAXBLOCKS    50
#define HASH_SIZE    27
#define MAXSTR       16
#define SYM_ARGM        0
#define SYM_BASIC       1
#define SYM_CONST       2
#define SYM_VAR         3
#define SYM_SUBRANGE    4
#define SYM_FUNCTION    5
#define SYM_ARRAY       6
#define SYM_RECORD      7
#define SYM_TYPE        8
#define SYM_POINTER     9
#define SYM_ARGLIST     10


typedef struct symtbr {
    struct symtbr *link;
    char   name[MAXSTR];
    int    kind;
    int    bType;
    struct symtbr *dType;
    int    blockLevel;
    int    size;
    int    offset;
    union  {
        char  stringValue[MAXSTR];
        int   intValue;
        double realValue;
        char charValue;
    } constval;
    int    lowBound;
    int    highBound;
} SYMBOLREC, *SYMBOL;


SYMBOL symalloc(void);
SYMBOL makesym(char name[]);
SYMBOL insertsym(char name[]);
SYMBOL searchlev(char name[], int level);
SYMBOL searchst(char name[]);
SYMBOL searchins(char name[]);
SYMBOL insertbt(char name[], int basictp, int siz);
SYMBOL insertfn(char name[], SYMBOL resulttp, SYMBOL argtp);
SYMBOL insertfnx(char name[], SYMBOL resulttp, SYMBOL arglist);
SYMBOL insertsymat(char name[], int level);
void pprintsym(SYMBOL sym, int col);
void ppsym(SYMBOL sym);
void printsymbol(SYMBOL sym);
void printstlevel(int level);
void output_symbol_table(void);
void initsyms(void);
void insert_label(int internal_id, TK label_tok);
int alignsize(SYMBOL sym);
int user_label_exists(TK label_tok);
int get_internal_id(int external_id);
int hashfunc(char name[]);

int blocknumber;
int blockoffs[MAXBLOCKS];
int basicsizes[5];
int contblock[MAXBLOCKS];
int lastblock;

#endif
