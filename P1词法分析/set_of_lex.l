%{
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include"set_of_lex.h"

int make_TK(int type,int id);
int make_id();
int make_real();
int make_int();
int make_str();
int yywrap();
void make_Err();
void make_cmt();

TK yylval;
int noline = 1;
%}

delim       [ \t]
ws          {delim}+
letter      [A-Za-z]
digit       [0-9]
id          {letter}(_|{letter}|{digit})*
integer     {digit}+
real        {digit}+(\.{digit}+)?(([E|e])[+\-]?{digit}+)?
comment1    ("{")
comment2    (\/\/.*\n)
string      [\']
%%

{ws}        { /* no action and no return */ }
{comment1}  { make_cmt(); }
{comment2}  { noline++; }
true|false|maxint { return make_TK(RESERVED, SYS_CON); }
abs|chr|odd|ord|pred|sqr|sqrt|succ { return make_TK(RESERVED, SYS_FUNCT); }
write|writeln { return make_TK(RESERVED, SYS_PROC); }
integer|boolean|char|real|string { return make_TK(RESERVED, SYS_TYPE); }

{id}        { return make_id(); }
{string}    { return make_str(); }
{integer}   { return make_int(); }
{real}      { return make_real(); }
.           { make_Err(); }

"\n"        { noline++; }
"("         { return make_TK(DELIMITER, LP); }
")"         { return make_TK(DELIMITER, RP); }
"["         { return make_TK(DELIMITER, LB); }
"]"         { return make_TK(DELIMITER, RB); }
".."        { return make_TK(DELIMITER, DOTDOT); }
","         { return make_TK(DELIMITER, COMMA); }
":"         { return make_TK(DELIMITER, COLON); }
";"         { return make_TK(DELIMITER, SEMI); }

"."         { return make_TK(OPERATOR, DOT); }
"+"         { return make_TK(OPERATOR, PLUS); }
"-"         { return make_TK(OPERATOR, MINUS); }
"*"         { return make_TK(OPERATOR, MUL); }
"/"         { return make_TK(OPERATOR, DIV_R); }
"<"         { return make_TK(OPERATOR, LT); }
"<="        { return make_TK(OPERATOR, LE); }
"="         { return make_TK(OPERATOR, EQ); }
"<>"        { return make_TK(OPERATOR, NE); }
">"         { return make_TK(OPERATOR, GT); }
">="        { return make_TK(OPERATOR, GE); }
":="        { return make_TK(OPERATOR, ASSIGN); }
"and"           { return make_TK(OPERATOR, AND); }
"or"              { return make_TK(OPERATOR, OR); }
"not"           { return make_TK(OPERATOR, NOT); }
"div"          { return make_TK(OPERATOR, DIV); }
"mod"|"%"       { return make_TK(OPERATOR, MOD); }

"array"             { return make_TK(RESERVED, ARRAY); }
"begin"             { return make_TK(RESERVED, BEGIN_T); }
"case"                { return make_TK(RESERVED, CASE); }
"const"             { return make_TK(RESERVED, CONST); }
"do"                      { return make_TK(RESERVED, DO); }
"downto"          { return make_TK(RESERVED, DOWNTO); }
"else"                { return make_TK(RESERVED, ELSE); }
"end"                   { return make_TK(RESERVED, END); }
"for"                   { return make_TK(RESERVED, FOR); }
"function"    { return make_TK(RESERVED, FUNCTION); }
"goto"                { return make_TK(RESERVED, GOTO); }
"if"                      { return make_TK(RESERVED, IF); }
"in"                      { return make_TK(RESERVED, IN); }
"of"                      { return make_TK(RESERVED, OF); }
"packed"          { return make_TK(RESERVED, PACKED); }
"procedure" { return make_TK(RESERVED, PROCEDURE); }
"program"      { return make_TK(RESERVED, PROGRAM); }
"read"                { return make_TK(RESERVED, READ); }
"record"          { return make_TK(RESERVED, RECORD); }
"repeat"          { return make_TK(RESERVED, REPEAT); }
"set"                   { return make_TK(RESERVED, SET); }
"then"                { return make_TK(RESERVED, THEN); }
"to"                      { return make_TK(RESERVED, TO); }
"type"                { return make_TK(RESERVED, TYPE); }
"until"             { return make_TK(RESERVED, UNTIL); }
"var"                   { return make_TK(RESERVED, VAR); }
"while"             { return make_TK(RESERVED, WHILE); }
"with"                { return make_TK(RESERVED, WITH); }

%%

void make_cmt()
{
    char c;
    while ((c = input()) != '}')
        if (c == '\n')
            noline++;
}

int make_TK(int type, int id)
{
    yylval = (TK) initilize();
    yylval->tType = type;
    int i=0;
    if (type == RESERVED)
    {
        yylval->idval = id - RESERVED_BIAS;
        for (; i < yyleng; i++)
            yylval->stringval[i] = tolower(yytext[i]);
        yylval->stringval[yyleng] = '\0';
    }
    else if (type == OPERATOR)
        yylval->idval = id - OPERATOR_BIAS;
    else if (type == DELIMITER)
        yylval->idval = id - DELIMITER_BIAS;
    else
        yylval->idval = id;

    return id;
}

int make_id()
{
    int i, c, n;
    yylval = (TK) initilize();
    yylval->tType = TOKEN_ID;
    n = yyleng;
    if (n > 15) n = 15;

    for (i=0; i < n; i++)
    {
        c = yytext[i];
        yylval->stringval[i] = c;
    }
    yylval->stringval[n] = '\0';

    return ID;
}

int make_str()
{
    yylval = (TK) initilize();
    char c;
    int len=0;
    while ((c = input()) != '\'')
    {
        if (len < 15)
            yylval->stringval[len] = c;
        len++;
    }
    if (len > 15)
        len = 15;

    if (len == 1)
    {
        yylval->charval = yylval->stringval[0];
        yylval->tType = TOKEN_CHAR;
        yylval->dType = TYPE_CHAR;
        return CONST_CHAR;
    }
    else
    {
        yylval->stringval[len] = '\0';
        yylval->tType = TOKEN_STR;
        yylval->dType = TYPE_STR;
        return CONST_STR;
    }

}

int make_real()
{
    yylval = (TK) initilize();
    yylval->tType = TOKEN_NUM;
    yylval->dType = TYPE_REAL;

    float num = 0;
    sscanf(yytext, "%f", &num);
    yylval->realval = num;
    return CONST_REAL;
}

int make_int()
{
    yylval = (TK) initilize();
    yylval->tType = TOKEN_NUM;
    yylval->dType = TYPE_INT;

    int num;
    sscanf(yytext, "%i", &num);
    yylval->intval = num;
    return CONST_INT;
}

void make_Err()
{
    printf("line %d Lex Error!\n", noline);
}

int yywrap()
{
    return 1;
}
