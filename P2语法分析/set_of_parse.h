#ifndef PARSE_H
#define PARSE_H

SYMBOL searchinsst(char name[]);

TK makeProgram(TK program_head, TK routine);
TK makeOp(int opNum);
TK makePnb(TK tok, TK statements);
TK makeProgn(TK tok, TK statements);
TK cons(TK list, TK item);
TK findType(TK tok);
TK binop(TK op, TK lhs, TK rhs);
TK binop_type_coerce(TK op, TK lhs, TK rhs);
TK makefix(TK tok);
TK makefloat(TK tok);
TK findId(TK tok);
TK instEnum(TK idlist);
TK makeSubrange(TK tok, int low, int high);
TK makeIntc(int num);
TK copyTok(TK origtok);
TK instDotdot(TK lowtok, TK dottok, TK hightok);
TK instArray(TK bounds, TK typetok);
TK instRec(TK rectok, TK argstok);
TK instFields(TK idlist, TK typetok);
TK doLabel(TK labeltok, TK tok, TK statement);
TK arrayRef(TK arr, TK tok, TK subs, TK tokb);
TK makeAref(TK var, TK off, TK tok);
TK makePlus(TK lhs, TK rhs, TK tok);
TK addInt(TK exp, TK off, TK tok);
TK makeFuncall(TK tok, TK fn, TK args);
TK write_fxn_args_type_check(TK fn, TK args);
TK unaryop(TK op, TK lhs);
TK reduceDot(TK var, TK dot, TK field);
TK get_last_link(TK tok);
TK get_last_operand(TK tok);
TK makeRealTok(float num);
TK makeIf(TK tok, TK exp, TK thenpart, TK elsepart);
TK makeRepeat(TK tok, TK statements, TK tokb, TK expr);
TK makeLabel();
TK makeGoto(int label);
TK makeWhile(TK tok, TK expr, TK tokb, TK statement);
TK makeFor(TK tok, TK asg, TK dir, TK endexpr,TK tokc, TK statement);
TK makeLoopIncr(TK var, int incr_amt);
TK doGoto(TK tok, TK labeltok);
TK nconc(TK lista, TK listb);
TK makeFunDcl(TK head, TK body);
TK instFun(TK head);
TK endDecl(TK decl);
TK addoffs(TK exp, TK off);
TK appendst(TK statements, TK more);
TK dopoint(TK var, TK tok);
TK fillintc(TK tok, int num);
TK get_rec(TK rec, TK offset);
TK get_rec_field(TK rec, TK field);
TK instpoint(TK tok, TK typename);
TK maketimes(TK lhs, TK rhs, TK tok);
TK mulint(TK exp, int n);
TK search_rec(SYMBOL recsym, TK field);
TK std_fxn_args_type_check(TK fn, TK args);
TK initilize();

void  endVarPart();
void  instlabel (TK num);
void  instType(TK typename, TK typetok);
void  instVars(TK idlist, TK typetok);
void  settoktype(TK tok, SYMBOL typ, SYMBOL ent);
void  instConst(TK idtok, TK consttok);
int   wordaddress(int n, int wordsize);



typedef short boolean;
#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1

#ifdef true
#undef true
#endif
#define true 1

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef false
#undef false
#endif
#define false 0

#define NUM_COERCE_IMPLICIT     1
#define ELIM_NESTED_PROGN       1
#define DB_PRINT_ARGS           1

#define DB_CONS       1
#define DB_BINOP      2
#define DB_MAKEIF     4
#define DB_MAKEPROGN  8
#define DB_PARSERES  16

#define DB_MAKEPROGRAM  32
#define DB_MAKEFUNCALL  64
#define DB_ADDINT       96
#define DB_FINDID       128
#define DB_FINDTYPE     256
#define DB_ADDOFFS      384
#define DB_INSTVARS     512
#define DB_INSTENUM     768
#define DB_MAKEFOR      1024
#define DB_INSTTYPE     1280
#define DB_MULINT       2048
#define DB_MAKEREPEAT   3072
#define DB_UNARYOP      4096
#define DB_MAKEOP       5120
#define DB_MAKEFLOAT    6144
#define DB_MAKEFIX      7168
#define DB_MAKEGOTO     8192
#define DB_MAKELABEL    9216
#define DB_MAKEPNB      10240
#define DB_INSTCONST    11264
#define DB_MAKEWHILE    12288
#define DB_COPYTOK      13312
#define DB_INSTDOTDOT   14336
#define DB_SEARCHINSST  15360
#define DB_INSTPOINT    16384
#define DB_INSTREC      17408
#define DB_INSTFIELDS   18432
#define DB_MAKEPLUS     19456
#define DB_MAKEAREF     20480
#define DB_REDUCEDOT    21504
#define DB_ARRAYREF     22528
#define DB_DOPOINT      23552
#define DB_INSTARRAY    24576
#define DB_NCONC        25600
#define DB_MAKEINTC     26624
#define DB_APPENDST     27648
#define DB_DOGOTO       28672
#define DB_DOLABEL      29696
#define DB_INSTLABEL    30720
#define DB_SETTOKTYPE   31744
#define DB_MAKESUBRANGE 32768
#define DB_MAKEFUNDCL   1
#define DB_INSTFUN      2

#define GEN_OUTPUT         1
#define DBL_MAX 1.7976931348623157e+308
#define DBL_MIN 2.2250738585072014e-308
#define FLT_MAX 3.40282347e+38F
#define FLT_MIN 1.17549435e-38F
#define INT_MAX 2147483647
#define INT_MIN -2147483648


#endif
