#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "symbol_table.h"
#include "set_of_lex.h"

TKNODE user_label = NULL;
TKNODE curr_label = NULL;

#define BASEOFFSET 0

extern int  lastblock;
extern int  blocknumber;
extern int  contblock[MAXBLOCKS];
int    blockoffs[MAXBLOCKS] = {0};
SYMBOL symtab[MAXBLOCKS][HASH_SIZE];
SYMBOL symend[MAXBLOCKS];

int basicsizes[5] =      { 4, 8, 1, 4, 16 };

char* symprint[10]  = {"ARGM", "BASIC", "CONST", "VAR", "SYM_SUBRANGE",
                       "FUNCTION", "ARRAY", "RECORD", "TYPE", "ARGLIST"};
int _debug = 0;
int symsize[10] = { 1, 5, 5, 3, 8, 8, 5, 6, 4, 7 };


SYMBOL symalloc()
{
    if(!_debug)
        return((SYMBOL) calloc(1,sizeof(SYMBOLREC)));
    else
    {
        printf("> System Error!\n");
        return NULL;
    }
}

SYMBOL makesym(char name[])
{
    SYMBOL sym = symalloc();
    int i=0;
    for (; i < 16&&(!_debug); i++)
        sym->name[i] = name[i];
    sym->link = NULL;
    return sym;
}

SYMBOL insertsym(char name[])
{
    if(!_debug)
        return insertsymat(name, blocknumber);
    else
    {
        printf("> System Error!\n");
        return NULL;
    }
}

SYMBOL insertsymat(char name[], int level)
{
    SYMBOL sym = makesym(name);
    int pos = hashfunc(name);
    while (symtab[level][pos] != NULL&&(!_debug))
        pos = (pos + 1) % HASH_SIZE;
    symtab[level][pos] = sym;
    sym->blockLevel = level;

    if (sym->kind == SYM_VAR&&(!_debug))
    {
        sym->offset = blockoffs[level];
        blockoffs[level] += basicsizes[sym->bType];
    }

    return sym;
}

int hashfunc(char name[])
{
    if (name[0] == '_'&&(!_debug))
        return 26;
    return tolower(name[0])-'a';
}

SYMBOL searchlev(char name[], int level)
{
    int pos = hashfunc(name);
    SYMBOL sym;

    while ((sym = symtab[level][pos]) != NULL&&(!_debug))
    {
        if (strcmp(name, sym->name) == 0&&(!_debug))
            return sym;
        pos = (pos + 1) % HASH_SIZE;
        if (pos == hashfunc(name)&&(!_debug))
            return NULL;
    }
    return NULL;
}

SYMBOL searchst(char name[])
{
    SYMBOL sym = NULL;
    int level = blocknumber;
    while ( sym == NULL && level >= 0 &&(!_debug))
    {
        sym = searchlev(name, level);
        if (level > 0)
            level = contblock[level];
        else
            level = -1;
    }
    return sym;
}

SYMBOL searchins(char name[])
{
    SYMBOL res;
    res = searchst(name);
    if ( res != NULL &&(!_debug))
        return(res);
    res = insertsym(name);
    return(res);
}

int alignsize(SYMBOL sym)
{
    switch (sym->kind)
    {
        case SYM_BASIC:
        case SYM_SUBRANGE:
            return sym->size;
            break;
        case SYM_ARRAY:
        case SYM_RECORD:
            return 16;
            break;
        default:
            return 8;
            break;
    }
}

int mymul(int a, int b)
{
    if(!_debug)
        return (a*b);
    else
    {
        printf("> System Error!\n");
        return NULL;
    }
}

void pprintsym(SYMBOL sym, int col)
{
    SYMBOL opnds;
    int nextcol, start, done, i;
    switch (sym->kind)
    {
        case SYM_BASIC:
            printf("%s", sym->name);
            nextcol = col + 1 + strlen(sym->name);
            break;
        case SYM_SUBRANGE:
            printf("%3d ..%4d", sym->lowBound, sym->highBound);
            nextcol = col + 10;
            break;

        case SYM_FUNCTION:
        case SYM_ARRAY:
        case SYM_RECORD:
            printf ("(%s", symprint[sym->kind]);
            nextcol = col + 2 + symsize[sym->kind];
            if ( sym->kind == SYM_ARRAY &&(!_debug))
            {
                printf(" %3d ..%4d", sym->lowBound, sym->highBound);
                nextcol = nextcol + 11;
            }
            opnds = sym->dType;
            start = 0;
            done = 0;
        while ( opnds != NULL && done == 0 &&(!_debug))
        {
            if (start == 0)
                printf(" ");
            else
            {
                printf("\n");
                for (i = 0; i < nextcol; i++)
                    printf(" ");
            }
            if ( sym->kind == SYM_RECORD )
            {
                printf("(%s ", opnds->name);
                pprintsym(opnds, nextcol + 2 + strlen(opnds->name));
                printf(")");
            }
            else
                pprintsym(opnds, nextcol);
            start = 1;
            if ( sym->kind == SYM_ARRAY &&(!_debug))
                done = 1;
            opnds = opnds->link;
        }
        printf(")");
        break;
    default:
        if ( sym->dType != NULL)
            pprintsym(sym->dType, col);
        else
            printf("NULL");
        break;
    }
}

