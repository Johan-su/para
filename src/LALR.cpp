#include "helper.hpp"
#include "lexer.hpp"



















// <S> := <S'>
// <S'> := <FuncDecl>
// <S'> := <VarDecl>
// <S'> := <E>
//
// <FuncDecl> := <Id>(<Id>) = <E>
// <VarDecl> := <Id> = <E>
// <Id> = [a-zA-Z]+
//
// <E> := (<E>)
// <E> := <Number>
// <E> := <Var>
// <E> := <FuncCall>
//
// <E> := -<E>
// <E> := +<E>
// <E> := <E> / <E>
// <E> := <E> * <E>
// <E> := <E> - <E>
// <E> := <E> + <E>
// 
// <FuncCall> := <Id>(<E>)
// <Var> := <Id>
// <Number> := [0-9]+ || [0.9]+.[0-9]*



enum class ProdType
{
    FUNCTION_DECLARATION,
    VARIABLE_DECLARATION,
    EXPRESSION,
    IDENTIFIER,
    NUMBER,
    VARIABLE,
    FUNCTION_CALL,
};


struct Prod
{
    Prod *sub[2];
    ProdType pt;
};









static Lexer g_lexer = {
    .tokens = {},
    .count = 0,
};



static int run_parser(int argc, const char *argv[])
{
    (void)argc; (void)argv;
    const char *source = "1 + 1";

    tokenize(&g_lexer, source);
    print_tokens(&g_lexer);
    return 0;
}