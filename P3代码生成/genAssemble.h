#ifndef GENAssemble__H
#define GENAssemble__H

#define MINIMMEDIATE -2147483648
#define MAXIMMEDIATE  2147483647

#define RBASE 0
#define RMAX  3
#define FBASE 16
#define FMAX  31

#define  EAX   0
#define  RAX   0
#define  ECX   1
#define  EDX   2
#define  EBX   3

#define  ESI   4
#define  EDI   5
#define  ESP   6
#define  RSP   6
#define  EBP   7
#define  RBP   7
#define  XMM0  16

#define BYTE     0
#define HALFWORD 1
#define WORD     2
#define FLOAT    3
#define ADDR     4
#define WORDSIZE      4
#define FLOATSIZE     8
#define POINTERSIZE  16

#define  JMP  0
#define  JNE  1
#define  JE   2
#define  JGE  3
#define  JL   4
#define  JG   5
#define  JLE  6

#define MOVL   0
#define MOVSD  1
#define MOVQ   2
#define CLTQ   3
#define ADDL   4
#define SUBL   5
#define IMULL  6
#define DIVL    7
#define ANDL    8
#define NEGL    9
#define ORL    10

#define CMPL   12

#define ADDSD  13
#define SUBSD  14
#define MULSD  15
#define DIVSD  16
#define NEGSD  17
#define CMPQ   18
#define CMPSD  19
#define ADDQ   20
#define SUBQ   21
#define IMULQ  22
#define ANDQ   23
#define NEGQ   24
#define ORQ    25
#define NOTQ   26

void Assemble_ldr(int inst, int offset, int reg, int dstreg, char str[]);
void Assemble_ldrr(int inst, int offset, int reg, int dstreg, char str[]);
void Assemble_ldrrm(int inst, int offset, int reg, int mult, int dstreg, char str[]);
void Assemble_str(int inst, int srcreg, int offset, int reg, char str[]);
void Assemble_strr(int inst, int srcreg, int offset, int reg, char str[]);
void Assemble_label(int label);
void Assemble_call(char name[]);
void Assemble_jump(int code, int label);
void Assemble_immed(int inst, int ival, int dstreg);
void Assemble_strrm(int inst, int srcreg, int offset, int reg, int mult, char str[]);
void Assemble_ldflit(int inst, int label, int dstreg);
void Assemble_litarg(int label, int dstreg);
void Assemble_lshift(int srcreg, int n, int dstreg);
void Assemble_op(int inst);
void Assemble_rr(int inst, int srcreg, int dstreg);
void Assemble_ld(int inst, int off, int reg, char str[]);
void Assemble_st(int inst, int reg, int off, char str[]);
void Assemble_sttemp( int reg );
void Assemble_ldtemp( int reg );
void Assemble_float(int reg, int freg);
void Assemble_fix(int freg, int reg);
void Assemble_fneg(int reg, int extrareg);
void Assemble_exit(char name[]);
void Assemble_labelstr(char name[]);
void Assemble_1r(int inst, int reg);
void Assemble_st2(int inst, int off);
int Assemble_entry(char name[], int size);

void makeilit(int n, int label);
void makeflit(float f, int label);
void makeblit(char s[], int label);
void outlits();
void printcode(char* lines[]);

#endif
