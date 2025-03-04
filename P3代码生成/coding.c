
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "genAssemble.h"
#include "coding.h"
#include "output.c"

#define NUM_INT_REGS    8
#define NUM_FP_REGS     24
#define NUM_REGS        32

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
void Assemble_labelstr(char name[]);
void Assemble_1r(int inst, int reg);
void Assemble_st2(int inst, int off);
void Assemble_exit(char name[]);
void makeilit(int n, int label);
void makeflit(float f, int label);
void makeblit(char s[], int label);
void outlits();
void printcode(char* lines[]);
void genc(TK code);
void print_used_regs();
int Assemble_entry(char name[], int size);
int symbol_is_null_int(char *str);

boolean nil_flag = false;
boolean new_funcall_flag = false;
boolean nested_refs = false;
TK saved_rhs_reg = NULL;
TK inline_funcall = NULL;
TK first_op_genarith = NULL;
TK nested_ref_stop_at = NULL;
TK last_ptr = NULL;
double saved_float_reg = -DBL_MAX;
int nextlabel,stkframesize,num_funcalls_in_curr_tree,num_inlines_processed,saved_inline_reg;
int last_ptr_reg_num,last_ptr_deref_offs,last_id_reg_num;
int saved_rhs_reg_num = -1;
int saved_inline_regs[10];
int saved_label_num = -1;
int saved_float_reg_num = -1;
int arg_reg[4] = {EDI, ESI, EDX, ECX};
int used_regs[32] = { 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char* funtopcode[] = {
  "	pushq	%rbp",
  "	movq	%rsp, %rbp",
  "	subq	$32, %rsp",
  "",};
char* funbotcode[] = {
	"	movq    %rbp, %rsp",
	"	popq    %rbp",
	"	ret",
	"",};

void gencode(TK pcode, int varsize, int maxlabel)
{
	TK name = pcode->operands;
	nextlabel = maxlabel + 1;
	TK fund = name->link;

	stkframesize = Assemble_entry(name->stringval,varsize);
	Assemble_jump(JMP, 0);
	while (fund->idval == OP_FUNDCL)
    {
		Assemble_labelstr(fund->operands->operands->link->stringval);
		genc(fund);
		fund = fund->link;
	}
	TK body = fund;
	Assemble_label(0);
	blocknumber = 1;
	genc(body);
	Assemble_exit(name->stringval);
}

int getreg(int kind)
{
	int i = 0;
	int stop = NUM_INT_REGS;
	if (kind !=  TYPE_INT)
    {
		i = 16;
		stop = NUM_REGS;
	}

	for (; i < 7; i++)
    {
		if (used_regs[i] == 0)
        {
			if (i == EDI || i == ESI || i == EDX || i == ECX)
                continue;
			used_regs[i] = 1;
			return i;
		}
	}
	if (i >= stop)
        printf("Regster Overflow.\n");
	return RBASE;
}

int genarith(TK code)
{
    int num, reg_num, lhs_reg, rhs_reg;
    SYMBOL sym;
	switch ( code->tType )
    {
		case TOKEN_NUM:
			switch (code->dType)
            {
				case TYPE_INT:
                {
					num = code->intval;
					reg_num = getreg(TYPE_INT);
					if (num >= MINIMMEDIATE && num <= MAXIMMEDIATE)
                    {
						if (last_ptr && last_ptr_reg_num > -1)
                        {
							Assemble_immed(MOVQ, num, reg_num);
							last_ptr_reg_num = -1;
						}
						else if (!nil_flag)
							Assemble_immed(MOVL, num, reg_num);
						else
							Assemble_immed(MOVQ, num, reg_num);
					}
				}
				break;
				case TYPE_REAL:
                {
					reg_num = getreg(TYPE_REAL);
					saved_float_reg = code->realval;
					saved_float_reg_num = reg_num;
					makeflit(code->realval, nextlabel);
					Assemble_ldflit(MOVSD, nextlabel++, reg_num);
				}
				break;
			}
			break;
		case TOKEN_ID:
        {
			sym = searchst(code->stringval);
			if (!sym)
				return symbol_is_null_int(code->stringval);
			if (sym->blockLevel == 0)
            {
				int temp_reg = getreg(TYPE_INT);
				Assemble_ld(MOVL, 0, temp_reg, "static link");
				reg_num = getreg(TYPE_INT);
				Assemble_ld(MOVL, sym->offset - stkframesize, reg_num, code->stringval);
				break;
			}

			num = sym->offset;

			if (sym->kind == SYM_FUNCTION)
            {
				reg_num = getreg(sym->dType->bType);
				inline_funcall = code;
			}
			else
            {
				reg_num = getreg(code->dType);
				if (reg_num < NUM_INT_REGS)
                {
					SYMBOL temp = searchst(code->stringval);
					if (!temp)
						return symbol_is_null_int(code->stringval);
					SYMBOL next = temp->dType;
					if (!next)
						return symbol_is_null_int(NULL);

					if (next->kind != SYM_ARRAY)
                    {
						last_id_reg_num = reg_num;
						Assemble_ld(MOVL, sym->offset - stkframesize, reg_num, code->stringval);
					}

				}
				else
                {
					last_id_reg_num = reg_num;
					Assemble_ld(MOVSD, sym->offset - stkframesize, reg_num, code->stringval);
				}
			}
		}
		break;
		case OPERATOR:
        {
			if (code->idval == OP_MOD)
            {
				lhs_reg = genarith(code->operands);
				rhs_reg = genarith(code->operands->link);
				Assemble_rr(MOVL, lhs_reg, EAX);
				Assemble_rr(MOVL, lhs_reg, EDX);
				Assemble_call("mod");
				reg_num = EAX;
				break;
			}
			if (code->idval == OP_MUL)
            {
				lhs_reg = genarith(code->operands);
				rhs_reg = genarith(code->operands->link);
				if (lhs_reg != EAX)
					Assemble_rr(MOVL, lhs_reg, EAX);
				Assemble_1r(IMULL, rhs_reg);
				reg_num = EAX;
				break;
			}

			if (first_op_genarith == NULL)
				first_op_genarith = code;
			else
				nested_refs = true;

			if (code->idval == OP_MINUS)
                lhs_reg = genarith(code->operands->link);
			else
				lhs_reg = genarith(code->operands);

			int count = 0;
			if (code->operands->link)
            {
				if (code->idval == OP_MINUS)
					rhs_reg = genarith(code->operands);
				else if (code->idval == OP_FUNCALL)
                {
					TK arglist = code->operands->link;
					int temp_reg;
					while (arglist)
                    {
						temp_reg = genarith(arglist);
						if (temp_reg != arg_reg[count])
                        {
							Assemble_rr(MOVL, temp_reg, arg_reg[count]);
							mark_reg_used(arg_reg[count++]);
							free_reg(temp_reg);
						}
						arglist = arglist->link;
					}
				}
				else rhs_reg = genarith(code->operands->link);
			}
			else
				rhs_reg = 0;
			if (code->idval == OP_FUNCALL) saved_inline_reg = EAX;

			if (code->idval == OP_MINUS)
				lhs_reg = genop(code, lhs_reg, rhs_reg);
			else
				lhs_reg = genop(code, rhs_reg, lhs_reg);

			if (code->idval == OP_FUNCALL)
            {
				int i;
				for (i = 0; i < count; i++)
					free_reg(arg_reg[i]);
			}
			else
                free_reg(rhs_reg);
			reg_num = lhs_reg;
		}
		break;
		default:
			return symbol_is_null_int(NULL);
		break;
	}
	first_op_genarith = NULL;
	return reg_num;
}

int genop(TK code, int rhs_reg, int lhs_reg)
{
    int out = 0;
    int id_val = code->idval;

    switch(id_val)
	{
    case OP_PLUS:
    {
        if (at_least_one_float(lhs_reg, rhs_reg)) Assemble_rr(ADDSD, rhs_reg, lhs_reg);
        else Assemble_rr(ADDL, rhs_reg, lhs_reg);
    }
    break;

    case OP_MINUS:
    {
        if (lhs_reg > 15 && lhs_reg < NUM_REGS)
        {
            Assemble_fneg(lhs_reg, getreg(TYPE_REAL));
            rhs_reg = lhs_reg;
        }
        else if (at_least_one_float(lhs_reg, rhs_reg)) Assemble_rr(SUBSD, rhs_reg, lhs_reg);
            else Assemble_rr(SUBL, rhs_reg, lhs_reg);
        out = lhs_reg;
    }
    break;

    case OP_MUL:
    {
        if (at_least_one_float(lhs_reg, rhs_reg)) Assemble_rr(MULSD, rhs_reg, lhs_reg);
        else Assemble_rr(IMULL, rhs_reg, lhs_reg);
        out = lhs_reg;
    }
    break;

    case OP_DIV_R:
    {
        if (at_least_one_float(lhs_reg, rhs_reg)) Assemble_rr(DIVSD, rhs_reg, lhs_reg);
        else Assemble_rr(DIVL, rhs_reg, lhs_reg);
        out = lhs_reg;
    }
    break;

    case OP_MOD:
    {
        int temp_reg = getreg(TYPE_INT);
    	Assemble_rr(MOVL, lhs_reg, temp_reg);
    	Assemble_rr(DIVL, rhs_reg, lhs_reg);
    	Assemble_rr(IMULL, rhs_reg, lhs_reg);
    	Assemble_rr(SUBL, lhs_reg, temp_reg);
        out = temp_reg;
    }
    break;

    case OP_EQ:
    {
        out = ++nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JE, out);
    }
    break;

    case OP_NE:
    {
        out = ++nextlabel;
        Assemble_rr(CMPQ, rhs_reg, lhs_reg);
        Assemble_jump(JNE, out);
    }
    break;

    case OP_LT:
    {
        out = ++nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JL, out);
    }
    break;

    case OP_LE:
    {
        out = ++nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JLE, out);
    }
    break;

    case OP_GE:
    {
        out = ++nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JGE, out);
    }
    break;

    case OP_GT:
    {
        out = ++nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JG, out);
    }
    break;

    case OP_FUNCALL:
    {
        int temp_reg;
        if (inline_funcall)
        {
        	Assemble_call(inline_funcall->stringval);
            temp_reg = getreg(TYPE_INT);
            if (temp_reg != EAX)
                Assemble_rr(MOVL, EAX, temp_reg);
        }
        out = temp_reg;
    }
    break;

    case OP_AREF:
    {
        if (saved_float_reg != -DBL_MAX)
            Assemble_ldr(MOVQ, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
        else
        {
            if (last_id_reg_num > -1)
             {
                int temp = rhs_reg;
                if (last_id_reg_num > -1 && last_id_reg_num < 16)
                {
                    if (last_id_reg_num == rhs_reg)
                    {
                        rhs_reg = getreg(TYPE_INT);
                        free_reg(temp);
                    }

                    if (last_ptr && last_ptr_reg_num > -1)
                    {
                        boolean found = false;
                        SYMBOL temp0, temp1, temp2, temp3, temp4, temp5, typsym;
                        temp0 = searchst(last_ptr->stringval);
                        typsym = NULL;
                        if (!temp0)
                            return symbol_is_null_int(code->stringval);
                        temp1 = searchst(temp0->link->name);
                        if (!temp1)
                            return symbol_is_null_int(code->stringval);
                        if (temp1->dType->kind == SYM_ARRAY)
                        {
                            typsym = temp1->dType;
                            while (typsym && typsym->kind == SYM_ARRAY)
                                typsym = typsym->dType;
                            if (!typsym)
                                return symbol_is_null_int(code->stringval);
                            temp2 = typsym->dType;
                            if (temp2 && temp2->kind == SYM_RECORD)
                            {
                                temp3 = temp2->dType;
                                while (temp3 && !found)
                                {
                                    if (temp3->offset == last_ptr_deref_offs)
                                    {
                                        found = true;
                                        if (temp3->size > basicsizes[TYPE_INT])
                                            Assemble_ldr(MOVQ, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                                        else
                                            Assemble_ldr(MOVL, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                                    }
                                    temp3 = temp3->link;
                                }
                            }

                        }
                        if (!found)
                            Assemble_ldr(MOVL, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                        last_ptr_reg_num = -1;
                    }
                    else
                        Assemble_ldr(MOVL, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                }
                else
                {
                    if (last_id_reg_num == rhs_reg)
                    {
                        rhs_reg = getreg(TYPE_REAL);
                        free_reg(temp);
                    }
                    Assemble_ldr(MOVSD, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                }
            }
            else
            {
                free_reg(rhs_reg);
                rhs_reg = getreg(TYPE_REAL);
                Assemble_ldr(MOVSD, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
            }

        }
        last_ptr_reg_num = rhs_reg;
        out = rhs_reg;
    }
    break;

    case OP_FLOAT:
    {
        int freg = getreg(TYPE_REAL);
        Assemble_float(rhs_reg, freg);
        free_reg(lhs_reg);
        free_reg(rhs_reg);
        out = freg;
    }
    break;

    case OP_FIX:
    {
        int dreg;
        dreg = getreg(TYPE_INT);
        Assemble_fix(lhs_reg, dreg);
        free_reg(lhs_reg);
        free_reg(rhs_reg);
        out = dreg;
    }
    break;
	}

    if (inline_funcall != NULL && num_funcalls_in_tree > 0)
        saved_inline_reg = rhs_reg;

    return out;
}

void genc(TK code)
{
	TK tok, lhs, rhs;
	int reg_num, offs;
	SYMBOL sym;
	if ( code->tType != OPERATOR )
    {
		if (code->tType == TOKEN_NUM && code->dType == TYPE_INT && new_funcall_flag)
        {
            reset_regs();
            new_funcall_flag = false;
            return;
        }
		printf("Bad code TK");
	}

    reset_regs();
	switch ( code->idval )
    {
		case OP_PROGN:
        {
			last_ptr = NULL;
			last_ptr_reg_num = -1;
			last_ptr_deref_offs = -1;
			nested_ref_stop_at = NULL;

			int i;
			for (i = 0; i < 10; i++) {
				saved_inline_regs[i] = -1;
			}
			num_inlines_processed = 0;
			last_id_reg_num = -1;

			tok = code->operands;
			while (tok)
            {
				num_funcalls_in_curr_tree = num_funcalls_in_tree(tok->operands, 0);
				saved_inline_reg = 0;
				if (tok->idval == OP_LABEL)
					saved_label_num = tok->operands->intval;
				if (search_tree_str(tok, "new"))
					new_funcall_flag = true;
				genc(tok);
				tok = tok->link;
			}
		}
		break;
		case OP_ASSIGN:
        {
			TK last_operand = get_last_operand(code);
			TK outer_link = code->operands->link;

			lhs = code->operands;
			rhs = lhs->link;

			if (code->operands->operands != NULL)
				nested_ref_stop_at = code->operands->operands;
			reg_num = genarith(rhs);
			saved_rhs_reg = rhs;
			saved_rhs_reg_num = reg_num;
			sym = searchst(lhs->stringval);
			int dType = code->dType;
			if (sym)
            {
				offs = sym->offset - stkframesize;

				switch (code->dType)
                {
					case TYPE_INT:
					   Assemble_st(MOVL, reg_num, offs, lhs->stringval);
					   break;

					case TYPE_REAL:
					   Assemble_st(MOVSD, reg_num, offs, lhs->stringval);
					   break;

					default:break;
				}
			}
			else
            {
				sym = searchst(lhs->operands->stringval);
				if (sym)
                {
					offs = sym->offset - stkframesize;

					TK last_link = get_last_link(lhs->operands);

					if (last_link)
                    {
						if (last_link->tType == TOKEN_NUM && last_link->dType == TYPE_INT)
                        {

							Assemble_immed(MOVL, last_link->intval, EAX);
							Assemble_op(CLTQ);

							if (reg_num >= 0 && reg_num < 16)
								Assemble_strr(MOVL, reg_num, offs, EAX, sym->name);

							else
								Assemble_strr(MOVSD, reg_num, offs, EAX, sym->name);
						}
						else if (last_link->tType == OPERATOR && last_link->idval == OP_PLUS)
                        {
							TK plus_operand = last_link->operands;
							TK mul_operand = plus_operand->link->operands;
							TK last_operand = mul_operand->link;

							mark_reg_unused(EAX);
							mark_reg_used(last_ptr_reg_num);

							int move_plus_to = getreg(TYPE_INT);
							int move_mul_to = getreg(TYPE_INT);
							int move_last_to;

							Assemble_immed(MOVL, plus_operand->intval, move_plus_to);
							Assemble_immed(MOVL, mul_operand->intval, move_mul_to);

							if (last_operand->tType == TOKEN_NUM)
                            {
								move_last_to = getreg(TYPE_INT);
								Assemble_immed(MOVL, last_operand->intval, move_last_to);
							}
							else
                            {
								sym = searchst(last_operand->stringval);
								offs = sym->offset - stkframesize;
								Assemble_ld(MOVL, offs, EBX, sym->name);
								Assemble_rr(IMULL, EBX, move_mul_to);
								Assemble_rr(ADDL, move_mul_to, move_plus_to);
								Assemble_op(CLTQ);

								sym = searchst(lhs->operands->stringval);
								offs = sym->offset - stkframesize;
								Assemble_strr(MOVL, last_ptr_reg_num, offs, 0, lhs->operands->stringval);
							}
						}
					}
					else {
						if (reg_num >= 0 && reg_num < 16) {
							Assemble_strr(MOVL, reg_num, offs, getreg(TYPE_INT), sym->name);
						}
						else {
							Assemble_strr(MOVSD, reg_num, offs, getreg(TYPE_INT), sym->name);
						}
					}
				}
			}
			if (lhs->operands)
				reg_num = genarith(lhs->operands);

			nil_flag = false;
			saved_float_reg = -DBL_MAX;
			saved_rhs_reg = NULL;
			saved_rhs_reg_num = -1;
			nested_refs = false;

			last_ptr = NULL;
			last_ptr_reg_num = -1;

			nested_ref_stop_at = NULL;
		}
		break;

		case OP_GOTO:
        {
			lhs = code->operands;
			Assemble_jump(JMP, lhs->intval);
		}
		break;
		case OP_LABEL:
        {
			lhs = code->operands;
			Assemble_label(lhs->intval);
		}
		break;
		case OP_IF:
        {

			lhs = code->operands;
			rhs = code->operands->link;
			int if_label_num = genarith(lhs);

			int else_label_num, endif_label_num;
			if (rhs->link) else_label_num = nextlabel++;
			endif_label_num = nextlabel++;
			if (rhs->link) {
				Assemble_jump(JMP, else_label_num);

				Assemble_label(if_label_num);
				genc(rhs);
				Assemble_jump(JMP, endif_label_num);

				Assemble_label(else_label_num);
				genc(rhs->link);
				Assemble_label(endif_label_num);
			}
			else {
				Assemble_jump(JMP, endif_label_num);

				Assemble_label(if_label_num);
				genc(rhs);
				Assemble_label(endif_label_num);
			}
		}
		break;
		case OP_FUNCALL:
        {
			lhs = code->operands;
			rhs = code->operands->link;
			SYMBOL argsym;

			if (strstr(lhs->stringval, "write"))
            {
				sym = searchst(lhs->stringval);

				if (rhs->tType == TOKEN_STR)
                {
					Assemble_litarg(nextlabel, EDI);
					Assemble_call(lhs->stringval);
					makeblit(rhs->stringval, nextlabel++);
				}
				else if (rhs->tType == TOKEN_CHAR)
                {
					char s[2];
					s[0] = rhs->charval;
					s[1] = '\0';
					Assemble_litarg(nextlabel, EDI);
					Assemble_call(lhs->stringval);
					makeblit(rhs->stringval, nextlabel++);
				}
				else if (rhs->tType == OPERATOR)
                {
					if (rhs->idval == OP_AREF)
                    {
						sym = searchst(rhs->operands->stringval);
						if (!sym) {
							sym = searchst(rhs->operands->operands->stringval);
							if (sym)
                            {
								reg_num = getreg(TYPE_INT);
								offs = sym->offset - stkframesize;
								Assemble_ld(MOVQ, offs, reg_num, sym->name);
								offs = rhs->operands->link->intval;
								int temp = getreg(TYPE_REAL);
								Assemble_ldr(MOVSD, offs, reg_num, temp, "^.");
								Assemble_call(lhs->stringval);
							}
						}
					}
				}

				else if (sym != NULL && (sym->dType->bType == TYPE_INT ||sym->dType->bType == TYPE_REAL))
                {

					SYMBOL argsym;
					if (rhs->tType == TOKEN_NUM)
						printf("\nTOKEN_NUM UNFINISHED\n");
					else if (rhs->tType == TOKEN_ID)
                    {
						argsym = searchst(rhs->stringval);
						if (!argsym)
                        {
							printf("Error: no symbol table entry for var \"%s\"", rhs->stringval);
							return;
						}

						if (argsym->bType == TYPE_INT)
                        {
							reg_num = getreg(TYPE_INT);
							offs = argsym->offset - stkframesize;

							Assemble_ld(MOVL, offs, reg_num, argsym->name);
							Assemble_rr(MOVL, reg_num, EDI);
							Assemble_call(lhs->stringval);
						}

						else if (argsym->bType == TYPE_REAL)
                        {
							reg_num = getreg(TYPE_REAL);
							offs = argsym->offset - stkframesize;
							Assemble_ld(MOVSD, offs, reg_num, argsym->name);
							Assemble_rr(MOVSD, reg_num, EDI);
							Assemble_call(lhs->stringval);
						}
					}
				}
			}
			else
            {
				TK arglist = rhs;
				int temp_reg, index = 0;
				while (arglist)
                {
					temp_reg = genarith(arglist);
					Assemble_rr(MOVL, temp_reg, arg_reg[index]);
					mark_reg_used(arg_reg[index++]);
					free_reg(temp_reg);
					arglist = arglist->link;
				}
				Assemble_call(lhs->stringval);
				free_reg(arg_reg[0]);
				free_reg(arg_reg[1]);
				free_reg(arg_reg[2]);
				free_reg(arg_reg[3]);
			}
		}
		break;
		case OP_FUNDCL:
        {
			lhs = code->operands;
			rhs = code->operands->link;
			blocknumber = lhs->operands->intval;
			printcode(funtopcode);
			TK arglist;
			if (strcmp(lhs->stringval, "function") == 0)
				arglist = lhs->operands->link->link->link;
			else
				arglist = lhs->operands->link->link;
			int index = 0;
			while (arglist)
            {
				SYMBOL argsym = searchst(arglist->stringval);
				switch (argsym->bType)
                {
					case TYPE_INT:
                    {
						reg_num = arg_reg[index++];
						mark_reg_used(reg_num);
						offs = argsym->offset - stkframesize;
						Assemble_st(MOVL, reg_num, offs, argsym->name);
					}
					break;
					case TYPE_REAL:
                    {
						reg_num = arg_reg[index++];
						mark_reg_used(reg_num);
						offs = argsym->offset - stkframesize;
						Assemble_st(MOVSD, reg_num, offs, argsym->name);
					}
					break;
				}
				arglist = arglist->link;
			}
			genc(rhs);

			TK fun_name = lhs->operands->link;
			if (strcmp(lhs->stringval, "function") == 0)
            {
				char fun_var[16];
				int i;
				fun_var[0] = '_';
				for (i = 1; i < 16; i++)
					fun_var[i] = fun_name->stringval[i-1];
				SYMBOL sym = searchst(fun_var);
				switch (sym->bType)
                {
					case TYPE_INT:
                    {
						offs = sym->offset - stkframesize;
						Assemble_ld(MOVL, offs, EAX, sym->name);
					}
					break;
					case TYPE_REAL:
                    {
						offs = sym->offset - stkframesize;
						Assemble_ld(MOVSD, offs, EAX, sym->name);
					}
					break;
				}
			}

			printcode(funbotcode);
		}
		break;
	}
}
void reset_regs()
{
    int i;
    for (i = 0; i < NUM_REGS; i++)
        used_regs[i] = 0;
}

void free_reg(int reg_num)
{
    if (reg_num < 0 || reg_num >= NUM_REGS)
    {
        printf("Error: cannot free register number %d\n", reg_num);
        return;
    }
    used_regs[reg_num] = 0;
}

boolean at_least_one_float(int lhs_reg, int rhs_reg)
{
    if ((lhs_reg >= NUM_INT_REGS && lhs_reg < NUM_REGS) || (rhs_reg >= NUM_INT_REGS && rhs_reg < NUM_REGS))
        return true;
    return false;
}

boolean both_float(int lhs_reg, int rhs_reg)
{
    if ((lhs_reg >= NUM_INT_REGS && lhs_reg < NUM_REGS) && (rhs_reg >= NUM_INT_REGS && rhs_reg < NUM_REGS))
        return true;

    return false;
}

void mark_reg_unused(int reg_num)
{
    if (reg_num < 0 || reg_num >= NUM_REGS)
    {
        printf("1 Error: register %d out of bounds\n", reg_num);
        return;
    }
    used_regs[reg_num] = 0;
}

void mark_reg_used(int reg_num)
{
    if (reg_num < 0 || reg_num >= NUM_REGS)
    {
        printf("2 Error: register %d out of bounds\n", reg_num);
        return;
    }
    used_regs[reg_num] = 1;
}

boolean funcallin(TK code) {
    int num = num_funcalls_in_tree(code, 0);
    if (num > 0)
        return true;
    return false;
}

int num_funcalls_in_tree(TK tok, int num)
{
    if (tok == NULL)
        return num;
    if (tok->idval == OP_FUNCALL)
        num++;
    if (tok->link != NULL)
        num = num_funcalls_in_tree(tok->link, num);
    if (tok->operands != NULL)
        num = num_funcalls_in_tree(tok->operands, num);
    return num;
}

boolean search_tree_str(TK tok, char str[])
{
    if (tok == NULL)
        return false;
    boolean found = false;

    if (strcmp(tok->stringval, str) == 0)
        return true;
    if (tok->link != NULL)
        found = search_tree_str(tok->link, str);
    if (tok->operands != NULL)
        found = search_tree_str(tok->operands, str);
    return found;
}

void print_used_regs()
{
    printf("\nUsed registers: %d", used_regs[0]);
    int i=1;
    for (; i < NUM_REGS; i++)
        printf(" %d", used_regs[i]);
    printf("\n\n");
}

int symbol_is_null_int(char *str)
{
    if (str)
        printf("Error: NULL symbol (\"%s\")\n", str);
    return 0;
}

boolean is_equal(TK a, TK b)
{
    if (!a || !b)
        return false;
    if ((long) a == (long) b)
        return true;
    return false;
}

boolean is_gen_purpose_reg(int reg_num)
{
    if (reg_num < 0 || reg_num >= NUM_INT_REGS)
        return false;
    return true;
}

boolean is_fp_reg(int reg_num)
{
    if (reg_num < NUM_INT_REGS || reg_num >= NUM_REGS)
        return false;
    return true;
}
