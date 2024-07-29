#include "./rvcc.h"
    
int main(int Argc, char **Argv)
{
    if (Argc != 2)
    {
        error("%s: Invalid number of args", Argv[0]);
    }

    //解析Argv[1], 生成终结符流
    Token *Tok = tokenize(Argv[1]);
    
    //解析终结符流
    Node *Nd = parse(Tok);

    //生成代码
    codegen(Nd);
    return 0;
}