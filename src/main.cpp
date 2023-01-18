#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "helper.hpp"


#define LEXER_HPP_IMPLEMENTATION
#include "lexer.hpp"




// #include "shunting_yard_parser.cpp"

#include "LALR.cpp"










int main(int argc, const char *argv[])
{
    run_parser(argc, argv);



    return 0;
}