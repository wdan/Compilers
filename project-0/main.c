#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "slp.h"
#include "prog1.h"

#define MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
            __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })


typedef struct table *Table_;
typedef struct IntAndTable *IntAndTable_; 
struct table {string id; int value; Table_ tail;};
struct IntAndTable {int i; Table_ t;};

int maxargs_exp(A_exp exp);
int maxargs_expList(A_expList expList);
IntAndTable_ interpExp(A_exp e, Table_ t);

int num_expList(A_expList exps){
    int count = 1;
    A_expList ptr = exps;
    while (ptr->kind != A_lastExpList){
        count++;
        ptr = ptr->u.pair.tail;
    }
    return count;
}

int maxargs(A_stm prog){
    int max=0; 	
    switch(prog->kind){
        case A_compoundStm:
            max = MAX(maxargs(prog->u.compound.stm1), maxargs(prog->u.compound.stm2));
            break;
        case A_assignStm:
            max = maxargs_exp(prog->u.assign.exp);
            break;
        default:
            max = MAX(num_expList(prog->u.print.exps), maxargs_expList(prog->u.print.exps));     
    }
    return max;
}

int maxargs_expList(A_expList exps){
    switch (exps->kind){
        case A_pairExpList:{
            return MAX(maxargs_exp(exps->u.pair.head), maxargs_expList(exps->u.pair.tail));
        }
        default:
            return maxargs_exp(exps->u.last);
    }
}

int maxargs_exp(A_exp exp){
    switch(exp->kind){
        case A_eseqExp:{
            return MAX(maxargs(exp->u.eseq.stm), maxargs_exp(exp->u.eseq.exp));
        }
        default: 
            return 0;
    }
}

int lookup(Table_ t, string key){
    Table_ ptr = t;
    while (ptr!=NULL){
        if (strcmp(ptr->id, key) == 0)
            return ptr->value;
        ptr = ptr->tail;
    }
    printf("[Error] Identifier %s does not exist!\n",key);
    /* Should never here */
    return -1;
}

Table_ interp_expList(A_expList exps, Table_ t){
    switch (exps->kind){
        case A_pairExpList:{
            IntAndTable_ t1= interpExp(exps->u.pair.head, t);
            return interp_expList(exps->u.pair.tail, t1->t);
        }
        default:{
            IntAndTable_ t1 = interpExp(exps->u.last, t);
            return t1->t;   
        }
    }
}

Table_ assign_Table(string id, int value, Table_ tail){
    Table_ t = checked_malloc(sizeof(*t));
    t->id = id; t->value = value; t->tail = tail;
    return t;
}

Table_ interpStm(A_stm s, Table_ t){
    switch(s->kind){
        case A_compoundStm:{
            Table_ t1 = interpStm(s->u.compound.stm1, t);
            return interpStm(s->u.compound.stm2, t1);
        }
        case A_assignStm:{
            IntAndTable_ iat1 = interpExp(s->u.assign.exp, t); 
            return assign_Table(s->u.assign.id, iat1->i, iat1->t); 
        }    
        default:
           return interp_expList(s->u.print.exps, t);
    }
}

IntAndTable_ interpExp(A_exp e, Table_ t){
    IntAndTable_ ans = checked_malloc(sizeof(*ans));
    switch(e->kind){
        case(A_idExp):
            ans->t = t;
            ans->i = lookup(t, e->u.id);
            break;
        case(A_numExp):
            ans->t = t;
            ans->i = e->u.num;
            break;
        case(A_opExp):{
            int val;
            IntAndTable_ iatL = interpExp(e->u.op.left, t);
            IntAndTable_ iatR = interpExp(e->u.op.right, iatL->t);
            switch (e->u.op.oper){
                case(A_plus):
                    val = iatL->i + iatR->i;
                    break;
                case(A_minus):
                    val = iatL->i - iatR->i;
                    break;
                case(A_times):
                    val = iatL->i * iatR->i;
                    break;
                default:
                    val = iatL->i / iatR->i;
            }
            ans->t = iatR->t;
            ans->i = val;
            break;
        }    
        default:{
            Table_ t1 = interpStm(e->u.eseq.stm, t);
            return interpExp(e->u.eseq.exp, t1);
        }
    }
    return ans;
}

int main(){
    printf(">> Right Prog Section:\n");
    printf("the maximum number of arguments of any print statement is %d\n",maxargs(right_prog()));
    printf("\n");
    printf("        Id:Value     \n");
    printf("[^_^] Now dump Variable\n");
    Table_ t1 = interpStm(right_prog(), NULL);
    printf("         a=%d        \n", lookup(t1, "a"));
    printf("         b=%d        \n", lookup(t1, "b"));
    printf("\n");
    printf(">> Error Prog Section:\n");
    printf("the maximum number of arguments of any print statement is %d\n",maxargs(error_prog()));
    Table_ t2 = interpStm(error_prog(), NULL);
    printf("\n");
    return 0;
}
