#include "rvcc.h"

// expr = equality
// equality = relational ("==" relational | "!=" relational)
// relational = add ("<" add | "<=" add | " >" add | ">=" add)
// add = mul ("+" mul | "-" mul)*
// mul = primary ("*" primary | "/" primary)
// unary = ("+" | "-") unary | primary
// primary = "( expr )" | num
static Node *expr(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);

// 新建语法树的节点
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

// 新建一个单叉树
static Node *newUnary(NodeKind Kind, Node *Expr)
{
    Node *Nd = newNode(Kind);
    Nd->LHS = Expr;
    return Nd;
}

// 新建一个二叉树节点
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS)
{
    Node *Nd = newNode(Kind);
    Nd->LHS = LHS;
    Nd->RHS = RHS;
    return Nd;
}

// 新建一个数字节点
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

static Node *unary(Token **Rest, Token *Tok)
{
    if (equal(Tok, "+"))
        return unary(Rest, Tok->Next);
    
    if (equal(Tok, "-"))
        return newUnary(ND_NEG, unary(Rest, Tok->Next));

    return primary(Rest, Tok);
}

static Node *mul(Token **Rest, Token *Tok)
{
    // primary
    Node *Nd = unary(&Tok, Tok);

    while (true)
    {
        //("*" | "/" primary)
        if (equal(Tok, "*"))
        {
            Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next));
            continue;
        }

        if (equal(Tok, "/"))
        {
            Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

static Node *add(Token **Rest, Token *Tok)
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

static Node *relational(Token **Rest, Token *Tok)
{
    Node *Nd = add(&Tok, Tok);

    while (true)
    {
        if (equal(Tok, "<"))
        {
            Nd = newBinary(ND_LT, Nd, add(&Tok, Tok->Next));
            continue;
        }

        if (equal(Tok, "<="))
        {
            Nd = newBinary(ND_LE, Nd, add(&Tok, Tok->Next));
            continue;
        }

        //X > Y 等价于 Y < X
        if (equal(Tok, ">"))
        {
            Nd = newBinary(ND_LT, add(&Tok, Tok->Next), Nd);
            continue;
        }

        if (equal(Tok, ">="))
        {
            Nd = newBinary(ND_LE, add(&Tok, Tok->Next), Nd);
            continue;
        }

        *Rest = Tok;
        return Nd;
    }

}

static Node *equality(Token **Rest, Token *Tok)
{
    Node *Nd = relational(&Tok, Tok);

    while (true)
    {
        if (equal(Tok, "=="))
        {
            Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok));
            continue;
        }

        if (equal(Tok, "!="))
        {
            Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

static Node *expr(Token **Rest, Token *Tok)
{
    return equality(Rest, Tok);
}