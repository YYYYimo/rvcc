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
    CurrentInput = Input;

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
            Cur = Cur->Next;
        }
    }
}