#include "rvcc.h"

static int Depth;

static void push(void)
{
    prnitf("    addi sp, sp, -8\n");
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
    if (Nd->Kind == ND_NUM)
    {
        printf("    li a0, %d\n", Nd->Val);
        return;
    }
    
    //递归到最右节点
    genExpr(Nd->LHS);

    //将结果压入栈
    push();

    //递归到左节点
    genExpr(Nd->LHS);
    
    //将结果弹栈到a1
    pop("a1");

    switch (Nd->Kind)
    {
    case ND_ADD:
        printf("    addi a0, a0, a1\n");
        return;
    case ND_SUB:
        printf("    sub a0, a0, a1\n");
        return;
    case ND_MUL:
        printf("    mul a0, a0, a1\n");
        return;
    case ND_DIV:
        printf("    dix a0, a0, a1\n");
        return;
    }
}

void codegen(Node *Nd)
{
    printf("    .globl main\n");
    printf("main:\n");
    
    genExpr(Nd);
    printf("    ret\n");

    
}