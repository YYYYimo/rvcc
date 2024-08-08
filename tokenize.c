#include "rvcc.h"

static char *CurrentInput;

//（Head）-> 1 -> + -> 2 -> - ->3
Token *newToken(TokenKind Kind, char *Start, char *End);
static int readPunct(char *Ptr);
static bool startsWith(char *Str, char *SubStr);
static void convertKeywords(Token *Tok);
static bool isKeyword(Token *Tok);


void error(char *Fmt, ...) {
  // 定义一个va_list变量
  va_list VA;
  // VA获取Fmt后面的所有参数
  va_start(VA, Fmt);
  // vfprintf可以输出va_list类型的参数
  vfprintf(stderr, Fmt, VA);
  // 在结尾加上一个换行符
  fprintf(stderr, "\n");
  // 清除VA
  va_end(VA);
  // 终止程序
  exit(1);
}

void verrorAt(char *Loc, char *Fmt, va_list VA)
{
    //输出源信息
    fprintf(stderr, "%s\n", CurrentInput);

    //计算出错位置
    int Pos = Loc - CurrentInput;
    fprintf(stderr, "%*s", Pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, Fmt, VA);
    fprintf(stderr, "\n");
    va_end(VA);
    exit(1);
}

void errorAt(char *Loc, char *Fmt, ...)
{
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Loc, Fmt, VA);
}

void errorTok(Token *Tok, char *Fmt, ...)
{
    va_list VA;
    va_start(VA, Fmt);
    verrorAt(Tok->Loc, Fmt, VA);
}

bool equal(Token *Tok, char *Str)
{
    //判断 Str[Tok->Len] == '\0' 是为了确保 Str 的长度与 Tok->Len 完全匹配
    //并且 Str 在 Tok->Len 位置之后没有额外的字符
    return memcmp(Tok->Loc, Str, Tok->Len) == 0 && Str[Tok->Len] == '\0';
}

Token *skip(Token *Tok, char *Str)
{
    if (!equal(Tok, Str))
        error("expected '%s'", Str);
    return Tok->Next;
}

bool consume(Token **Rest, Token *Tok, char *Str)
{
    if (equal(Tok, Str))
    {
        *Rest = Tok->Next;
        return true;
    }

    *Rest = Tok;
    return false;
}

Token *newToken(TokenKind Kind, char *Start, char *End)
{
    Token *Tok = (Token *)calloc(1, sizeof(Token));
    Tok->Kind = Kind;
    Tok->Loc = Start;
    Tok->Len = End - Start;
    return Tok;
}

static bool startsWith(char *Str, char *SubStr)
{
    return strncmp(Str, SubStr, strlen(SubStr)) == 0;
}

// 返回操作符的长度
static int readPunct(char *Ptr)
{
    if (startsWith(Ptr, "==") || startsWith(Ptr, "!=") ||
        startsWith(Ptr, ">=") || startsWith(Ptr, "<="))
        return 2;
    
    return ispunct(*Ptr) ? 1 : 0;
}

static bool isIdent1(char C)
{
    return ('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z') || C == '_';
}

static bool isIdent2(char C)
{
    return isIdent1(C) || ('0' <= C && C <= '9');
}

static void convertKeywords(Token *Tok)
{
    for (Token *T = Tok; T->Kind != TK_EOF; T = T->Next)
    {
        if (isKeyword(T))
            T->Kind = TK_KEYWORD;
    }  
}

static bool isKeyword(Token *Tok)
{
    static char *KW[] = {"return", "if", "else", "for", "while", "int"};

    for (int i = 0; i < sizeof(KW) / sizeof(*KW); ++i)
    {
        if (equal(Tok, KW[i]))
            return true;
    }

    return false;
}

Token *tokenize(char *P)
{
    CurrentInput = P;

    Token Head = {};
    Token *Cur = &Head;

    while (*P) 
    {
        //空格、制表符的情况
        if (isspace(*P))
        {
            ++P;
            continue;
        }

        //解析数字
        if (isdigit(*P))
        {
            Cur->Next = newToken(TK_NUM, P, P);
            //指向下一个空节点
            Cur = Cur->Next;
            const char *OldPtr = P;
            Cur->Val = strtoul(P, &P, 10);
            Cur->Len = P - OldPtr;
            continue;
        }

        //解析操作符
        int PunctLen = readPunct(P);
        if (ispunct(*P))
        {
            Cur->Next = newToken(TK_PUNCT, P, P + PunctLen);
            Cur = Cur->Next;
            P += PunctLen;
            continue;
        }

        //解析标记符
        //[a-zA-Z_][a-zA-Z0-9_]*
        if (isIdent1(*P))
        {
            char *Start = P;
            do {
                ++P;
            } while (isIdent2(*P));
            Cur->Next = newToken(TK_IDENT, Start, P);
            Cur = Cur->Next;
            continue;
        }

        errorAt(P, "invaild token");
    }

    Cur->Next = newToken(TK_EOF, P, P + 1);

    convertKeywords(Head.Next);

    return Head.Next;
}