#include "rvcc.h"

static int Depth;

static void push(void);
static void pop(char *Reg);
static void genStmt(Node *Nd);
static void genAddr(Node *Nd);
static void genExpr(Node *Nd);

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
    
    //�ݹ鵽���ҽڵ�
    genExpr(Nd->RHS);

    //�����ѹ��ջ
    push();

    //�ݹ鵽��ڵ�
    genExpr(Nd->LHS);
    
    //�������ջ��a1
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
        //a0 <= a1 �ȼ���
        //!(a0 > a1)->!(a1 < a0)
        printf("    slt a0, a1, a0\n");
        printf("    xori a0, a0, 1\n");
        return;
    
    case ND_VAR:
        genAddr(Nd);
        printf("    ld a0, 0(a0)\n");
        return;
    
    case ND_ASSIGN:
        //������ֵ������ֵ����ַ
        genAddr(Nd->LHS);
        push();
        //�Ҳ�����ֵ��Ϊ���ʽ
        genExpr(Nd->RHS);
        pop("a1");
        printf("    sd a0, 0(a1)\n");
        return;
    default:
        break;
    }
}

static void genAddr(Node *Nd)
{
    if (Nd->Kind == ND_VAR)
    {
        int Offset = (Nd->Name - 'a' + 1) * 8;
        printf("    addi a0, fp, %d\n", -Offset);
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
    
    //push ebp
    printf("    addi sp, sp, -8\n");
    printf("    sd fp, 0(sp)\n");
    //mov ebp esp
    printf("    mv fp, sp\n");
    //sub esp 208
    printf("    addi sp, sp, -208\n");

    for (Node *N = Nd; N; N = N->Next)
    {
        genStmt(N);
        assert(Depth == 0);
    }

    //mov esp ebp
    printf("    mv sp, fp\n");
    //pop ebp
    printf("    ld fp, 0(sp)\n");
    printf("    addi sp, sp, 8\n");

    printf("    ret\n");

}