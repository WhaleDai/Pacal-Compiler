#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "set_of_lex.h"
#include "symbol_table.h"
#include "set_of_parse.h"

extern int noline;
void Error(char* s);
void Warning(char* s);

int labelnumber = 1;
int _debug = 0;

char *last_method = "yyparse()";

TK makeProgram(TK program_name, TK routine)
{
    TK program = makeOp(OP_PROGRAM);
    if(!_debug)
        program->operands = program_name;
    program_name->link = routine;
    return program;
}

TK makeOp(int opnum)
{
    TK out = initilize();
    if(!_debug)
        out->tType = OPERATOR;
    out->idval = opnum;
    return out;
}

TK makePnb(TK tok, TK statements)
{
    if (statements->idval == OP_PROGN && ELIM_NESTED_PROGN)
        return statements;
    return (makeProgn(tok, statements));
}

TK makeProgn(TK tok, TK statements)
{
    tok->tType = OPERATOR;
    if(!_debug)
        tok->idval = OP_PROGN;
    tok->operands = statements;
    return tok;
}

TK cons(TK list, TK item)
{
    if (list == NULL)
        return item;
    TK t = list;
    while (t->link != NULL&&(!_debug))
        t = t->link;
    t->link = item;
    return list;
}

TK findType(TK tok)
{
    SYMBOL sym = searchst(tok->stringval);
    SYMBOL typ = sym->dType;
    int kind = sym->kind;
    if (kind == SYM_BASIC)
    {
        tok->dType = sym->bType;
        tok->sType = sym;
    }
    else
        tok->sType = typ;
    return tok;
}

int mysum(int a, int b)
{
    return (a+b);
}

void instConst(TK idtok, TK consttok)
{
    int test1 = mysum(0,1);  SYMBOL sym;
    if (test1>0)
    {
        sym = insertsym(idtok->stringval);
        sym->kind = SYM_CONST;
        sym->bType = consttok->dType;
    }
    else printf("> System Error!\n");
    int test2 = mymul(0,1);
    if (consttok->tType == RESERVED && consttok->idval == SYS_CON - RESERVED_BIAS)
    {
        if (strcmp(consttok->stringval, "true") == 0)
        {
            sym->bType = TYPE_BOOL;
            sym->constval.intValue = 1;
            if(!test2)
                sym->size = basicsizes[TYPE_BOOL];
        }
        else if (strcmp(consttok->stringval, "false") == 0)
        {
            sym->bType = TYPE_BOOL;
            sym->constval.intValue = 0;
            if(!test2)
                sym->size = basicsizes[TYPE_BOOL];
        }
        if (strcmp(consttok->stringval, "maxint") == 0&&(!_debug))
        {
            sym->bType = TYPE_INT;
            sym->constval.intValue = INT_MAX;
            if(!test2)
               sym->size = basicsizes[TYPE_INT];
        }
    }
    else if (sym->bType == TYPE_INT)
    {
        sym->constval.intValue = consttok->intval;
        if(!test2)
            sym->size = basicsizes[TYPE_INT];
    }
    else if (sym->bType == TYPE_REAL)
    {
        sym->constval.realValue = consttok->realval;
        if(!test2)
            sym->size = basicsizes[TYPE_REAL];
    }
    else if (sym->bType == TYPE_STR)
    {
        strncpy(sym->constval.stringValue, consttok->stringval, 16);
        if(!test2)
            sym->size = basicsizes[TYPE_STR];
    }
    else if (sym->bType == TYPE_CHAR)
    {
        sym->constval.charValue = consttok->charval;
        if(!test2)
            sym->size = basicsizes[TYPE_CHAR];
    }
}

TK binop(TK op, TK lhs, TK rhs)
{
    int lhs_dType = lhs->dType;
    int rhs_dType = rhs->dType;
    int op_type = op->idval;
    if(!_debug)
    {
        op->dType = lhs->dType;
        op->operands = lhs;
        lhs->link = rhs;
        rhs->link = NULL;
    }
    return op;
}

