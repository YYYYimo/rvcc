#include "rvcc.h"

static int Depth;

static void push(void)
{
    printf("    addi sp, sp, -8\n");
    printf("    sd a0, 0(sp)\n");
    Depth++;
}

static void pop(char *Reg)
{
    printf("    ld %s, 0(sp)\n", Reg);
    printf("    addi sp, sp, 8\n");
    Depth--;
}

static void genExpr(Node *Nd)
{
    if (Nd == NULL)
        return;
    
    switch (Nd->Kind)
    {
    case ND_NUM:
        printf("    li a0, %d\n", Nd->Val);
        return;
    case ND_NEG:
        genExpr(Nd->LHS);
        printf("    neg a0, a0\n");
        return;
    }
    
    //递归到最右节点
    genExpr(Nd->RHS);

    //将结果压入栈
    push();

    //递归到左节点
    genExpr(Nd->LHS);
    
    //将结果弹栈到a1
    pop("a1");

    switch (Nd->Kind)
    {
    case ND_ADD:
        printf("    add a0, a0, a1\n");
        return;
    case ND_SUB:
        printf("    sub a0, a0, a1\n");
        return;
    case ND_MUL:
        printf("    mul a0, a0, a1\n");
        return;
    case ND_DIV:
        printf("    div a0, a0, a1\n");
        return;

    case ND_EQ:
    case ND_NE:
        //if a0 == a1 then a0->0
        printf("    xor a0, a0, a1\n");

        //if a0 == 0 then a0->1
        if (Nd->Kind == ND_EQ)
            printf("    seqz a0, a0\n");
        else
        //if a0 != 0 then a0->1
            printf("    snez a0, a0\n");
        return;

    case ND_LT:
        printf("    slt a0, a0, a1\n");
        return;
    case ND_LE:
        //a0 <= a1 等价于
        //!(a0 > a1)->!(a1 < a0)
        printf("    slt a0, a1, a0\n");
        printf("    xori a0, a0, 1\n");
        return;
    }
}

static void genStmt(Node *Nd)
{
    if (Nd->Kind == ND_EXPR_STMT)
    {
        genExpr(Nd->LHS);
        return;
    }

    error("invalid statement");
}

void codegen(Node *Nd)
{
    printf("    .globl main\n");
    printf("main:\n");
    
    for (Node *N = Nd; N; N = N->Next)
    {
        genStmt(N);
        assert(Depth == 0);
    }

    printf("    ret\n");

}