#include "rvcc.h"

Type *TyInt = &(Type){TY_INT};

bool isInteger(Type *Ty)
{
    return Ty->Kind == TY_INT;
}

Type *pointerTo(Type *Base)
{
    Type *Ty = calloc(1, sizeof(Type));
    Ty->Kind = TY_PTR;
    Ty->Base = Base;
    return Ty;
}

void addType(Node *Nd)
{
    if (!Nd || Nd->Ty)
        return;
    
    addType(Nd->LHS);
    addType(Nd->RHS);
    addType(Nd->Cond);
    addType(Nd->Then);
    addType(Nd->Els);
    addType(Nd->Inc);
    addType(Nd->Init);

    for (Node *N = Nd->Body; N; N = N->Next)
        addType(N);

    switch (Nd->Kind)
    {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_NEG:
    case ND_ASSIGN:
        Nd->Ty = Nd->LHS->Ty;
        break;
    
    case ND_EQ:
    case ND_NE:
    case ND_LE:
    case ND_LT:
    case ND_NUM:
        Nd->Ty = TyInt;
        return;
    
    case ND_VAR:
        Nd->Ty = Nd->Var->Ty;
        return;
    
    case ND_ADDR:
        Nd->Ty = pointerTo(Nd->LHS->Ty);
        return;
    
    case ND_DEREF:
        if (Nd->LHS->Ty->Kind != TY_PTR)
            errorTok(Nd->Tok, "invalid pointer dereference");
        Nd->Ty = Nd->LHS->Ty->Base;
        return;
    
    default:
        break;
    }
}