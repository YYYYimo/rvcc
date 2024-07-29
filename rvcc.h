#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum 
{
    TK_PUNCT,
    TK_NUM,
    TK_EOF
} TokenKind;

typedef struct Token Token;
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
    ND_EXPR_STMT
} NodeKind;

//AST中二叉树节点
typedef struct Node Node;
struct Node
{
    NodeKind Kind;
    Node *LHS;
    Node *RHS;
    Node *Next; //指代下一语句
    int Val;
};

//语法分析与代码生成
Node *parse(Token *Tok);

//代码生成入口函数
void codegen(Node *Nd);