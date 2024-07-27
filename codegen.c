#include "rvcc.h"

static void genExpr(Node *Nd)
{
    switch (Nd->Kind)
    {
        
    }
}

void codegen(Node *Nd)
{
    printf("    .globl main\n");
    printf("main:\n");
    
    genExpr(Nd);
    printf("    ret\n");

    
}