#include "rvcc.h"


Obj *Locals;

static Obj *findVar(Token *Tok);
// program = "{" compounudStmt 
// compoundStmt = (declaration | stmt)* "}"
// declaration = declspec (declarator "=" expr)? ("," declarator "=" expr)? ";"
// declspec = "int"
// declarator = "*"* ident
// stmt = "return" expr ";" 
//          | exprStmt
//          | "if" "(" expr ")" stmt ("else" stmt)?
//          | "for" "(" exprStmt expr? ";" expr? ")" stmt
//          | "while" "(" expr ")" stmt
//          | "{" compoundStmt
// exprStmt = expr? ";"
// expr = assign
// assign = equality ("=" assign recursion) 
// equality = relational ("==" relational | "!=" relational)
// relational = add ("<" add | "<=" add | " >" add | ">=" add)
// add = mul ("+" mul | "-" mul)*
// mul = primary ("*" primary | "/" primary)
// unary = ("+" | "-" | "*" | "&") unary | primary
// primary = "( expr )" | num | ident
static Node *compoundStmt(Token **Rest, Token *Tok);
static Node *declaration(Token **Rest, Token *Tok);
static Type *declspec(Token **Rest,  Token *Tok);
static Type *declarator(Token **Rest, Token *Tok, Type *Ty);
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
static Node *newNode(NodeKind Kind, Token *Tok);
static Node *newUnary(NodeKind Kind, Node *Expr, Token *Tok);
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS, Token *Tok);
static Node *newNum(int64_t Val, Token *Tok);
static Obj* newLVar(char *Name, Type *Ty);
static Node *newVarNode(Obj *Var, Token *Tok);
static Node *newAdd(Node *LHS, Node *RHS, Token *Tok);
static Node *newSub(Node *LHS, Node *RHS, Token *Tok);

static char *getIdent(Token *Tok)
{
    if (Tok->Kind != TK_IDENT)
        errorTok(Tok, "expected an identifier");
    return Strndup(Tok->Loc, Tok->Len);
}

char *Strndup(const char *s, size_t n)
{
    if (!s)
    {
        error("wrong name");
        return NULL;
    }

    char *new_str = (char *)calloc(n + 1, sizeof(char));
    if (new_str == NULL) 
    {
        error("memory allocation failed");
        return NULL;
    }
    strncpy(new_str, s, n);
    new_str[n] = '\0';
    return new_str;
}

static Node *newNode(NodeKind Kind, Token *Tok)
{
    Node *Nd = (Node *)calloc(1, sizeof(Node));
    Nd->Kind = Kind;
    Nd->Tok = Tok;
    return Nd;
}

// 新建一个单叉树
static Node *newUnary(NodeKind Kind, Node *Expr, Token *Tok)
{
    Node *Nd = newNode(Kind, Tok);
    Nd->LHS = Expr;
    return Nd;
}

// 新建一个二叉树节点
static Node *newBinary(NodeKind Kind, Node *LHS, Node *RHS, Token *Tok)
{
    Node *Nd = newNode(Kind, Tok);
    Nd->LHS = LHS;
    Nd->RHS = RHS;
    return Nd;
}

// 新建一个数字节点
static Node *newNum(int64_t Val, Token *Tok)
{
    Node *Nd = newNode(ND_NUM, Tok);
    Nd->Val = Val;
    return Nd;
}

static Obj *newLVar(char *Name, Type *Ty)
{
    Obj *Var = (Obj *)calloc(1, sizeof(Obj));
    if (!Var)
    {
        error("Memory allocation failed");
        exit(1);
    }
    Var->Name = Name;
    Var->Next = Locals;
    Var->Ty = Ty;
    Locals = Var;
    return Var;
}

static Node *newVarNode(Obj *Var, Token *Tok)
{
    Node *Nd = newNode(ND_VAR, Tok);
    Nd->Var = Var;
    return Nd;
}

static Node *newAdd(Node *LHS, Node *RHS, Token *Tok)
{
    addType(LHS);
    addType(RHS);

    //num + num
    if (isInteger(LHS->Ty) && isInteger(RHS->Ty))
        return newBinary(ND_ADD, LHS, RHS, Tok);
    
    // ptr + ptr 不能解析
    if (LHS->Ty->Base && RHS->Ty->Base)
        errorTok(Tok, "invalid operands");
    
    // num + ptr 转化为ptr + num
    if (!LHS->Ty->Base && RHS->Ty->Base)
    {
        Node *tmp = LHS;
        LHS = RHS;
        RHS = tmp;
    }

    // ptr + num
    RHS = newBinary(ND_MUL, RHS, newNum(8, Tok), Tok);
    return newBinary(ND_ADD, LHS, RHS, Tok);

}