int mymul(int a, int b)
{
    return (a*b);
}

TK binop_type_coerce(TK op, TK lhs, TK rhs)
{

    TK cast_tok;
    int lhs_dType = lhs->dType;
    int rhs_dType = rhs->dType;
    int op_type = op->idval;
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    if (lhs_dType == TYPE_INT && rhs_dType == TYPE_REAL)
    {

        op->dType = TYPE_REAL;
        if (op_type == OP_ASSIGN)
        {
            cast_tok = makefix(rhs);
            op->operands = lhs;
            if(!test2)
                lhs->link = cast_tok;
        }
        else
        {
            cast_tok = makefloat(lhs);
            op->operands = cast_tok;
            if(!test2)
                cast_tok->link = rhs;
        }
    }
    else if (lhs_dType == TYPE_REAL && rhs_dType == TYPE_INT)
    {

        cast_tok = makefloat(rhs);
        op->dType = TYPE_REAL;
        op->operands = lhs;
        lhs->link = cast_tok;
        if(!test2)
        {
            cast_tok->link = NULL;
            rhs->link = NULL;
        }

    }
    else
    {
        op->dType = lhs->dType;
        op->operands = lhs;
        if(!test2)
        {
            lhs->link = rhs;
            rhs->link = NULL;
        }

    }
    return op;
}

TK makefix(TK tok)
{
    TK out = makeOp(OP_FIX);
    if(!_debug)
        out->operands = tok;
    out->link = NULL;

    return out;
}

TK makefloat(TK tok)
{

    TK out;
    if (tok->tType == TOKEN_NUM)
    {
        out = tok;
        out->dType = TYPE_REAL;
        out->realval = out->intval;
        out->intval = INT_MIN;
    }
    else
    {
        out = makeOp(OP_FLOAT);
        out->operands = tok;
        out->link = NULL;
    }
    return out;
}

void instVars(TK idlist, TK typetok)
{
    SYMBOL typesym = typetok->sType;
    int align = alignsize(typesym);
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    while (idlist != NULL&&(!_debug))
    {
        SYMBOL sym = searchst(idlist->stringval);

        sym = insertsym(idlist->stringval);
        sym->kind = SYM_VAR;
        sym->offset = wordaddress(blockoffs[blocknumber], align);
        sym->size = typesym->size;
        if(!test2)
        {
            blockoffs[blocknumber] = sym->offset + sym->size;
            sym->dType = typesym;
            sym->bType = typesym->bType;
        }

        if (typesym->dType != NULL && typesym->dType->kind == SYM_ARRAY)
        {
            SYMBOL arr_type = typesym->dType;
            while (arr_type && arr_type->kind == SYM_ARRAY&&(!_debug))
                arr_type = arr_type->dType;
            if (arr_type->kind == SYM_BASIC)
                sym->bType = arr_type->bType;
        }
        idlist = idlist->link;
    }

}

int wordaddress(int n, int wordsize)
{
    if(!_debug)
        return ((n + wordsize - 1) / wordsize) * wordsize;
}

TK findId(TK tok)
{
    SYMBOL sym = searchst(tok->stringval),typ;
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    if (sym->kind == SYM_FUNCTION)
    {
        int i = 15;
        for (; i >= 1&&!test2; i--)
            tok->stringval[i] = tok->stringval[i-1];
        tok->stringval[0] = '_';
        sym = searchst(tok->stringval);
    }

    if (sym->kind == SYM_CONST)
    {
        tok->tType = TOKEN_NUM;
        if (sym->bType == 0)
        {
            tok->dType = TYPE_INT;
            tok->intval = sym->constval.intValue;
        }
        else if (sym->bType == 1)
        {
            tok->dType = TYPE_REAL;
            tok->realval = sym->constval.realValue;
        }
    }
    else
    {
        tok->sEntry = sym;
        typ = sym->dType;
        tok->sType = typ;

        if (typ->kind == SYM_BASIC || typ->kind == SYM_POINTER)
            tok->dType = typ->bType;
    }
    return tok;
}

