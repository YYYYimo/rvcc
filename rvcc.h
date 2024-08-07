//ʹ��POSIX.1 ��׼
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
    TK_IDENT, //��Ƿ�����������������
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

//������
void error(char *Fmt, ...);
void verrorAt(char *Loc, char *Fmt, va_list);
void errorAt(char *Loc, char *Fmt, ...);
void errorTok(Token *Tok, char *Fmt, ...);

bool equal(Token *Tok, char *Str);
Token *skip(Token *Tok, char *Str);
//�ʷ�����
Token *tokenize(char *Input);

//�����﷨������
//AST�ڵ�����
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
    ND_ASSIGN, //��ֵ�ڵ�
    ND_VAR, //�����ڵ�
    ND_RET,
    ND_BLOCK,
    ND_IF,
    ND_FOR,
    ND_ADDR, // ȡ��ַ &
    ND_DEREF //������ *
} NodeKind;

//AST�ж������ڵ�
struct Node
{
    NodeKind Kind;
    Node *LHS;
    Node *RHS;
    Node *Next; //ָ����һ���
    Token *Tok; //�ڵ��Ӧ���ս��
    Type *Ty; //�ڵ�洢����������

    //ND_VAR
    Obj *Var; 
    int Val;
    Node *Body; //{}

    //ND_IF
    Node *Cond; // �����ڵ����
    Node *Then; // ��������������
    Node *Els; //���������������

    //ND_FOR
    Node *Init;
    Node *Inc;
};

//�﷨�������������
Function *parse(Token *Tok);

//����������ں���
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

// �ж��Ƿ�Ϊ����
bool isInteger(Type *TY);
// Ϊ�ڵ��ڵ����нڵ��������
void addType(Node *Nd);