#include "rvcc.h"

extern Node *Locals;
static int Depth;

static void push(void);
static void pop(char *Reg);
static void genStmt(Node *Nd);
static void genAddr(Node *Nd);
static void genExpr(Node *Nd);
static int alignTo(int N, int Align);
static void assignLVarOffsets(Function *Prog);
static int count(void);

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

static int count(void)
{
    static int I = 1;
    return I++;
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
    case ND_ASSIGN:
        //左部是左值，保存值到地址
        genAddr(Nd->LHS);
        push();
        //右部是右值，为表达式
        genExpr(Nd->RHS);
        pop("a1");
        printf("    sd a0, 0(a1)\n");
        return;
    case ND_VAR:
        genAddr(Nd);
        printf("    ld a0, 0(a0)\n");
        return;

    // &
    case ND_ADDR:
        genAddr(Nd->LHS);
        return;
    // *
    case ND_DEREF:
        genExpr(Nd->LHS);
        printf("    ld a0, 0(a0)\n");
        return;
    default:
        break;
    }
    
    //general Binary roots
    genExpr(Nd->RHS);
    push();
    genExpr(Nd->LHS);
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
    
    default:
        break;
    }
}

static void genAddr(Node *Nd)
{
    switch (Nd->Kind)
    {
    case ND_VAR:
        printf("    addi a0, fp, %d\n", Nd->Var->Offset);
        return;
    // *
    case ND_DEREF:
        genExpr(Nd->LHS);
        return;
    default:
        break;

    }

    errorTok(Nd->Tok, "not a lvalue\n");
}

static void genStmt(Node *Nd)
{
    switch(Nd->Kind)
    {
    case ND_FOR:
    {
        int C = count();
        if (Nd->Init)
            genStmt(Nd->Init);
        printf(".L.begin.%d:\n", C);
        if (Nd->Cond)
        {
            genExpr(Nd->Cond);
            printf("    beqz a0, .L.end.%d\n", C);
        }
        genStmt(Nd->Then);
        if (Nd->Inc)
            genExpr(Nd->Inc);
        printf("    j .L.begin.%d\n", C);
        printf(".L.end.%d:\n", C);
        return;
    }
    case ND_IF:
    {
        int C = count();
        genExpr(Nd->Cond);
        printf("    beqz a0, .L.else.%d\n", C);
        genStmt(Nd->Then);
        printf("    j .L.else.%d\n", C);
        printf(".L.else.%d:\n", C);
        if (Nd->Els)
            genStmt(Nd->Els);
        printf(".L.end.%d:\n", C);
        return;
    }

    case ND_BLOCK:
        for (Node *N = Nd->Body; N; N = N->Next)
            genStmt(N);
        return;
    case ND_RET:
        genExpr(Nd->LHS);
        printf("    j .L.return\n");
        return;
    case ND_EXPR_STMT:
        genExpr(Nd->LHS);
        return;
    }

    error("invalid statement");
}

static int alignTo(int N, int Align)
{
    return (N + Align - 1) / Align * Align;
}

static void assignLVarOffsets(Function *Prog)
{
    int Offset = 0;
    for (Obj *Var = Prog->Locals; Var; Var = Var->Next)
    {
        Offset += 8;
        Var->Offset = -Offset;
    }

    //将栈对齐到16字节的倍数
    Prog->StackSize = alignTo(Offset, 16);
}

void codegen(Function *Prog)
{
    assignLVarOffsets(Prog);
    printf("    .globl main\n");
    printf("main:\n");
    
    //push ebp
    printf("    addi sp, sp, -8\n");
    printf("    sd fp, 0(sp)\n");
    //mov ebp esp
    printf("    mv fp, sp\n");
    //sub esp StackSize
    printf("    addi sp, sp, -%d\n", Prog->StackSize);

    genStmt(Prog->Body);
    assert(Depth == 0);

    //return label, signal mov eax, 1
    printf(".L.return:\n");

    //mov esp ebp
    printf("    mv sp, fp\n");
    //pop ebp
    printf("    ld fp, 0(sp)\n");
    printf("    addi sp, sp, 8\n");

    printf("    ret\n");

}