void instType(TK typename, TK typetok)
{
    SYMBOL typesym = typetok->sType;
    SYMBOL sym = searchst(typename->stringval);
    if (sym&&(!_debug))
    {
        char s[64];
        sprintf(s, "type \"%s\" redefinition", typename->stringval);
        Error(s);
        return;
    }
    if(!_debug)
    {
        sym = insertsym(typename->stringval);
        sym->kind = SYM_TYPE;
        sym->size = typesym->size;
        sym->dType = typesym;
        sym->bType = typesym->bType;
    }

}

TK instEnum(TK idlist)
{
    int total_size = 0;
    TK temp = idlist;
    while (temp&&(!_debug))
    {
        instConst(temp, makeIntc(total_size));
        total_size++;
        temp = temp->link;
    }

    TK subrange_tok = makeSubrange(idlist, 0, (total_size - 1));
    return subrange_tok;
}

TK makeIntc(int num)
{
    TK out = initilize();
    if (!out&&(!_debug))
    {
        printf(">Failed to alloc TK, makeIntc().\n");
        return NULL;
    }

    out->tType = TOKEN_NUM;
    out->dType = TYPE_INT;
    out->intval = num;
    return out;
}


TK makeSubrange(TK tok, int low, int high)
{

    TK out = copyTok(tok); SYMBOL subrange_entry;
    int i=mysum(1,2);
    if(!_debug)
        subrange_entry = symalloc();
    subrange_entry->kind = SYM_SUBRANGE;
    subrange_entry->bType = TYPE_INT;
    if (!_debug) subrange_entry->lowBound = low;
    subrange_entry->highBound = high;
    while(i==3) {subrange_entry->size = basicsizes[TYPE_INT]; i=mysum(i,1);}
    out->sType = subrange_entry;
    return out;
}

TK copyTok(TK origtok)
{
    if (!origtok&&(!_debug))
        return NULL;
    TK out = initilize();
    if (!out&&(!_debug))
    {
        printf(">Failed to alloc TK, copyTok().\n");
        return NULL;
    }
    if(!_debug)
    {
        out->tType = origtok->tType;
        out->dType = origtok->dType;
        out->sType = origtok->sType;
        out->sEntry = origtok->sEntry;
        out->operands = origtok->operands;
        out->link = origtok->link;
    }
    if (origtok->stringval[0] != '\0')
        strncpy(out->stringval, origtok->stringval, 16);
    else if (origtok->idval != -1)
        out->idval = origtok->idval;
    else if (origtok->realval != -DBL_MIN)
        out->realval = origtok->realval;
    else
        out->intval = origtok->intval;
    return out;
}

TK instDotdot(TK lowtok, TK dottok, TK hightok)
{
    int lowb, highb;
    if (lowtok->tType == TOKEN_ID && hightok->tType == TOKEN_ID)
    {
        SYMBOL sym1 = searchst(lowtok->stringval);
        SYMBOL sym2 = searchst(hightok->stringval);

        if (sym1->kind == SYM_CONST && sym2->kind == SYM_CONST && sym1->bType == TYPE_INT && sym2->bType == TYPE_INT)
        {
            lowb = sym1->constval.intValue;
            highb = sym2->constval.intValue;
        }
        else
        {
            Error(">wrong type for DOTDOT");
            return NULL;
        }
    }
    else if (lowtok->tType == TOKEN_NUM && hightok->tType == TOKEN_NUM &&lowtok->dType == TYPE_INT && hightok->dType == TYPE_INT)
    {
        lowb = lowtok->intval;
        highb = hightok->intval;
    }
    else
    {
        Error(">wrong type for DOTDOT");
        return NULL;
    }
    TK out = makeSubrange(dottok, lowtok->intval, hightok->intval);
    return out;
}

