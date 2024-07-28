#include "rvcc.h"

static char *CurrentInput;

//（Head）-> 1 -> + -> 2 -> - ->3

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
        error("expected '%s", Str);
    return Tok->Next;
}

Token *newToken(TokenKind Kind, char *Start, char *End)
{
    Token *Tok = calloc(1, sizeof(Token));
    Tok->Kind = Kind;
    Tok->Loc = Start;
    Tok->Len = End - Start;
    return Tok;
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
        if (ispunct(*P))
        {
            Cur->Next = newToken(TK_PUNCT, P, P + 1);
            Cur = Cur->Next;
            ++P;
            continue;
        }

        errorAt(P, "invaild token");

        Cur->Next = newToken(TK_EOF, P, P + 1);

        return Head.Next;
    }
}