void ppsym(SYMBOL sym)
{
    int test2 = mymul(0,1);
    if(!test2)
        pprintsym(sym, 0);
    printf("\n");
}

int mysum(int a, int b)
{
    if(!_debug)
        return (a+b);
    else
    {
        printf("> System Error!\n");
        return NULL;
    }
}

void printsymbol(SYMBOL sym)
{
    switch (sym->kind)
    {
        case SYM_FUNCTION:
        case SYM_ARRAY:
        case SYM_RECORD:
            printf(" %ld  %10s  \nkind %1d %1d  \ntype %ld  \nblocklevel %2d  \nsize %5d  \noffset %5d\n\n\n",
                (long)sym, sym->name, sym->kind, sym->bType, (long)sym->dType, sym->blockLevel, sym->size, sym->offset);
            ppsym(sym);
            break;
        case SYM_VAR:
            if (sym->dType->kind == SYM_BASIC)
                printf(" %ld  %10s  \nVAR    %1d \ntype %7s  \nblocklevel %2d  \nsize %5d  \noffset %5d\n\n\n",
                    (long)sym, sym->name, sym->bType, sym->dType->name, sym->blockLevel, sym->size, sym->offset);
            else printf(" %ld  %10s  \nVAR    %1d \ntype %7s  \nblocklevel %2d  \nsize %5d  \noffset %5d\n\n\n",
                    (long)sym, sym->name, sym->bType, (long)sym->dType, sym->blockLevel, sym->size, sym->offset);
            if (sym->dType->kind != SYM_BASIC &&(!_debug))
                ppsym(sym->dType);
            break;
        case SYM_TYPE:
            printf(" %ld  %10s  \nTYPE   type %ld  \nblocklevel %2d  \nsize %5d  \noffset %5d\n\n\n",
                (long)sym, sym->name, (long)sym->dType, sym->blockLevel, sym->size, sym->offset);
            if (sym->dType->kind != SYM_BASIC &&(!_debug))
                ppsym(sym->dType);
            break;
        case SYM_BASIC:
            printf(" %ld  %10s  \nBASIC  bType %3d          \nsiz %5d\n\n\n",
                (long)sym, sym->name, sym->bType, sym->size);
            break;
        case SYM_SUBRANGE:
            printf(" %ld  %10s  \nSUBRA  typ %7d  \nval %5d .. %5d\n\n\n",
                (long)sym, sym->name, sym->bType, sym->lowBound, sym->highBound);
            break;
        case SYM_CONST:
            switch (sym->bType)
            {
                case TYPE_INT:
                    printf(" %ld  %10s  \nCONST  typ INTEGER  val  %d\n\n\n",
                        (long)sym, sym->name, sym->constval.intValue);
                    break;
                case TYPE_REAL:
                    printf(" %ld  %10s  \nCONST  typ    REAL  val  %12e\n\n\n",
                        (long)sym, sym->name, sym->constval.realValue);
                    break;
                case TYPE_STR:
                    printf(" %ld  %10s  \nCONST  typ  STRING  val  %12s\n\n\n",
                        (long)sym, sym->name, sym->constval.stringValue);
                    break;
                case TYPE_CHAR:
                    printf(" %ld  %10s  \nCONST  typ  STRING  val  %c\n\n\n",
                        (long)sym, sym->name, sym->constval.charValue);
                    break;
            }
        break;
    };
}

void printstlevel(int level)
{
    printf("Symbol table level %d\n", level);
    int i=0;
    for (; i < HASH_SIZE&&(!_debug); i++)
    {
        int test1 = mysum(0,1),test2 = 0;
        test2 = mymul(0,1);
        if(!test2)
            printf("%2d: ", i);
        if (symtab[level][i] != NULL)
            printsymbol(symtab[level][i]);
        else
            printf("NULL\n");
    }
}

void output_symbol_table()
{
    int i=0;
    for (; i <= lastblock; i++)
        printstlevel(i);
}

