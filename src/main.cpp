#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef size_t Usize;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;


static_assert(sizeof(U8) == 1);
static_assert(sizeof(U16) == 2);
static_assert(sizeof(U32) == 4);
static_assert(sizeof(U64) == 8);

static_assert(sizeof(I8) == 1);
static_assert(sizeof(I16) == 2);
static_assert(sizeof(I32) == 4);
static_assert(sizeof(I64) == 8);

#define DEBUG_ASSERTS

#ifdef DEBUG_ASSERTS
#define assert(expression)                                                                                  \
    do                                                                                                      \
    {                                                                                                       \
        if (!(expression))                                                                                  \
        {                                                                                                   \
            fprintf(stderr, "ERROR: assertion failed %s, at %s:%d in \n", #expression, __FILE__, __LINE__); \
            exit(1);                                                                                        \
        }                                                                                                   \
    } while (0)
#else
#define assert(expression)
#endif

#define TODO(message) \
do \
{ \
    fprintf(stderr, "ERROR: TODO %s, at %s:%d in \n", message, __FILE__, __LINE__); \
    exit(1); \
} while (0);




enum TokenType
{
    INVALID_TOKEN = 0,
    OPERATOR_PLUS,    
    OPERATOR_MINUS,    
    OPERATOR_MULTIPLY,    
    OPERATOR_DIVIDE,

    PARENTHESIS_RIGHT,
    PARENTHESIS_LEFT,

    NUMBER,


    TOKEN_COUNT
};
/*
static const char *operators[] = {
    "+",
    "-",
    "*",
    "/",
}; 

static const char *delimiters[] = {
    "(",
    ")",
};
*/


struct Token
{
    const char *str_val;
    Usize str_count;
    TokenType token_type;
};


#define MAX_TOKEN_CAP 65535


struct Lexer
{
    Token tokens[MAX_TOKEN_CAP];
    Usize count;
};

static Lexer g_lexer = {
    .tokens = {},
    .count = 0,
};


static void push_token(Lexer *lexer, Token token)
{
    if (lexer->count >= MAX_TOKEN_CAP)
    {
        TODO("handle ERROR more tokens than token cap");
    }

    lexer->tokens[lexer->count] = token;
    lexer->count += 1;
}



static bool is_digit(char n)
{
    if (n >= '0' && n <= '9')
    {
        return true;
    }
    return false;
}

static bool is_whitespace(char n)
{
    switch (n)
    {
        case ' ': return true;
    }
    return false;
}


static void tokenize(Lexer *lexer, const char *source)
{
    for (Usize i = 0; source[i] != '\0'; ++i)
    {
             if (source[i] == '(') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = PARENTHESIS_RIGHT});
        else if (source[i] == ')') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = PARENTHESIS_LEFT});
        else if (source[i] == '+') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = OPERATOR_PLUS});
        else if (source[i] == '-') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = OPERATOR_MINUS});
        else if (source[i] == '*') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = OPERATOR_MULTIPLY});
        else if (source[i] == '/') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = OPERATOR_DIVIDE});
        else if (is_digit(source[i])) // TODO(Johan): add handling for decimal numbers
        {
            Token number_token = {};
            {
                number_token.str_val = &source[i];
                number_token.token_type = NUMBER;

                Usize count = 1;
                while (is_digit(source[i + count]))
                {
                    count += 1;
                }
                number_token.str_count = count;
                i += count - 1;
            }
            push_token(lexer, number_token);
        }
        else if (is_whitespace(source[i])) /*do nothing*/;
        else
        {
            fprintf(stderr, "ERROR: unhandled token \"%c\"\n", source[i]);
            assert(false && "unhandled token");
        }
    }
}



static const char *tokentype_to_str(TokenType tt)
{
    switch (tt)
    {
        case INVALID_TOKEN: return "INVALID_TOKEN";
        case OPERATOR_PLUS: return "OPERATOR_PLUS";    
        case OPERATOR_MINUS: return "OPERATOR_MINUS";    
        case OPERATOR_MULTIPLY: return "OPERATOR_MULTIPLY";    
        case OPERATOR_DIVIDE: return "OPERATOR_DIVIDE";
        case PARENTHESIS_RIGHT: return "PARENTHESIS_RIGHT";
        case PARENTHESIS_LEFT: return "PARENTHESIS_LEFT";
        case NUMBER: return "NUMBER";
        case TOKEN_COUNT: return "TOKEN_COUNT"; 
    }
    
    assert(false && "no string representation of token type found");
    return nullptr;
}



static void print_tokens(Lexer *lexer)
{
    for (Usize i = 0; i < lexer->count; ++i)
    {
        Token *token = &lexer->tokens[i];
        printf("%s VALUE: %.*s\n", tokentype_to_str(token->token_type), (int)token->str_count, token->str_val);
    }
}


template<typename T>
static T *alloc(Usize count)
{
    return (T *)malloc(sizeof(T) * count);
}



enum ExprType
{
    EXPR = 0,
    EXPR_NUMBER,  
    EXPR_OPERATOR,
};

struct Expr
{
    Expr *left;
    Expr *right;
    Expr *parent;
    ExprType expr_type;
};


struct Number
{
    Expr expr;
    I64 val;
};


enum OperatorType
{
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
};

struct Operator
{
    Expr expr;
    OperatorType opt;
};


static void parse_tokens(Lexer *lexer, Expr *root)
{
    Usize unmatched_par = 0;
    for (Usize i = 0; i < lexer->count; ++i)
    {
        Expr *current_tree = root;
        switch (lexer->tokens[i].token_type)
        {
            case INVALID_TOKEN:
            {
                assert(false);
            } break;
            case OPERATOR_PLUS:
            {
                //TODO(Johan): was here
                if (current_tree->expr_type == EXPR_NUMBER)
                {

                }
            } break;
            case OPERATOR_MINUS:
            {
                TODO("implement");
            } break;
            case OPERATOR_MULTIPLY:
            {
                TODO("implement");
            } break;
            case OPERATOR_DIVIDE:
            {
                TODO("implement");
            } break;
            case PARENTHESIS_RIGHT:
            {
                unmatched_par += 1;
                current_tree->left = alloc<Expr>(1);
                memset(current_tree->left, 0, sizeof(Expr));
                current_tree->left->parent = current_tree;
                current_tree = current_tree->left;
            } break;
            case PARENTHESIS_LEFT:
            {
                TODO("implement");
            } break;
            case NUMBER:
            {
                Number *number = alloc<Number>(1);
                number->val = atoll(lexer->tokens[i].str_val);
                if (current_tree->expr_type == EXPR_OPERATOR)
                {
                    current_tree->right = (Expr *)number;
                    current_tree->right->parent = current_tree;
                    current_tree = current_tree->parent;
                }
                else
                {
                    current_tree->left = (Expr *)number;
                    current_tree->left->parent = current_tree;
                }
            } break;
            case TOKEN_COUNT:
            {
                assert(false);
            } break;
            default:
            {
                fprintf(stderr, "ERROR: unknown token\n");
                assert(false && "unknown token");
            } break;
        } 
    }
}



int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        TODO("display usage when no argument is provided");
    }

    const char *source = " ( 55+ 5) * (4 +4)";

    tokenize(&g_lexer, source);

    print_tokens(&g_lexer);

    Expr expression_tree = {};

    parse_tokens(&g_lexer, &expression_tree);


    return 0;
}