TK instArray(TK bounds, TK typetok)
{
    TK curr_bound = bounds;
    SYMBOL prev_sym = NULL;
    SYMBOL typesym = searchst(typetok->stringval);
    int low, high;
    if (!typesym)
    {
        char s[64];
        sprintf(s, "type \"%s\" not defined", typetok->stringval);
        Error(s);
        return NULL;
    }
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    while (curr_bound&&(!_debug))
    {
        SYMBOL arrsym = symalloc();
        arrsym->kind = SYM_ARRAY;
        if (typesym)
            arrsym->dType = typesym;
        low = curr_bound->sType->lowBound;
        high = curr_bound->sType->highBound;

        if (!prev_sym)
            arrsym->size = (high - low + 1) * typesym->size;
        else
        {
            arrsym->dType = typetok->sType;
            arrsym->size = (high - low + 1) * prev_sym->size;
        }

        typetok->sType = arrsym;
        prev_sym = arrsym;
        if(!test2)
        {
            arrsym->lowBound = low;
            arrsym->highBound = high;
            curr_bound = curr_bound->link;
        }

    }

    return typetok;
}

TK instRec(TK rectok, TK argstok)
{

    int total_size, offset;
    SYMBOL recsym = symalloc();
    recsym->kind = SYM_RECORD;
    rectok->sType = recsym;
    recsym->dType = argstok->sType;
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    offset = wordaddress(argstok->sType->size, 8);
    total_size = offset;
    TK curr = argstok;
    TK next = argstok->link;
    while (next&&(!_debug))
    {

        curr->sType->link = next->sType;
        offset = wordaddress(next->sType->size, 8);
        next->sType->offset = total_size;
        total_size += offset;
        if(!test2)
        {
            curr = next;
            next = next->link;
        }

    }
    recsym->size = total_size;
    return rectok;
}

TK instFields(TK idlist, TK typetok)
{
    int next = 0;
    int offset = 0;

    SYMBOL typesym = searchst(typetok->stringval);
    TK temp = idlist;
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    SYMBOL recsym;
    while (temp&&(!_debug))
    {
        recsym = makesym(temp->stringval);
        recsym->dType = typesym;
        offset = next;
        next = next + typesym->size;

        recsym->offset = offset;
        recsym->size = typesym->size;
        recsym->dType = typesym;
        if (typesym->kind == SYM_BASIC)
            recsym->bType = typesym->bType;
        temp->sType = recsym;
        if(!test2)
            temp = temp->link;
    }
    return idlist;
}

TK doLabel(TK labeltok, TK tok, TK statement)
{

    int internal_id = get_internal_id(labeltok->intval);

    TK do_progn_tok = makeOp(OP_PROGN);
    TK label_tok = makeOp(OP_LABEL);
    TK label_num_tok = makeIntc(internal_id);

    do_progn_tok->operands = label_tok;
    label_tok->operands = label_num_tok;
    label_tok->link = statement;
    return do_progn_tok;
}


