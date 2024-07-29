#include "./rvcc.h"
    
int main(int Argc, char **Argv)
{
    if (Argc != 2)
    {
        error("%s: Invalid number of args", Argv[0]);
    }

    //����Argv[1], �����ս����
    Token *Tok = tokenize(Argv[1]);
    
    //�����ս����
    Node *Nd = parse(Tok);

    //���ɴ���
    codegen(Nd);
    return 0;
}