#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "helper.hpp"


#define LEXER_HPP_IMPLEMENTATION
#include "lexer.hpp"




#include "shunting_yard_parser.cpp"









// <S> := <Expr>
// <Expr> := (<Expr>) <Expr'>
// <Expr> := <Number> <Expr'>
// <Expr> := <U><Expr>
// <Expr'> := <BinOperator> <Expr> <Expr'>
// <Expr'> := <>



// <S> := <Expr>
// <Expr> := (<Expr>) <Expr'>
// <Expr> := <Number> <Expr'>
// <Expr'> := <BinOperator> <Expr> <Expr'>
// <Expr'> := <>







int main(int argc, const char *argv[])
{
    run_parser(argc, argv);



    return 0;
}