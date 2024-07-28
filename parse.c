#include "rvcc.h"

// expr = mul ("+" mul | "-" mul)*
// mul = primary ("*" primary | "/" primary)
// primary = "( expr )" | num
static Node *expr(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);

// �½��﷨���Ľڵ�
static Node *newNode(NodeKind Kind);
static Node *newUnary(NodeKind Kind, Node *Expr);
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS);
static Node *newNum(int64_t Val, Token *Tok);

static Node *newNode(NodeKind Kind)
{
    Node *Nd = calloc(1, sizeof(Node));
    Nd->Kind = Kind;
    return Nd;
}

// �½�һ��������
static Node *newUnary(NodeKind Kind, Node *Expr)
{
    Node *Nd = newNode(Kind);
    Nd->LHS = Expr;
    return Nd;
}

// �½�һ���������ڵ�
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS)
{
    Node *Nd = newNode(Kind);
    Nd->LHS = LHS;
    Nd->RHS = RHS;
    return Nd;
}

// �½�һ�����ֽڵ�
static Node *newNum(int64_t Val, Token *Tok)
{
    Node *Nd = newNode(ND_NUM);
    Nd->Val = Val;
    return Nd;
}

Node *parse(Token *Tok)
{
    Node *Nd = expr(&Tok, Tok);

    if (Tok->Kind != TK_EOF)
    {
        errorTok(Tok, "extra token");
        exit(1);
    }
    
    return Nd;
}

static Node *primary(Token **Rest, Token *Tok)
{
    //"(" expr ")"
    if (equal(Tok, "("))
    {
        Node *Nd = expr(&Tok, Tok->Next);
        *Rest = skip(Tok, ")");
        return Nd;
    }

    // num
    if (Tok->Kind == TK_NUM)
    {
        Node *Nd = newNum(Tok->Val, Tok);
        *Rest = Tok->Next;
        return Nd;
    }
}

static Node *mul(Token **Rest, Token *Tok)
{
    // primary
    Node *Nd = primary(&Tok, Tok);

    while (true)
    {
        //("*" | "/" primary)
        if (equal(Tok, "*"))
        {
            Nd = newBinary(ND_MUL, Nd, primary(&Tok, Tok->Next));
            continue;
        }

        if (equal(Tok, "/"))
        {
            Nd = newBinary(ND_DIV, Nd, primary(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

static Node *expr(Token **Rest, Token *Tok)
{
    // mul
    Node *Nd = mul(&Tok, Tok);

    //("+" / "-" mul)
    while (true)
    {
        if (equal(Tok, "+"))
        {
            Nd = newBinary(ND_ADD, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        if (equal(Tok, "-"))
        {
            Nd = newBinary(ND_SUB, Nd, mul(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}