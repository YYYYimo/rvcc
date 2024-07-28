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

void verrorAt(char *Loc, char *Fmt, va_list VA)
{
    //���Դ��Ϣ
    fprintf(stderr, "%s\n", CurrentInput);

    //�������λ��
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
    //�ж� Str[Tok->Len] == '\0' ��Ϊ��ȷ�� Str �ĳ����� Tok->Len ��ȫƥ��
    //���� Str �� Tok->Len λ��֮��û�ж�����ַ�
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
            //ָ����һ���սڵ�
            Cur = Cur->Next;
            const char *OldPtr = P;
            Cur->Val = strtoul(P, &P, 10);
            Cur->Len = P - OldPtr;
            continue;
        }

        //����������
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