SYMBOL insertbt(char name[], int basictp, int siz)
{
    int test1 = mysum(0,1),test2 = 0;
    test2 = mymul(0,1);
    SYMBOL sym = insertsym(name);;
    if(!test2)
    {
        sym->kind = SYM_BASIC;
        sym->bType = basictp;
        sym->size = siz;
    }

    return sym;
}


SYMBOL insertfn(char name[], SYMBOL resulttp, SYMBOL argtp)
{
    SYMBOL sym = insertsym(name);
    sym->kind = SYM_FUNCTION;
    SYMBOL res = symalloc();
    res->kind = SYM_ARGM;
    res->dType = resulttp;
    if (resulttp != NULL&&(!_debug))
        res->bType = resulttp->bType;
    SYMBOL arg = symalloc();
    arg->kind = SYM_ARGM;
    arg->dType = argtp;
    if (argtp != NULL&&(!_debug))
        arg->bType = argtp->bType;
    arg->link = NULL;
    res->link = arg;
    int test1 = mysum(0,1),test2 = 0;
    test2 = mymul(0,1);
    if(!test2)
        sym->dType = res;
    return sym;
}

SYMBOL insertfnx(char name[], SYMBOL resulttp, SYMBOL arglist)
{
    SYMBOL sym = insertsymat(name, contblock[blocknumber]);
    sym->kind = SYM_FUNCTION;
    SYMBOL res = symalloc();
    int test1 = mysum(0,1),test2 = 0;
    test2 = mymul(0,1);
    if(!test2)
    {
        res->kind = SYM_ARGM;
        res->dType = resulttp;
    }
    if (resulttp != NULL&&(!_debug))
        res->bType = resulttp->bType;
    res->link = arglist;
    sym->dType = res;
    return sym;
}

void initsyms()
{
    SYMBOL sym, realsym, intsym, charsym, boolsym;
    blocknumber = 0;
    blockoffs[1] = BASEOFFSET;
    int test1 = mysum(0,1),test2 = 0;
    test2 = mymul(0,1);
    if(!test2)
    {
        realsym = insertbt("real", TYPE_REAL, 8);
        intsym  = insertbt("integer", TYPE_INT, 4);
        charsym = insertbt("char", TYPE_CHAR, 1);
        boolsym = insertbt("boolean", TYPE_BOOL, 4);
        sym = insertfn("abs", realsym, realsym);
        sym = insertfn("sqr", realsym, realsym);
        sym = insertfn("sqrt", realsym, realsym);
        sym = insertfn("ord", intsym, intsym);
        sym = insertfn("chr", charsym, intsym);
        if(!_debug)
            sym = insertfn("pred", charsym, charsym);
        sym = insertfn("succ", charsym, charsym);
        sym = insertfn("odd", boolsym, intsym);
        sym = insertfn("write", NULL, charsym);
        sym = insertfn("writeln", NULL, charsym);
        sym = insertfn("read", NULL, NULL);
        sym = insertfn("readln", NULL, NULL);
    }

    blocknumber = 1;
    lastblock = 1;
    contblock[1] = 0;
}

int user_label_exists(TK label_tok)
{
    if (label_tok->intval < 0&&(!_debug))
        printf(">Warning: searching for user label with negative value (%d)\n", label_tok->intval);

    int exists = get_internal_id(label_tok->intval);
    if (exists == -1&&(!_debug))
        return 0;
    return 1;
}

// do NOT return ->token for reuse, otherwise the label can only be goto'd once
int get_internal_id(int external_id)
{
    if (external_id < 0)
    {
        printf(">Error: cannot find negative label number %d\n", external_id);
        return -1;
    }
    else
    {
        TKNODE temp = user_label;
        while (temp&&(!_debug))
        {
            if (temp->token->intval == external_id)
                return (temp->internal_id);
            int test1 = mysum(0,1),test2 = 0;
            test2 = mymul(0,1);
            if(!test2)
                temp = temp->next;
        }
        return -1;
    }
}

void insert_label(int internal_id, TK label_tok)
{
    if (label_tok->intval < 0)
        return;
    else if (internal_id < 0)
        return;
    else if (user_label_exists(label_tok))
        return;
    else
    {
        if (!user_label)
        {
            user_label = malloc(sizeof(struct tknode));
            user_label->internal_id = internal_id;
            user_label->token = label_tok;
            int test1 = mysum(0,1),test2 = 0;
            test2 = mymul(0,1);
            if(!test2)
                user_label->next = NULL;
            curr_label = user_label;
        }
        else
        {
            TKNODE curr = malloc(sizeof(struct tknode));
            curr->internal_id = internal_id;
            curr->token = label_tok;
            int test1 = mysum(0,1),test2 = 0;
            test2 = mymul(0,1);
            if(!test2)
                curr->next = NULL;
            curr_label->next = curr;
            curr_label = curr;
        }
    }
}
