//使用POSIX.1 标准
#define _POSIX_C_SOURCE 200808L

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct Node Node;
typedef struct Token Token;
typedef struct Obj Obj;
typedef struct Function Function;
typedef struct Type Type;

extern Type *TyInt;

char *Strndup(const char *s, size_t n);

struct Function
{
    Node *Body;
    Obj *Locals;
    int StackSize;
};

//local varible
struct Obj
{
    Obj *Next;
    char *Name;
    int Offset; //offset of fp
};


typedef enum 
{
    TK_IDENT, //标记符，变量名、函数名
    TK_KEYWORD,
    TK_PUNCT,
    TK_NUM,
    TK_EOF
} TokenKind;


struct Token
{
    TokenKind Kind;
    Token *Next;
    int Val;
    char *Loc;
    int Len;
};

//报错函数
void error(char *Fmt, ...);
void verrorAt(char *Loc, char *Fmt, va_list);
void errorAt(char *Loc, char *Fmt, ...);
void errorTok(Token *Tok, char *Fmt, ...);

bool equal(Token *Tok, char *Str);
Token *skip(Token *Tok, char *Str);
//词法分析
Token *tokenize(char *Input);

//抽象语法树部分
//AST节点类型
typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG,
    ND_NE,
    ND_EQ,
    ND_LT,
    ND_LE,
    ND_NUM,
    ND_EXPR_STMT,
    ND_ASSIGN, //赋值节点
    ND_VAR, //变量节点
    ND_RET,
    ND_BLOCK,
    ND_IF,
    ND_FOR,
    ND_ADDR, // 取地址 &
    ND_DEREF //解引用 *
} NodeKind;

//AST中二叉树节点
struct Node
{
    NodeKind Kind;
    Node *LHS;
    Node *RHS;
    Node *Next; //指代下一语句
    Token *Tok; //节点对应的终结符
    Type *Ty; //节点存储的数据类型

    //ND_VAR
    Obj *Var; 
    int Val;
    Node *Body; //{}

    //ND_IF
    Node *Cond; // 条件内的语句
    Node *Then; // 符合条件后的语句
    Node *Els; //不符合条件的语句

    //ND_FOR
    Node *Init;
    Node *Inc;
};

//语法分析与代码生成
Function *parse(Token *Tok);

//代码生成入口函数
void codegen(Function *Prog);


typedef enum 
{
    TY_INT,
    TY_PTR
} TypeKind;

struct Type
{
    TypeKind Kind;
    Type *Base;
};

// 判断是否为整型
bool isInteger(Type *TY);
// 为节点内的所有节点添加类型
void addType(Node *Nd);