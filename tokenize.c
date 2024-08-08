#include "rvcc.h"

static char *CurrentInput;

//��Head��-> 1 -> + -> 2 -> - ->3
Token *newToken(TokenKind Kind, char *Start, char *End);
static int readPunct(char *Ptr);
static bool startsWith(char *Str, char *SubStr);
static void convertKeywords(Token *Tok);
static bool isKeyword(Token *Tok);


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

// ���ز������ĳ���
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
        int PunctLen = readPunct(P);
        if (ispunct(*P))
        {
            Cur->Next = newToken(TK_PUNCT, P, P + PunctLen);
            Cur = Cur->Next;
            P += PunctLen;
            continue;
        }

        //������Ƿ�
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