static Node *newSub(Node *LHS, Node *RHS, Token *Tok)
{
    addType(LHS);
    addType(RHS);

    // num - num
    if (isInteger(LHS->Ty) && isInteger(RHS->Ty))
        return newBinary(ND_SUB, LHS, RHS, Tok);
    
    // ptr - num
    if (LHS->Ty->Base && isInteger(RHS->Ty))
    {
        RHS = newBinary(ND_MUL, RHS, newNum(8, Tok), Tok);
        addType(RHS);
        Node *Nd = newBinary(ND_SUB, LHS, RHS, Tok);
        Nd->Ty = LHS->Ty;
        return Nd;
    }

    // ptr - ptr
    if (LHS->Ty->Base && RHS->Ty->Base)
    {
        Node *Nd = newBinary(ND_SUB, LHS, RHS, Tok);
        Nd->Ty = TyInt;
        return newBinary(ND_DIV, Nd, newNum(8, Tok), Tok);
    }

    errorTok(Tok, "invalid operand");
    return NULL;
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
            errorTok(Tok, "undefined variable");
        *Rest = Tok->Next;
        return newVarNode(Var, Tok);
    }
}

// unary = ("+" | "-" | "*" | "&") unary | primary
static Node *unary(Token **Rest, Token *Tok)
{
    if (equal(Tok, "+"))
        return unary(Rest, Tok->Next);
    
    if (equal(Tok, "-"))
        return newUnary(ND_NEG, unary(Rest, Tok->Next), Tok);
    
    if (equal(Tok, "&"))
        return newUnary(ND_ADDR, unary(Rest, Tok->Next), Tok);

    if (equal(Tok, "*"))
        return newUnary(ND_DEREF, unary(Rest, Tok->Next), Tok);

    return primary(Rest, Tok);
}

