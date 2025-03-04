#ifndef CODING_H
#define CODING_H

#include "set_of_parse.h"

int moveop(TK code);
int genfun(TK code);
int genop(TK code, int rhs_reg, int lhs_reg);
int getreg(int kind);
int genaref(TK code, int storereg);
int genarith(TK code);
int findfunop(char str[]);
int nonvolatile(int reg);
int num_funcalls_in_tree(TK tok, int num);
void clearreg();
void free_reg(int reg_num);
void genc(TK code);
void gencode(TK pcode, int varsize, int maxlabel);
void mark_reg_unused(int reg);
void reset_regs(void);
void restorereg();
void savereg();
void mark_reg_used(int reg);
boolean at_least_one_float(int lhs_reg, int rhs_reg);
boolean both_float(int lhs_reg, int rhs_reg);
boolean funcallin(TK code);
boolean is_equal(TK a, TK b);
boolean is_fp_reg(int reg_num);
boolean is_gen_purpose_reg(int reg_num);
boolean search_tree_str(TK tok, char str[]);

#endif
