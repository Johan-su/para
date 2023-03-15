#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "helper.hpp"


#define LEXER_HPP_IMPLEMENTATION
#include "lexer.hpp"




// #include "shunting_yard_parser.cpp"











static Lexer g_lexer = {};

static const char *src = "f(x) = 5 * 5 + 4"; 

int main(void)
{
    tokenize(&g_lexer, src);
    print_tokens(&g_lexer);


    

    return 0;
}