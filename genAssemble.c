#include <stdio.h>
#include <ctype.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "genAssemble.h"

int Assemble_entry(char name[], int size);
void Assemble_label(int label);
void Assemble_call(char name[]);
void Assemble_jump(int code, int label);
void Assemble_immed(int inst, int ival, int dstreg);
void Assemble_op(int inst);
void Assemble_rr(int inst, int srcreg, int dstreg);
void Assemble_ld(int inst, int off, int reg, char str[]);
void Assemble_st(int inst, int reg, int off, char str[]);
void Assemble_sttemp( int reg );
void Assemble_ldtemp( int reg );
void Assemble_ldr(int inst, int offset, int reg, int dstreg, char str[]);
void Assemble_ldrr(int inst, int offset, int reg, int dstreg, char str[]);
void Assemble_ldrrm(int inst, int offset, int reg, int mult, int dstreg, char str[]);
void Assemble_str(int inst, int srcreg, int offset, int reg, char str[]);
void Assemble_strr(int inst, int srcreg, int offset, int reg, char str[]);
void Assemble_strrm(int inst, int srcreg, int offset, int reg, int mult, char str[]);
void Assemble_ldflit(int inst, int label, int dstreg);
void Assemble_litarg(int label, int dstreg);
void Assemble_lshift(int srcreg, int n, int dstreg);
void Assemble_float(int reg, int freg);
void Assemble_fix(int freg, int reg);
void Assemble_fneg(int reg, int extrareg);
void Assemble_exit(char name[]);
void makeilit(int n, int label);
void makeflit(float f, int label);
void makeblit(char s[], int label);
void outlits();
void printcode(char* lines[]);
void Assemble_labelstr(char name[]);
void Assemble_1r(int inst, int reg);
void Assemble_st2(int inst, int off);

char* regpr[] = {"%eax", "%ecx", "%edx", "%ebx",
                 "%esi", "%edi", "%esp", "%ebp",
                 "%r8d", "%r9d", "%r10d", "%r11d",
                 "%r12d", "%r13d", "%r14d", "%r15d",
                 "%xmm0",  "%xmm1",   "%xmm2",  "%xmm3",
                 "%xmm4",  "%xmm5",  "%xmm6",  "%xmm7",
                 "%xmm8",  "%xmm9",  "%xmm10", "%xmm11",
                 "%xmm12", "%xmm13", "%xmm14", "%xmm15" };

char* dregpr[] = {"%rax", "%rcx", "%rdx", "%rbx",
                  "%rsi", "%rdi", "%rsp", "%rbp",
                  "%r8", "%r9", "%r10", "%r11",
                  "%r12", "%r13", "%r14", "%r15"  };

char* jumppr[]  = {"jmp", "jne", "je", "jge", "jl", "jg", "jle" };

char* jumpcompr[]  = {"", "if     !=", "if     ==", "if     >=", "if     <",
                      "if     >", "if     <=" };

 char* instpr[] =
 {"movl", "movsd", "movq", "cltq", "addl", "subl", "imull", "divl",
  "andl", "negl", "orl", "notl", "cmpl", "addsd", "subsd", "mulsd",
  "divsd", "negsd", "cmpq", "cmpsd","addq", "subq", "imulq", "andq", "negq", "orq", "notq"
  };
char* instcompr[] = {"->", "->",  "->", "sign-extend", "+", "-", "*", "/",
                     " and", "-", "or", "notl", "compare", "+", "-", "*",
                     "/", "-", "compare", "compare",
    "+",     "-",     "*",    "&",   "negq", "orq", "notq" };

char* topcodeb[] = {
  ".READY:",
  "	.startproc",
  "	pushq	%rbp",
  "	.offset 16",
  "	movq	%rsp, %rbp",
  "	.offset 6, -16",
  "	.register 6",
  ""};

char* topcodec[] = {
  "	movq	%rbx, %r9",
  ""};

char* bottomcode[] = {
  "	movq	%r9, %rbx",
  "        leave",
  "        ret",
  ".END",
  ""};

static int   iliterals[100];
static int   ilabels[100];
static float fliterals[100];
static int   flabels[100];
static char  bliterals[1000];
static int   blindex[100];
static int   blabels[100];
static int   nilit = 0;
static int   nflit = 0;
static int   nblit = 0;
static int   floatconst = 0;
static int   fnegused = 0;
static int   stackframesize = 0;


void printcode(char* lines[])
{
  int i = 0;
  while ( lines[i][0] != 0 )
  {
    printf("%s\n", lines[i]);
    i++;
  }
}


int Assemble_entry(char name[], int size)
{

     printf(".program_name  %s\n", name);
     printf("%s:\n", name);
     printcode(topcodeb);
     printf("        subq\t$%d, %%rsp \t  \n",stackframesize );
     printcode(topcodec);
     return stackframesize;
}

void Assemble_exit(char name[])
{
  printcode(bottomcode);
  outlits();
}

void Assemble_label(int label)
{
  printf("function%d:\n", label);
}
void Assemble_labelstr(char name[])
{
  printf("%s:\n", name);
}

void Assemble_call(char name[])
{
  printf("\tcall\t%s\n", name);
}

void Assemble_jump(int code, int label)
{
  printf("\t%s\t.funciton%d \t\t\n",jumppr[code], label);
}

char* regnm(int reg, int instr)
{
  return ( instr == MOVQ || instr == CMPQ ) ? dregpr[reg] : regpr[reg];
}