static Node *mul(Token **Rest, Token *Tok)
{
    // primary
    Node *Nd = unary(&Tok, Tok);

    while (true)
    {
        Token *Start = Tok;
        //("*" | "/" primary)
        if (equal(Tok, "*"))
        {
            Nd = newBinary(ND_MUL, Nd, unary(&Tok, Tok->Next), Start);
            continue;
        }

        if (equal(Tok, "/"))
        {
            Nd = newBinary(ND_DIV, Nd, unary(&Tok, Tok->Next), Start);
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
        Token *Start = Tok;
        if (equal(Tok, "+"))
        {
            Nd = newAdd(Nd, mul(&Tok, Tok->Next), Start);
            continue;
        }

        if (equal(Tok, "-"))
        {
            Nd = newSub(Nd, mul(&Tok, Tok->Next), Start);
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
        Token *Start = Tok;

        if (equal(Tok, "<"))
        {
            Nd = newBinary(ND_LT, Nd, add(&Tok, Tok->Next), Start);
            continue;
        }

        if (equal(Tok, "<="))
        {
            Nd = newBinary(ND_LE, Nd, add(&Tok, Tok->Next), Start);
            continue;
        }

        //X > Y 等价于 Y < X
        if (equal(Tok, ">"))
        {
            Nd = newBinary(ND_LT, add(&Tok, Tok->Next), Nd, Start);
            continue;
        }

        if (equal(Tok, ">="))
        {
            Nd = newBinary(ND_LE, add(&Tok, Tok->Next), Nd, Start);
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
        Token *Start = Tok;
        if (equal(Tok, "=="))
        {
            Nd = newBinary(ND_EQ, Nd, relational(&Tok, Tok->Next), Tok);
            continue;
        }

        if (equal(Tok, "!="))
        {
            Nd = newBinary(ND_NE, Nd, relational(&Tok, Tok->Next), Tok);
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
        Nd = newBinary(ND_ASSIGN, Nd, assign(&Tok, Tok->Next), Tok);
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
        return newNode(ND_BLOCK, Tok);
    }
    Node *Nd = newNode(ND_EXPR_STMT, Tok);
    Nd->LHS = expr(&Tok, Tok);
    *Rest = skip(Tok, ";");
    return Nd;
}


// stmt = "return" expr ";" 
//          | "if" "(" expr ")" stmt ("else" stmt)?
//          | "for" "(" exprStmt expr? ";" expr? ")" stmt
//          | "while" "(" expr ")" stmt
//          | "{" compoundStmt
//          | exprStmt
static Node *stmt(Token **Rest, Token *Tok)
{
    if (equal(Tok, "return"))
    {
        Node *Nd = newUnary(ND_RET, expr(&Tok, Tok->Next), Tok);
        *Rest = skip(Tok, ";");
        return Nd;
    }

    if (equal(Tok, "if"))
    {
        Node *Nd = newNode(ND_IF, Tok);

        Tok = skip(Tok->Next, "(");
        Nd->Cond = expr(&Tok, Tok);
        Tok = skip(Tok, ")");

        Nd->Then = stmt(&Tok, Tok);
        if (equal(Tok, "else"))
            Nd->Els = stmt(&Tok, Tok->Next);
        
        *Rest = Tok;
        return Nd;
    }

    if (equal(Tok, "for"))
    {
        Node *Nd = newNode(ND_FOR, Tok);
        //"("
        Tok = skip(Tok->Next, "(");
        //exprStmt
        Nd->Init = exprStmt(&Tok, Tok);
        //expr ? ";"
        if (!equal(Tok, ";"))
            Nd->Cond = expr(&Tok, Tok);
        Tok = skip(Tok, ";");
        // expr ? ")"
        if (!equal(Tok, ")"))
            Nd->Inc = expr(&Tok, Tok);
        Tok = skip(Tok, ")");

        //stmt
        Nd->Then = stmt(&Tok, Tok);
        *Rest = Tok;
        return Nd;
    }

    if (equal(Tok, "while"))
    {
        Node *Nd = newNode(ND_FOR, Tok);
        //"("
        Tok = skip(Tok->Next, "(");
        // expr
        Nd->Cond = expr(&Tok, Tok);
        // ")"
        Tok = skip(Tok, ")");
        // stmt
        Nd->Then = stmt(&Tok, Tok);

        *Rest = Tok;
        return Nd;
    }

    if (equal(Tok, "{"))
    {
        return compoundStmt(Rest, Tok->Next);
    }


    return exprStmt(Rest, Tok);
}

// declarator = "*"* ident
static Type *declarator(Token **Rest, Token *Tok, Type *Ty)
{
    while(consume(&Tok, Tok, "*"))
        Ty = pointerTo(Ty);

    if (Tok->Kind != TK_IDENT)
        errorTok(Tok, "expected a variable name");
    
    Ty->Name = Tok;
    *Rest = Tok->Next;
    return Ty;
}

// declspec = "int"
static Type *declspec(Token **Rest,  Token *Tok)
{
    *Rest = skip(Tok, "int");
    return TyInt;
}

// declaration = declspec (declarator "=" expr)? ("," declarator "=" expr)? ";"
static Node *declaration(Token **Rest, Token *Tok)
{
    //declspec（ basic type)
    Type *BaseTy = declspec(&Tok, Tok);

    Node Head = {};
    Node *Cur = &Head;

    // 对变量声明次数计数
    int I = 0;
    while (!equal(Tok, ";"))
    {
        if (I++ > 0)
            Tok = skip(Tok, ",");
        
        // declarator
        Type *Ty = declarator(&Tok, Tok, BaseTy);
        Obj *Var = newLVar(getIdent(Ty->Name), Ty);

        if (!equal(Tok, "="))
            continue;
        
        Node *LHS = newVarNode(Var, Ty->Name);
        Node *RHS = assign(&Tok, Tok->Next);
        Node *Node = newBinary(ND_ASSIGN, LHS, RHS, Tok);
        
        Cur->Next = newUnary(ND_EXPR_STMT, Node, Tok);
        Cur = Cur->Next;
    }

    Node *Nd = newNode(ND_BLOCK, Tok);
    Nd->Body = Head.Next;
    *Rest = Tok->Next;
    return Nd;

}

// compoundStmt = (declaration | stmt)* "}"
static Node *compoundStmt(Token **Rest, Token *Tok)
{
    Node Head = {};
    Node *Cur = &Head;
    Node *Nd = newNode(ND_BLOCK, Tok);

    while (!equal(Tok, "}"))
    {
        if (equal(Tok, "int"))
            Cur->Next = declaration(&Tok, Tok);
        else
            Cur->Next = stmt(&Tok, Tok);
        Cur = Cur->Next;
        addType(Cur);
    }


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