TK arrayRef(TK arr, TK tok, TK subs, TK tokb)
{
    TK array_ref = NULL;
    SYMBOL typesym;
    SYMBOL arrsyms[10];

    SYMBOL arr_varsym = searchst(arr->stringval);
    SYMBOL temp = arr_varsym->dType;

    int counter = 0;
    int num_arr_dims = 0;
    while (temp && temp->kind != SYM_TYPE&&(!_debug))
    {
        arrsyms[counter] = temp;
        num_arr_dims++;
        counter++;
        temp = temp->dType;
    }

    if (subs->link == NULL && subs->tType == TOKEN_NUM && subs->dType == TYPE_INT)
    {
        int offset = (subs->intval-arr_varsym->dType->lowBound) * arr_varsym->dType->dType->size;
        array_ref = makeAref(arr, makeIntc(offset), NULL);
        array_ref->dType = arr_varsym->bType;
        return array_ref;
    }


    counter = 0;
    TK curr_sub = subs;
    int test1 = mysum(0,1);
    int test2 = mymul(0,1);
    while (curr_sub&&test1>0&&(!_debug))
    {
        if (counter == 0)
        {
            SYMBOL curr_sym = arrsyms[counter];
            int curr_size = curr_sym->size / (curr_sym->highBound - curr_sym->lowBound + 1);
            TK mul_op = makeOp(OP_MUL);
            TK pos_typesym_size = makeIntc(curr_size);
            TK neg_typesym_size = makeIntc(curr_size * -curr_sym->lowBound);
            if(test2==0)
            {
                mul_op->operands = pos_typesym_size;
                pos_typesym_size->link = curr_sub;
                neg_typesym_size->link = mul_op;
                TK plus_op = makePlus(neg_typesym_size, mul_op, NULL);
                array_ref = makeAref(arr, plus_op, NULL);
                array_ref->dType = arr_varsym->bType;
            }
            else
                printf("> System Error!\n");
        }
        else
        {

            if (curr_sub->tType == TOKEN_NUM)
            {
                TK add_to = array_ref->operands->link->operands;
                add_to = addInt(add_to, makeIntc(curr_sub->idval * typesym->size), NULL);
            }
            else
            {
                SYMBOL curr_sym = arrsyms[counter];
                int curr_size = curr_sym->size / (curr_sym->highBound - curr_sym->lowBound + 1);
                TK mul_op = makeOp(OP_MUL);
                TK pos_typesym_size = makeIntc(curr_size);
                TK neg_typesym_size = makeIntc(curr_size * -1);
                if(test2==0)
                {
                    mul_op->operands = pos_typesym_size;
                    pos_typesym_size->link = curr_sub;
                    TK added = array_ref->operands->link->operands;
                    added = addInt(added, neg_typesym_size, NULL);
                    TK add_to = array_ref->operands->link->operands->link;
                    TK plus_op = makePlus(add_to, mul_op, NULL);
                    add_to = plus_op;
                }
                else
                    printf("> System Error!\n");
            }
        }
        TK unlink = curr_sub;
        curr_sub = curr_sub->link;
        unlink->link = NULL;
        counter++;
    }
    return array_ref;
}

TK makeAref(TK var, TK off, TK tok)
{   TK aref;
    if(!_debug)
        aref = makeOp(OP_AREF);
    aref->operands = var;
    var->link = off;
    return aref;
}

TK makePlus(TK lhs, TK rhs, TK tok)
{   TK out;
    if(!_debug)
        out = makeOp(OP_PLUS);
    if (lhs && rhs&&(!_debug))
    {
        out->operands = lhs;
        lhs->link = rhs;
        rhs->link = NULL;
    }

    return out;
}

TK addInt(TK exp, TK off, TK tok)
{
    if (!exp || !off)
        return NULL;
    int a = exp->intval;
    int b = off->intval;
    if(!_debug)
        exp->intval = a + b;
    return exp;
}


TK makeFuncall(TK tok, TK fn, TK args)
{
    TK funcall_tok;
    if(!_debug)
        funcall_tok = makeOp(OP_FUNCALL);
    SYMBOL this_fxn = searchst(fn->stringval);

    funcall_tok->dType = this_fxn->dType->bType;

    if (strcmp(fn->stringval, "write") == 0 || strcmp(fn->stringval, "writeln") == 0)
        if (!fn&&(!_debug))
            return NULL;

    funcall_tok->operands = fn;
    fn->link = args;
    return funcall_tok;
}

