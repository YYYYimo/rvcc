#include "rvcc.h"

static char *CurrentInput;

//��Head��-> 1 -> + -> 2 -> - ->3

void error(char *Fmt, ...) {
  // ����һ��va_list����
  va_list VA;
  // VA��ȡFmt��������в���
  va_start(VA, Fmt);
  // vfprintf�������va_list���͵Ĳ���
  vfprintf(stderr, Fmt, VA);
  // �ڽ�β����һ�����з�
  fprintf(stderr, "\n");
  // ���VA
  va_end(VA);
  // ��ֹ����
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

        //�ո��Ʊ�������
        if (isspace(*P))
        {
            ++P;
            continue;
        }

        //��������
        if (isdigit(*P))
        {
            Cur->Next = newToken(TK_NUM, P, P);
            Cur = Cur->Next;
        }
    }
}