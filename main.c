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
    Function *Prog = parse(Tok);

    //���ɴ���
    codegen(Prog);
    return 0;
}