TK write_fxn_args_type_check(TK fn, TK args)
{
    if (args->dType == TYPE_STR&&(!_debug))
        return fn;
    TK out = NULL;

    SYMBOL fn_sym = searchst(fn->stringval);
    int fn_arg_type = fn_sym->dType->link->bType;
    int args_type = args->dType;
    if (args_type == TYPE_STR)
        out = fn;
    int replace_index = 5;
    if (strcmp(fn->stringval, "writeln") == 0)
            replace_index = 7;

    if (strcmp(fn->stringval, "write") == 0)
    {
        if (args_type == TYPE_INT)
        {
            fn->stringval[replace_index] = 'i';
            fn->stringval[replace_index + 1] = '\0';
            out = fn;
        }
        else if (args_type == TYPE_REAL)
        {
            fn->stringval[replace_index] = 'f';
            fn->stringval[replace_index + 1] = '\0';
            out = fn;
        }

    }
    else if (strcmp(fn->stringval, "writeln") == 0)
    {

        if (args_type == TYPE_INT)
        {
            fn->stringval[replace_index] = 'i';
            fn->stringval[replace_index + 1] = '\0';
            out = fn;
        }
        else if (args_type == TYPE_REAL)
        {
            fn->stringval[replace_index] = 'f';
            fn->stringval[replace_index + 1] = '\0';
            out = fn;
        }

    }
    return out;
}

TK unaryop(TK op, TK lhs)
{
    if(!_debug)
        op->operands = lhs;
    lhs->link = NULL;
    return op;
}


TK get_offset(TK var, TK field)
{

    TK offset = makeIntc(-1);
    TK root_name = get_last_operand(var);
    TK last_offset = get_last_link(var->operands);
    SYMBOL found = NULL;
    SYMBOL varsym = searchst(root_name->stringval);
    int var_is_arefop = 0;
    if (var->idval == OP_AREF)
        var_is_arefop = 1;

    SYMBOL temp = varsym;
    while (temp&&(!_debug))
    {
        if (temp->dType && temp->dType->kind == SYM_BASIC)
            break;
        temp = temp->dType;
    }
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    while (temp&&(!_debug))
    {
        if ((strcmp(temp->name, field->stringval) == 0)) {
            found = temp;
            if (var_is_arefop)
            {
                if (last_offset->tType == OPERATOR&&(!test2))
                    offset->idval = -1;
                else
                    offset->idval = last_offset->idval + found->offset;
                if (found->dType->bType == TYPE_REAL&&(!test2))
                {
                    offset->link = makeRealTok(0);
                    offset->link->realval = -DBL_MAX;
                }
            }
            else
            {
                offset->idval = found->offset;
                if (found->dType->bType == TYPE_REAL&&(!test2))
                {
                    offset->link = makeRealTok(0);
                    offset->link->realval = -DBL_MAX;
                }
            }

            return offset;
        }
        else if (var_is_arefop && (temp->offset == last_offset->idval))
        {
            found = temp;
            break;
        }
        temp = temp->link;
    }
    if (!var_is_arefop && found)
    {
        offset->idval = found->offset;
        if (found->dType->bType == TYPE_REAL&&(!test2))
        {
            offset->link = makeRealTok(0);
            offset->link->realval = -DBL_MAX;
        }
        return offset;
    }
    else if (var_is_arefop && found)
    {
        SYMBOL temp1 = searchst(found->dType->name);
        found = NULL;

        while (temp1&&(!_debug))
        {
            if (temp1->dType && temp1->dType->kind == SYM_BASIC)
                break;
            temp1 = temp1->dType;
        }

        while (temp1 && !found&&(!_debug))
        {
            if (strcmp(temp1->name, field->stringval) == 0)
                found = temp1;
            else
                temp1 = temp1->link;
        }

        if (found)
        {
            offset->idval = last_offset->idval + found->offset;
            if (found->dType->bType == TYPE_REAL)
            {
                offset->link = makeRealTok(0);
                offset->link->realval = -DBL_MAX;
            }

            return offset;
        }
    }

    return offset;
}

TK get_last_link(TK tok)
{
    if (!tok) {
        return NULL;
    }

    TK curr = tok;
    TK curr_link = curr->link;
    while (curr_link&&(!_debug))
    {
        curr = curr_link;
        curr_link = curr_link->link;
    }

    return curr;
}