void Assemble_immed(int inst, int ival, int reg)
{
  printf("\t%s\t$%d,%s\n", instpr[inst], ival, regnm(reg, inst));
}

void Assemble_op(int inst)
{
  printf("\t%s\t \n", instpr[inst]);
}

void Assemble_rr(int inst, int srcreg, int dstreg)
{
  printf("\t%s\t%s,%s\n", instpr[inst], regnm(srcreg, inst),regnm(dstreg, inst));
}

void Assemble_ld(int inst, int off, int reg, char str[])
{
  printf("\t%s\t%d(%%rbp),%s\n", instpr[inst], off, regnm(reg, inst));
}

void Assemble_st(int inst, int reg, int off, char str[])
{
  printf("\t%s\t%s,%d(%%rbp)\n", instpr[inst], regnm(reg, inst), off);
}

void Assemble_st2(int inst, int off)
{
  printf("\t%s\t%s,%d(%%rbp)\n", instpr[inst], dregpr[RBP], off);
}

void Assemble_sttemp( int reg )
{
  Assemble_st( MOVSD, reg, -8, "temp");
}

void Assemble_ldtemp( int reg )
{
  Assemble_ld( MOVSD, -8, reg, "temp");
}

void Assemble_ldr(int inst, int offset, int reg, int dstreg, char str[])
{
  printf("\t%s\t%d(%s),%s\n", instpr[inst], offset, dregpr[reg],regnm(dstreg, inst));
}

void Assemble_ldrr(int inst, int offset, int reg, int dstreg, char str[])
{
  printf("\t%s\t%d(%%rbp,%s),%s\n", instpr[inst], offset, dregpr[reg],regnm(dstreg, inst));
}

void Assemble_ldrrm(int inst, int offset, int reg, int mult, int dstreg, char str[])
{
  printf("\t%s\t%d(%%rbp,%s,%d),%s\n", instpr[inst], offset, dregpr[reg], mult,regnm(dstreg, inst));
}

void Assemble_str(int inst, int srcreg, int offset, int reg, char str[])
{
  printf("\t%s\t%s,%d(%s)\n", instpr[inst], regnm(srcreg, inst), offset,dregpr[reg]);
}

void Assemble_strr(int inst, int srcreg, int offset, int reg, char str[])
{
  printf("\t%s\t%s,%d(%%rbp,%s)\n", instpr[inst], regnm(srcreg, inst), offset,dregpr[reg] );
}

void Assemble_strrm(int inst, int srcreg, int offset, int reg, int mult, char str[])
{
  printf("\t%s\t%s,%d(%%rbp,%s,%d)\n", instpr[inst], regnm(srcreg, inst), offset,dregpr[reg], mult);
}

void Assemble_ldflit(int inst, int label, int dstreg)
{
  int i;
  double d = 0.0;
  for (i=0; i<nflit; i++)
    if ( label == flabels[i] )  d = fliterals[i];
  printf("\t%s\t.func%d(%%rip),%s   \n", instpr[inst],label, regpr[dstreg]);
}

void Assemble_litarg(int label, int dstreg)
{
  printf("\tmovl\t$.func%d,%s\n",label, regpr[dstreg]);
}

void Assemble_float(int reg, int freg)
{
  printf("\tcvtsi2sd\t%s,%s\n", regpr[reg],regpr[freg]);
}

void Assemble_fix(int freg, int reg)
{
  printf("\tcvttsd2si\t%s,%s\n", regpr[freg],regpr[reg]);
}

void Assemble_fneg(int reg, int extrareg)
{
  fnegused = 1;
  Assemble_ldflit(MOVSD, 666, extrareg);
  printf("\txorpd\t%s,%s\n",regpr[extrareg], regpr[reg]);
}

void Assemble_1r(int inst, int reg)
{
  printf("\t%s\t%s\t\t\n", instpr[inst], regpr[reg] );
}

void makeilit(int n, int label)
{
  iliterals[nilit] = n;
  ilabels[nilit] = label;
  ++nilit;
}

void makeflit(float f, int label)
{
  fliterals[nflit] = f;
  flabels[nflit] = label;
  ++nflit;
}

void makeblit(char s[], int label)
{
    int index;
    int i=0, done=0;
    if ( nblit == 0 )
    { 
      index = 0;
      blindex[0] = 0;
    }
    else index = blindex[nblit];
    while ( i < 16 && done == 0 )
    { bliterals[index] = s[i];
      ++index;
      if ( s[i] == '\0' ) done = 1;
      ++i;
    };
    blabels[nblit] = label;
    blindex[nblit+1] = index;
    ++nblit;
}

void outlits()
{
  int i, j, start; double d; int ida, idb;
  for ( i = 0; i < nilit; ++i )
  {
    printf("\t.align  4\n");
    printf(".func%d:\n", ilabels[i]);
    printf("\t.long\t%d    \n", iliterals[i]);
  }
  for ( i = 0; i < nblit; ++i )
  {
    printf("\t.align  4\n");
    printf(".func%d:\n", blabels[i]);
    printf("\t.string\t\"%s\"\n", &bliterals[blindex[i]]);
  }
  for ( i = 0; i < nflit; ++i )
  {
    d = fliterals[i];
    printf("\t.align  8\n");
    printf(".func%d:\n", flabels[i]);
    printf("\t.long\t%d   \n", idb);
    printf("\t.long\t%d\n", ida);
  }
  printf("\n");
}
