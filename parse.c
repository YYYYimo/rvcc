#include "rvcc.h"


Obj *Locals;

static Obj *findVar(Token *Tok);
// program = "{" compounudStmt 
// compoundStmt = stmt* "}"
// stmt = "return" expr ";" | exprStmt
// exprStmt = expr? ";"
// expr = assign
// assign = equality ("=" assign recursion) 
// equality = relational ("==" relational | "!=" relational)
// relational = add ("<" add | "<=" add | " >" add | ">=" add)
// add = mul ("+" mul | "-" mul)*
// mul = primary ("*" primary | "/" primary)
// unary = ("+" | "-") unary | primary
// primary = "( expr )" | num | ident
static Node *compoundStmt(Token **Rest, Token *Tok);
static Node *stmt(Token **Rest, Token *Tok);
static Node *exprStmt(Token **Rest, Token *Tok);
static Node *assign(Token **Rest, Token *Tok);
static Node *expr(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Token);

// 新建语法树的节点
static Node *newNode(NodeKind Kind);
static Node *newUnary(NodeKind Kind, Node *Expr);
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS);
static Node *newNum(int64_t Val, Token *Tok);
static Obj* newLVar(char *Name);
static Node *newVarNode(Obj *Var);

char *Strndup(const char *s, size_t n)
{
    if (!s)
    {
        error("wrong name\n");
        return NULL;
    }

    char *new_str = (char *)calloc(n + 1, sizeof(char));
    if (new_str == NULL) 
    {
        error("memory allocation failed\n");
        return NULL;
    }
    strncpy(new_str, s, n);
    new_str[n] = '\0';
    return new_str;
}

static Node *newNode(NodeKind Kind)
{
    Node *Nd = (Node *)calloc(1, sizeof(Node));
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

static Obj *newLVar(char *Name)
{
    Obj *Var = (Obj *)calloc(1, sizeof(Obj));
    if (!Var)
    {
        error("Memory allocation failed\n");
        exit(1);
    }
    Var->Name = Name;
    Var->Next = Locals;
    Locals = Var;
    return Var;
}

static Node *newVarNode(Obj *Var)
{
    Node *Nd = newNode(ND_VAR);
    Nd->Var = Var;
    return Nd;
}

static Obj *findVar(Token *Tok)
{
    for (Obj *Var = Locals; Var; Var = Var->Next)
    {
        if (strlen(Var->Name) == Tok->Len && 
            strncmp(Tok->Loc, Var->Name, Tok->Len) == 0)
            return Var;
    }
    return NULL;
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

    //ident
    if (Tok->Kind == TK_IDENT)
    {
        Obj *Var = findVar(Tok);
        if (!Var)
            Var = newLVar(Strndup(Tok->Loc, Tok->Len));
        *Rest = Tok->Next;
        return newVarNode(Var);
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
            Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok->Next));
            continue;
        }

        if (equal(Tok, "!="))
        {
            Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok->Next));
            continue;
        }

        *Rest = Tok;
        return Nd;
    }
}

static Node *assign(Token **Rest, Token *Tok)
{
    Node *Nd = equality(&Tok, Tok);

    if (equal(Tok, "="))
        Nd = newBinary(ND_ASSIGN, Nd, assign(&Tok, Tok->Next));
    *Rest = Tok;
    return Nd;
}

static Node *expr(Token **Rest, Token *Tok)
{
    return assign(Rest, Tok);
}


static Node *exprStmt(Token **Rest, Token *Tok)
{
    if (equal(Tok, ";"))
    {
        *Rest = Tok->Next;
        return newNode(ND_BLOCK);
    }
    Node *Nd = newUnary(ND_EXPR_STMT, expr(&Tok, Tok));
    *Rest = skip(Tok, ";");
    return Nd;

}

static Node *stmt(Token **Rest, Token *Tok)
{
    if (equal(Tok, "return"))
    {
        Node *Nd = newUnary(ND_RET, expr(&Tok, Tok->Next));
        *Rest = skip(Tok, ";");
        return Nd;
    }

    if (equal(Tok, "{"))
    {
        return compoundStmt(Rest, Tok->Next);
    }

    return exprStmt(Rest, Tok);
}

static Node *compoundStmt(Token **Rest, Token *Tok)
{
    Node Head = {};
    Node *Cur = &Head;

    while (!equal(Tok, "}"))
    {
        Cur->Next = stmt(&Tok, Tok);
        Cur = Cur->Next;
    }

    Node *Nd = newNode(ND_BLOCK);
    Nd->Body = Head.Next;
    *Rest = Tok->Next;
    return Nd;
}

Function *parse(Token *Tok)
{
    Tok = skip(Tok, "{");
    
    // Body存储AST，Locals存储变量
    Function *Prog = (Function *)calloc(1, sizeof(Function));
    Prog->Body = compoundStmt(&Tok, Tok);
    Prog->Locals = Locals;
    return Prog;
}