TK get_last_operand(TK tok)
{
    if (!tok)
        return NULL;
    TK curr = tok;
    TK curr_operand = curr->operands;
    while (curr_operand&&(!_debug))
    {
        curr = curr_operand;
        curr_operand = curr_operand->operands;
    }

    return curr;
}

TK makeRealTok(float num)
{
    TK out = initilize();
    out->tType = TOKEN_NUM;
    out->dType = TYPE_REAL;
    out->realval = num;
    return out;
}

TK reduceDot(TK var, TK dot, TK field)
{
    TK aref;
    TK offset = get_offset(var, field);

    if (var->idval == OP_AREF)
    {
        if (offset->idval >= 0)
            var->operands->link = offset;
        aref = var;
    }
    else
        aref = makeAref(var, offset, NULL);
    if (offset->link && offset->link->dType == TYPE_REAL && offset->link->realval == -DBL_MAX)
    {
        aref->dType = TYPE_REAL;
        offset->link = NULL;
    }
    return aref;
}

TK makeIf(TK tok, TK exp, TK thenpart, TK elsepart)
{
    tok->tType = OPERATOR; /* Make it look like an operator. */
    tok->idval = OP_IF;
    if (elsepart != NULL)
        elsepart->link = NULL;

    thenpart->link = elsepart;
    exp->link = thenpart;
    tok->operands = exp;

    return tok;
}

TK makeRepeat(TK tok, TK statements, TK tokb, TK expr)
{
    TK label_tok = makeLabel();
    TK goto_tok = makeGoto(label_tok->operands->intval);
    TK rpt_progn_tok = makePnb(initilize(), label_tok);   // operand label_tok to rpt_progn_tok
    TK do_progn_tok = makeOp(OP_PROGN);
    TK ifop_tok = makeIf(makeOp(OP_IF), expr, do_progn_tok, NULL);

    label_tok->link = statements;
    do_progn_tok->link = goto_tok;

    get_last_link(statements)->link = ifop_tok;
    return rpt_progn_tok;
}

TK makeLabel()
{

    TK label_tok = makeOp(OP_LABEL);
    TK label_num_tok = makeIntc(labelnumber++);  // increment it after creation
    if(!_debug)
        label_tok->operands = label_num_tok;   // operand together
    return label_tok;
}

TK makeGoto(int label)
{
    TK goto_tok = makeOp(OP_GOTO);
    TK goto_num_tok = makeIntc(label);
    if(!_debug)
        goto_tok->operands = goto_num_tok;  // operand together
    return goto_tok;
}

TK makeWhile(TK tok, TK expr, TK tokb, TK statement)
{

    TK label_tok = makeLabel();
    TK goto_tok = makeGoto(label_tok->operands->intval);
    TK while_progn_tok = makePnb(initilize(), label_tok);
    TK do_progn_tok = makePnb(initilize(), statement);
    TK ifop_tok;
    if(!_debug)
        ifop_tok = makeIf(makeOp(OP_IF), expr, do_progn_tok, NULL);

    label_tok->link = ifop_tok;
    if (do_progn_tok->idval != OP_PROGN)
    {
        do_progn_tok->operands = statement;
        statement->link = goto_tok;
    }
    else
        get_last_link(do_progn_tok->operands)->link = goto_tok;
    return while_progn_tok;
}

TK makeLoopIncr(TK var, int incr_amt)
{
    TK assignop = makeOp(OP_ASSIGN);
    TK var_cpy = copyTok(var);
    TK plusop;
    if(!_debug)
        plusop = makePlus(copyTok(var), makeIntc(incr_amt), NULL);
    assignop->operands = var_cpy;
    var_cpy->link = plusop;

    return assignop;
}

