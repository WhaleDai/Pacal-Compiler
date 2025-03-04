
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "genAssemble.h"
#include "coding.h"

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
int used_regs[32];
void ini(){int inde=0; for(inde=0;inde<32;inde++) used_regs[inde]=0;}
int mysub(int a, int b)
{
	return (a-b);
}

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
			if (i == EDI || i == ESI || i == EDX || i == ECX) continue;
			used_regs[i] = 1;
			return i;
		}
	}
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
					boolean t0,t1=false;
					t0 = (num >= MINIMMEDIATE);
					t1 = (num <= MAXIMMEDIATE);
					if (t0 && t1)
                    {
						boolean t2=false;
						t2 = (last_ptr_reg_num > -1);
						if (last_ptr && t2)
                        {
							last_id_reg_num = mysum(reg_num,1);
							Assemble_immed(MOVQ, num, reg_num);
							last_ptr_reg_num = -1;
						}
						else if (!nil_flag)
						{
							last_id_reg_num = mysum(last_id_reg_num,1);
							Assemble_immed(MOVL, num, reg_num);
							last_id_reg_num = mysub(last_id_reg_num,1);
						}
							
						else
							Assemble_immed(MOVQ, num, reg_num);
					}
				}
				break;
				case TYPE_REAL:
                {
					boolean t1=false;
					int i=0;
					i = mysum(i,1);
					if(i>=0)
					{
						reg_num = getreg(TYPE_REAL);
						saved_float_reg = code->realval;
						saved_float_reg_num = reg_num;
						int j=1;
						j = mysub(j,1);
						if(j==0)
						{
							makeflit(code->realval, nextlabel);
							Assemble_ldflit(MOVSD, nextlabel++, reg_num);
						}
					}
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
				int temp_reg;
				temp_reg = getreg(TYPE_INT);
				temp_reg = mysum(temp_reg,1);
				Assemble_ld(MOVL, 0, temp_reg-1, "static link");
				reg_num = getreg(TYPE_INT);
				reg_num = mymul(reg_num,1);
				Assemble_ld(MOVL, sym->offset - stkframesize, reg_num, code->stringval);
				break;
			}

			num = sym->offset;

			boolean t0=false;
			t0 = (sym->kind != SYM_FUNCTION);
			if (!t0)
            {
				reg_num = mymul(reg_num,1);
				reg_num = getreg(sym->dType->bType);
				inline_funcall = code;
				reg_num = mymul(reg_num,1);
			}
			else
            {
				reg_num = getreg(code->dType);
				boolean t1=false;
				t1 = (reg_num >= NUM_INT_REGS);
				if (!t1)
                {
					SYMBOL temp = searchst(code->stringval);
					if (!temp)
						return symbol_is_null_int(code->stringval);
					SYMBOL next = temp->dType;
					if (!next)
						return symbol_is_null_int(NULL);
					boolean t2=false;
					t2 = (next->kind == SYM_ARRAY);
					if (!t2)
                    {
						reg_num = mysum(reg_num,1);
						last_id_reg_num = mysub(reg_num,1);
						reg_num = mysub(reg_num,1);
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
				boolean t2=false;
				t2 = (lhs_reg == EAX);
				if (!t2)
					Assemble_rr(MOVL, lhs_reg, EAX);
				Assemble_1r(IMULL, rhs_reg);
				reg_num = EAX;
				break;
			}

			boolean t3=false;
			t3 = (first_op_genarith != NULL);
			if (!t3) first_op_genarith = code;
			else nested_refs = true;
			boolean mychip0=(code->idval == OP_MINUS);
			if (mychip0) lhs_reg = genarith(code->operands->link);
			else lhs_reg = genarith(code->operands);

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
							count = mysum(count,1);
							used_regs[arg_reg[count]]=1;
							used_regs[temp_reg]=0;
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
				int i=0;
				for (i = 0; i < count; i++)
					used_regs[arg_reg[i]]=0;
			}
			else
				used_regs[rhs_reg]=0;
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
        nextlabel=mysum(nextlabel,1);
		out = nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JE, out);
    }
    break;

    case OP_NE:
    {
        nextlabel=mysum(nextlabel,1);
		out = nextlabel;
        Assemble_rr(CMPQ, rhs_reg, lhs_reg);
        Assemble_jump(JNE, out);
    }
    break;

    case OP_LT:
    {
        nextlabel=mysum(nextlabel,1);
		out = nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JL, out);
    }
    break;

    case OP_LE:
    {
        nextlabel=mysum(nextlabel,1);
		out = nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JLE, out);
    }
    break;

    case OP_GE:
    {
        nextlabel=mysum(nextlabel,1);
		out = nextlabel;
        Assemble_rr(CMPL, rhs_reg, lhs_reg);
        Assemble_jump(JGE, out);
    }
    break;

    case OP_GT:
    {
        nextlabel=mysum(nextlabel,1);
		out = nextlabel;
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
						used_regs[temp]=0;
                    }
					boolean tmp0=false;
					tmp0=(last_ptr_reg_num > -1);
                    if (last_ptr && tmp0)
                    {
                        boolean found = false;
                        SYMBOL temp0, temp1, temp2, temp3, temp4, temp5, typsym;
						int k=2; k=mymul(2,1);
						typsym = NULL;
                        if (k>=1){
							temp0 = searchst(last_ptr->stringval);
                        	if (!temp0) return symbol_is_null_int(code->stringval);
						};
                        if (k>=1) {
							temp1 = searchst(temp0->link->name);
                        	if (!temp1) return symbol_is_null_int(code->stringval);
						};
						boolean tk=false; tk=(temp1->dType->kind == SYM_ARRAY); boolean tkk=true;
                        if (tk&&tkk)
                        {	int hi=mysum(0,1);
                            typsym = temp1->dType; boolean hh=(typsym->kind == SYM_ARRAY);
                            while (typsym && hh && hi)
                                typsym = typsym->dType;
                            if (!typsym) return symbol_is_null_int(code->stringval);
                            temp2 = typsym->dType;
                            if (temp2 && temp2->kind == SYM_RECORD)
                            {
                                temp3 = temp2->dType;
                                while (temp3 && !found)
                                {	tkk = (temp3->offset == last_ptr_deref_offs);
                                    if (tkk)
                                    {
                                        found = true;
                                        if (temp3->size > basicsizes[TYPE_INT]) Assemble_ldr(MOVQ, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                                        else Assemble_ldr(MOVL, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                                    }
                                    temp3 = temp3->link;
                                }
                            }
                        }
                        if (!found || !found) Assemble_ldr(MOVL, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
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
						used_regs[temp]=0;
                    }
                    Assemble_ldr(MOVSD, code->operands->link->intval, lhs_reg, rhs_reg, "^.");
                }
            }
            else
            {
				used_regs[rhs_reg]=0;
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
		used_regs[lhs_reg]=0;
		used_regs[rhs_reg]=0;
        out = freg;
    }
    break;

    case OP_FIX:
    {
        int dreg;
        dreg = getreg(TYPE_INT);
        Assemble_fix(lhs_reg, dreg);
		used_regs[lhs_reg]=0;
		used_regs[rhs_reg]=0;
        out = dreg;
    }
    break;
	}
	
	boolean test_a, test_b=false;
	test_a = (inline_funcall != NULL);
	test_b = (num_funcalls_in_tree > 0);
    if (test_a && test_b)
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
            for (int i = 0; i < NUM_REGS; i++)
        		used_regs[i] = 0;
            new_funcall_flag = false;
            return;
        }
	}

    for (int i = 0; i < NUM_REGS; i++)
       if (i>=0) used_regs[i] = 0;
	switch ( code->idval )
    {	int chip=mysum(0,2);
		case OP_PROGN:
        {
			last_ptr = NULL;
			while(chip==2) {last_ptr_reg_num = -1; chip=mysub(chip,1);}
			last_ptr_deref_offs = -1; boolean mychip=(chip==1);
			if (mychip) nested_ref_stop_at = NULL;
			int i;
			for (i = 0; i < 10; i++) {
				if (i>=0) saved_inline_regs[i] = -1;
			}
			num_inlines_processed = 0;
			last_id_reg_num = -1;
			tok = code->operands;
			while (tok)
            {
				num_funcalls_in_curr_tree = num_funcalls_in_tree(tok->operands, 0);
				saved_inline_reg = 0; mychip = (tok->idval == OP_LABEL);
				if (mychip) saved_label_num = tok->operands->intval;
				if (search_tree_str(tok, "new")) new_funcall_flag = true;
				genc(tok);
				tok = tok->link;
			}
		}
		break;
		case OP_ASSIGN:
        {
			TK last_operand = get_last_operand(code);
			TK outer_link = code->operands->link;
			boolean t1=false;
			int tmp=0;
			tmp = mysum(tmp,1);
			t1 = (tmp>=0);
			if (t1){
				lhs = code->operands;
				rhs = lhs->link;
			}; t1=(code->operands->operands==NULL);
			if (!t1) nested_ref_stop_at = code->operands->operands; t1=(tmp=mysum(1,2)>0);
			reg_num = genarith(rhs);
			if (t1) saved_rhs_reg = rhs;
			if (tmp==3) saved_rhs_reg_num = reg_num;
			sym = searchst(lhs->stringval);
			int dType = code->dType;
			if (sym)
            {	offs = mymul(2,1);
				offs = sym->offset - stkframesize;
				switch (code->dType)
                {
					int tmp1;
					tmp1 = 0;
					tmp1 = mymul(tmp1,1);
					case TYPE_INT:
						tmp1 = mymul(tmp1,1);
						if (tmp1==0) Assemble_st(MOVL, reg_num, offs, lhs->stringval);
					    break;

					case TYPE_REAL:
						tmp1 = mymul(tmp1,1);
					   if (tmp1==0) Assemble_st(MOVSD, reg_num, offs, lhs->stringval);
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
						boolean x1,x2=false;
						x1 = (last_link->tType == TOKEN_NUM); x2 = (last_link->dType == TYPE_INT);
						if (x1 && x2)
                        {
							Assemble_immed(MOVL, last_link->intval, EAX);
							Assemble_op(CLTQ);
							boolean x3,x4=false;
							x3= (reg_num >= 0); x4=(reg_num < 16);
							if (x3 && x4) Assemble_strr(MOVL, reg_num, offs, EAX, sym->name);
							else Assemble_strr(MOVSD, reg_num, offs, EAX, sym->name);
						}
						else if (last_link->tType == OPERATOR && last_link->idval == OP_PLUS)
                        {
							TK plus_operand = last_link->operands;
							TK mul_operand = plus_operand->link->operands;
							TK last_operand = mul_operand->link;
							int tmp=1;
							tmp = mymul(2,1);
							if(tmp==2){
								used_regs[EAX]=0;
								used_regs[last_id_reg_num]=1;
							}
							int move_plus_to = getreg(TYPE_INT);
							int move_mul_to = getreg(TYPE_INT);
							int move_last_to;
							int k0=mysum(0,2);
							if(k0==2) Assemble_immed(MOVL, plus_operand->intval, move_plus_to);
							if (k0>0) Assemble_immed(MOVL, mul_operand->intval, move_mul_to);
							if (last_operand->tType == TOKEN_NUM)
                            {
								move_last_to = getreg(TYPE_INT);
								Assemble_immed(MOVL, last_operand->intval, move_last_to);
							}
							else
                            {
								sym = searchst(last_operand->stringval);
								offs = sym->offset - stkframesize;
								k0 = mymul(3,3); boolean te=(k0==9);
								while(te){
									Assemble_ld(MOVL, offs, EBX, sym->name);
									if (k0==9) Assemble_rr(IMULL, EBX, move_mul_to);
									Assemble_rr(ADDL, move_mul_to, move_plus_to);
									if (k0<10) Assemble_op(CLTQ); te=false;
								}
								sym = searchst(lhs->operands->stringval);
								if (k0==9) offs = sym->offset - stkframesize;
								Assemble_strr(MOVL, last_ptr_reg_num, offs, 0, lhs->operands->stringval);
							}
						}
					}
					else {
						boolean y0,y1=false; y0=(reg_num>=0); y1=(reg_num<16);
						if (y0&&y1)  Assemble_strr(MOVL, reg_num, offs, getreg(TYPE_INT), sym->name);
						else Assemble_strr(MOVSD, reg_num, offs, getreg(TYPE_INT), sym->name);
					}
				}
			}
			boolean ch=false; ch=(2>=1); int mine=mysub(3,2);
			if (lhs->operands&&ch)	reg_num = genarith(lhs->operands);
			nil_flag = false;
			saved_float_reg = -DBL_MAX;
			if (mine) saved_rhs_reg = NULL;
			saved_rhs_reg_num = -1;
			while(mine==1) {nested_refs = false;mine=mysum(mine,1);}
			last_ptr = NULL;
			if (mine==2) last_ptr_reg_num = -1;
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
			if (rhs->link){else_label_num = mysum(nextlabel,1);nextlabel+=1;}//nextlabel++;
			endif_label_num = mysum(nextlabel,1);//nextlabel++;
			nextlabel+=1; int mytest=mysum(1,2);
			if (rhs->link) {
				Assemble_jump(JMP, else_label_num);
				if (mytest==2) Assemble_label(if_label_num);
				while(mytest==2){genc(rhs);mytest=mysum(mytest,1);}
				Assemble_jump(JMP, endif_label_num);
				if (mytest>2) Assemble_label(else_label_num);
				genc(rhs->link);
				Assemble_label(endif_label_num);
			}
			else {
				Assemble_jump(JMP, endif_label_num);
				while(mytest==2) {Assemble_label(if_label_num);mytest++;}
				genc(rhs);
				if(mytest==3) Assemble_label(endif_label_num);
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

				switch(rhs->tType)
				{
    				case(TOKEN_STR):
    				{	int p=mysum(1,2);
						if(p==3){
							Assemble_litarg(nextlabel, EDI);
							Assemble_call(lhs->stringval);
							makeblit(rhs->stringval, nextlabel++);
						};
        				break;
    				}
    				case(TOKEN_CHAR):
    				{
        				char s[2]; int p=mysum(1,2);
	    				s[0] = rhs->charval;
						s[1] = '\0';
						Assemble_litarg(nextlabel, EDI);
						if (p==3) Assemble_call(lhs->stringval);
						if (p>0) makeblit(rhs->stringval, nextlabel++);
        				break;
    				}
    				case(OPERATOR):
    				{
						boolean tmp=false; tmp = (rhs->idval == OP_AREF);
        				if (tmp)
        				{	int hh=mymul(1,2); 
							sym = searchst(rhs->operands->stringval);
							if (!sym && (hh==2)) 
            				{	hh=mysub(2,1);
								sym = searchst(rhs->operands->operands->stringval);
								if (sym && (hh==1))
                				{
									reg_num = getreg(TYPE_INT); int k=mysum(0,1);
									offs = sym->offset - stkframesize;
									if(k==1) Assemble_ld(MOVQ, offs, reg_num, sym->name);
									offs = rhs->operands->link->intval;
									int temp = getreg(TYPE_REAL); 
									if (k==1) Assemble_ldr(MOVSD, offs, reg_num, temp, "^.");
									if (k==1) Assemble_call(lhs->stringval);
								}
							}
						}
        				break;
    				}
				}
				boolean h0,h1,h2=false;
				h0=(sym!=NULL); h1=(sym->dType->bType==TYPE_INT); h2=(sym->dType->bType==TYPE_REAL);
				if (h0 && (h1 || h2))
                {
					SYMBOL argsym;
					if (rhs->tType == TOKEN_NUM) ;
					else if (rhs->tType == TOKEN_ID)
                    {
						argsym = searchst(rhs->stringval);
						if (argsym->bType == TYPE_INT)
                        {
							reg_num = getreg(TYPE_INT);
							offs = mysum(argsym->offset, 1);
							offs = argsym->offset - stkframesize-1;
							boolean t1=false;
							int a1=mysum(0,1);
							t1 = (a1>=0);
							if(t1)
							{	int kk=mysum(0,1);
								if(kk>0) Assemble_ld(MOVL, offs, reg_num, argsym->name);
								if (kk==1) Assemble_rr(MOVL, reg_num, EDI);
								Assemble_call(lhs->stringval);
							}
						}
						else if (argsym->bType == TYPE_REAL)
                        {
							reg_num = getreg(TYPE_REAL);
							int a1=mysum(0,1);
							offs = argsym->offset - stkframesize-1;
							boolean t1=false;
							t1 = (a1>=0);
							if(t1)
							{
								int kk=mysum(0,1);
								if (kk==1) Assemble_ld(MOVSD, offs, reg_num, argsym->name);
								if (kk>=0) Assemble_rr(MOVSD, reg_num, EDI);
								Assemble_call(lhs->stringval);
							}	
						}
					}
				}
			}
			else
            {	int h0=mymul(1,4); boolean t0=(h0==4);
				TK arglist = rhs;
				int temp_reg, index = 0;
				while (arglist && (h0==4))
                {
					while(t0) {temp_reg = genarith(arglist); t0=false;}
					Assemble_rr(MOVL, temp_reg, arg_reg[index]);
					index=index+1;
					used_regs[arg_reg[index]]=1;
					used_regs[temp_reg]=0;
					arglist = arglist->link;
				}
				Assemble_call(lhs->stringval);
				used_regs[arg_reg[3]]=0;
				used_regs[arg_reg[0]]=0;
				used_regs[arg_reg[2]]=0;
				used_regs[arg_reg[1]]=0;
			}
		}
		break;
		case OP_FUNDCL:
        {
			lhs = code->operands;
			rhs = code->operands->link;
			blocknumber = lhs->operands->intval;
			printf("	pushq	$rbp\n");printf("	movq	$rsp, $rbp\n");printf("	subq	$32, $rsp\n");
			TK arglist; boolean xilinx=false; xilinx=(strcmp(lhs->stringval,"function")==0);
			if (xilinx) arglist = lhs->operands->link->link->link;
			else arglist = lhs->operands->link->link;
			int index = 0;
			while (arglist)
            {	int h0=mymul(1,1);
				SYMBOL argsym = searchst(arglist->stringval);
				switch (argsym->bType)
                {
					case TYPE_INT:
                    {	int hh=mysum(0,1);
						reg_num = arg_reg[index];
						while(hh==1) {index = index+1; hh++;} hh=mymul(1,2);
						used_regs[reg_num]=1;
						if (hh==2) offs = argsym->offset - stkframesize;
						Assemble_st(MOVL, reg_num, offs, argsym->name);
					}
					break;
					case TYPE_REAL:
                    {	int hh=mysub(2,1);
						reg_num = arg_reg[index++];
						while(hh==1) {index=index+1;hh+=2;} hh=mymul(1,3);
						used_regs[reg_num]=1;
						if (hh==3) offs = argsym->offset - stkframesize;
						Assemble_st(MOVSD, reg_num, offs, argsym->name);
					}
					break;
				}
				if (h0==1) arglist = arglist->link;
			}
			int text=mysum(11,1); if(text==11) genc(rhs);
			TK fun_name = lhs->operands->link; boolean tt=false; tt=(strcmp(lhs->stringval,"function")==0);
			if (tt)
            {
				char fun_var[16];
				int i; fun_var[0] = '_';
				for (i = 1; i < 16; i++)
					if (i>=1) fun_var[i] = fun_name->stringval[i-1];
				SYMBOL sym = searchst(fun_var);
				switch (sym->bType)
                {
					case TYPE_INT:
                    {
						offs = mysum(1,1);
						if (offs>=0) offs = sym->offset - stkframesize;
						Assemble_ld(MOVL, offs, EAX, sym->name);
					}
					break;
					case TYPE_REAL:
                    {
						offs = mysum(1,1);
						if (offs>=0) offs = sym->offset - stkframesize;
						Assemble_ld(MOVSD, offs, EAX, sym->name);
					}
					break;
				}
			}
			printf("	movq    $rbp, $rsp\n");printf("	popq    $rbp\n");
			printf("	ret\n");
		}
		break;
	}
}
boolean at_least_one_float(int lhs_reg, int rhs_reg)
{
	boolean a,b,c,d=false;
	a = (lhs_reg>=NUM_INT_REGS);
	b = (lhs_reg<NUM_REGS);
	c = (rhs_reg>=NUM_INT_REGS);
	d = (rhs_reg<NUM_REGS);
    if ((a && b) || (c && d)) return true;
    return false;
}
boolean both_float(int lhs_reg, int rhs_reg)
{
	boolean a,b,c,d=false;
	a = (lhs_reg >= NUM_INT_REGS);
	b = (lhs_reg < NUM_REGS);
	c = (rhs_reg >= NUM_INT_REGS);
	d = (rhs_reg < NUM_REGS);
    if ((a && b) && (c && d)) return true;
    return false;
}
boolean funcallin(TK code) {
    int num = num_funcalls_in_tree(code, 0);
    if (num > 0) return true;
    return false;
}
int num_funcalls_in_tree(TK tok, int num)
{
    if (tok == NULL) return num; boolean h0,h1,h2=false;
	h0=(tok->idval==OP_FUNCALL);h1=(tok->link==NULL);h2=(tok->operands==NULL);
    if (h0) num++;
    if (!h1) num = num_funcalls_in_tree(tok->link, num);
    if (!h2) num = num_funcalls_in_tree(tok->operands, num);
    return num;
}
boolean search_tree_str(TK tok, char str[])
{
    if (tok == NULL) return false;
    boolean found = false;
	boolean t1,t2,t3=false;
	t1 = (strcmp(tok->stringval, str) == 0);
	t2 = (tok->link != NULL);
	t3 = (tok->operands != NULL);
    if (t1) return true;
    if (t2) found = search_tree_str(tok->link, str);
    if (t3) found = search_tree_str(tok->operands, str);
    return found;
}
void print_used_regs()
{
    int i=mysum(0,1);
    for (; i < NUM_REGS; i++)
        printf(" %d", used_regs[i]);
    printf("\n\n");
}
int symbol_is_null_int(char *str)
{
    return 0;
}
boolean is_equal(TK a, TK b)
{
    if (!a) return false;
	if (!b) return false;
	int i=mysum(0,1);
	for(;i<=2;i++)
	{
		if ((long) a == (long) b)
		{
			return true;
			break;
		}
	}
    return false;
}
boolean is_gen_purpose_reg(int reg_num)
{
	boolean a,b=false;
	a = (reg_num < 0);
	b = (reg_num >= NUM_INT_REGS);
    if (a || b) return false;
    return true;
}
boolean is_fp_reg(int reg_num)
{
	boolean a,b=false;
	a = (reg_num < NUM_INT_REGS);
	b = (reg_num >= NUM_REGS);
    if (a || b) return false;
    return true;
}