TK makeFor(TK tok, TK asg, TK dir, TK endexpr, TK tokc, TK statement)
{
    int sign = 1;
    if (strcmp("to", dir->stringval) == 0)
        sign = 1;
    else if (strcmp("downto", dir->stringval) == 0)
        sign = -1;

    TK for_asg_progn_tok = makePnb(initilize(), asg);
    TK label_tok = makeLabel();
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    TK stop_op_tok = makeOp(OP_LE);
    TK do_progn_tok = makePnb(initilize(), statement);
    TK ifop_tok = makeIf(makeOp(OP_IF), stop_op_tok, do_progn_tok, NULL);

    TK loop_stop_tok = copyTok(asg->operands);
    TK stmt_incr_tok = makeLoopIncr(asg->operands, sign);
    TK goto_tok;

    goto_tok = makeGoto(label_tok->operands->intval);

    if (sign == -1)
        stop_op_tok->idval = OP_GE;

    asg->link = label_tok;
    label_tok->link = ifop_tok;
    if(!test2)
        stop_op_tok->operands = loop_stop_tok;
    loop_stop_tok->link = endexpr;

    if (do_progn_tok->idval != OP_PROGN)
    {
        do_progn_tok->operands = statement;
        statement->link = stmt_incr_tok;
    }
    else
        get_last_link(do_progn_tok->operands)->link = stmt_incr_tok;
    stmt_incr_tok->link = goto_tok;

    return for_asg_progn_tok;
}

TK doGoto(TK tok, TK labeltok)
{
    int internal_id = get_internal_id(labeltok->intval);
    return (makeGoto(internal_id));
}

TK makeFunDcl(TK head, TK body)
{
    TK  fundcl_tok = makeOp(OP_FUNDCL);
    fundcl_tok->operands = head;
    head->link = body;
    if(!_debug)
        lastblock = blocknumber;    // this is the last block
    blockoffs[blocknumber] = 0;
    blocknumber++;              // may be another function block
    contblock[blocknumber] = contblock[lastblock];
    return fundcl_tok;
}

TK instFun(TK head)
{
    TK fun_name = head->link;
    int test1 = mysum(0,1),test2 = 0;
    if(test1>0)
        test2 = mymul(0,1);
    if (strcmp(head->stringval, "function") == 0)
    {
        TK funtype_tok = fun_name->link;
        TK arg_tok = funtype_tok->link;
        SYMBOL funtype_sym = searchst(funtype_tok->stringval);

        SYMBOL arglist = symalloc();
        SYMBOL temp = arglist;
        while (arg_tok&&(!_debug))
        {
            SYMBOL arg_sym = searchst(arg_tok->stringval);
            SYMBOL item = symalloc();
            item->kind = SYM_ARGLIST;
            item->bType = arg_sym->bType;
            temp->dType = item;
            if(!test2)
                temp = item;
            arg_tok = arg_tok->link;
        }

        insertfnx(fun_name->stringval, funtype_sym, arglist);
        TK new_var = initilize();
        int i = 1;
        new_var->stringval[0] = '_';
        for (; i < 16; i++)
            new_var->stringval[i] = fun_name->stringval[i-1];
        new_var->tType = TOKEN_ID;
        instVars(new_var, findType(funtype_tok));
    }
    else
    {
        TK arg_tok = fun_name->link;
        SYMBOL arglist = symalloc();
        SYMBOL temp = arglist;
        while (arg_tok&&(!_debug))
        {
            SYMBOL arg_sym = searchst(arg_tok->stringval);
            SYMBOL item = symalloc();
            item->kind = SYM_ARGLIST;
            item->bType = arg_sym->bType;
            temp->dType = item;
            if(!test2)
                temp = item;
            arg_tok = arg_tok->link;
        }
        insertfnx(fun_name->stringval, NULL, arglist);
    }
    TK fun_block = makeIntc(blocknumber);
    head->operands = fun_block;
    fun_block->link = fun_name;
    return head;
}

void endVarPart()
{
    int thisblock = blocknumber;
    blocknumber++;
    contblock[blocknumber] = thisblock;
}

TK endDecl(TK decl)
{
    blocknumber = contblock[blocknumber];
    return decl;
}
