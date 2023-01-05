#define _CRT_SECURE_NO_WARNINGS
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


#define ARRAY_COUNT(array) sizeof(array) / sizeof(array[0])


enum TokenType
{
    INVALID_TOKEN = 0,
    OPERATOR_PLUS,    
    OPERATOR_MINUS,    
    OPERATOR_MULTIPLY,    
    OPERATOR_DIVIDE,

    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,

    NUMBER,

    END_TOKEN,

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
             if (source[i] == '(') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = OPEN_PARENTHESIS});
        else if (source[i] == ')') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = CLOSE_PARENTHESIS});
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
            fprintf(stderr, "ERROR: unhandled character \"%c\"\n", source[i]);
            assert(false && "unhandled character");
        }
    }
    push_token(lexer, Token {.str_val = nullptr, .str_count = 0, .token_type = END_TOKEN});
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
        case OPEN_PARENTHESIS: return "OPEN_PARENTHESIS";
        case CLOSE_PARENTHESIS: return "CLOSE_PARENTHESIS";
        case NUMBER: return "NUMBER";
        case END_TOKEN: return "END_TOKEN";
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
    ExprType expr_type;
};


struct Number
{
    Expr expr;
    I64 val;
};


enum BinOperatorType
{
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
};

struct BinOperator
{
    Expr expr;
    BinOperatorType opt;
};



static const char *expr_to_str(Expr *e)
{
    //TODO(Johan): clean up num buffer
    Usize num_buffer_len = 32;
    char *num_buffer = alloc<char>(num_buffer_len);
    memset(num_buffer, 0, num_buffer_len);
    switch (e->expr_type)
    { 
        case EXPR: return "EXPR";
        case EXPR_NUMBER:
        {
            snprintf(num_buffer, num_buffer_len, "%lld", ((Number *)e)->val);
            return num_buffer;
        } break;
        case EXPR_OPERATOR: return ((BinOperator *)e)->opt == PLUS ? "+" : ((BinOperator *)e)->opt == MINUS ? "-" : ((BinOperator *)e)->opt == MULTIPLY ? "*" : ((BinOperator *)e)->opt == DIVIDE ? "/" : ""; 
        default: assert(false);
    }
}

static Token *peek(Lexer *lexer, Usize amount)
{
    if (amount >= MAX_TOKEN_CAP)
    {
        TODO("handle error token outside cap");
        return nullptr;
    }
    return &lexer->tokens[amount];
}



static bool is_token(Token *token, TokenType tt)
{
    if (token->token_type == INVALID_TOKEN)
    {
        ;
    }
    assert(token->token_type != INVALID_TOKEN);
    return token->token_type == tt;
}



static bool is_operator_token(Token *token)
{
    return is_token(token, OPERATOR_PLUS) ||
    is_token(token, OPERATOR_MINUS) ||
    is_token(token, OPERATOR_MULTIPLY) ||
    is_token(token, OPERATOR_DIVIDE);
}




static Expr *parse_operator(Lexer *lexer, Usize *index)
{
    BinOperator *boperator = alloc<BinOperator>(1);
    boperator->expr.expr_type = EXPR_OPERATOR;


    if (is_token(peek(lexer, *index), OPERATOR_PLUS)) boperator->opt = PLUS;
    else if (is_token(peek(lexer, *index), OPERATOR_MINUS)) boperator->opt = MINUS;
    else if (is_token(peek(lexer, *index), OPERATOR_MULTIPLY)) boperator->opt = MULTIPLY;
    else if (is_token(peek(lexer, *index), OPERATOR_DIVIDE)) boperator->opt = DIVIDE;
    else
    {
        TODO("handle error expected operator but found none");
    }

    *index += 1;
    return (Expr *)boperator;
}



static Expr *parse_number(Lexer *lexer, Usize *index)
{
    Expr *root = nullptr;

    if (is_token(peek(lexer, *index), NUMBER))
    {
        if (is_token(peek(lexer, *index + 1), NUMBER))
        {
            TODO("unexpected number after number");
        }
        Number *number = alloc<Number>(1);
        number->val = atoll(lexer->tokens[*index].str_val);
        number->expr.expr_type = EXPR_NUMBER;
        root = (Expr *)number;
        *index += 1;
    }

    return root;
}

// <Expr> := <Expr> <BinOperator> <Expr> | (<Expr>) | <Number>


// https://www.tutorialspoint.com/what-is-left-recursion-and-how-it-is-eliminated

// not left recursion
// <Expr> := (<Expr>) <Expr'> | <Number> <Expr'>
// <Expr'> := {<BinOperator> <Expr> <Expr'>}*

static Expr *parse_expr(Lexer *lexer, Usize *index);
static Expr *parse_alt_expr(Lexer *lexer, Usize *index)
{
    Expr *root = nullptr;

    if (is_token(peek(lexer, *index), END_TOKEN))
    {
        return nullptr;
    }
    if (is_operator_token(peek(lexer, *index)))
    {
        Expr *op = parse_operator(lexer, index);
        Expr *expr = parse_expr(lexer, index);
        Expr *alt_expr = parse_alt_expr(lexer, index); 


        op->right = expr;

        if (alt_expr != nullptr)
        {
            assert(alt_expr->expr_type == EXPR_OPERATOR);
            alt_expr->left = op;
            root = alt_expr;
        }
        else
        {
            root = op;
        }
    }


    return root;
}

static Expr *parse_expr(Lexer *lexer, Usize *index)
{
    Expr *root = nullptr;

    assert(lexer->tokens[*index].token_type != INVALID_TOKEN);

    if (is_token(peek(lexer, *index), END_TOKEN))
    {
        return nullptr;
    }
    else if (is_token(peek(lexer, *index), OPEN_PARENTHESIS))
    {
        *index += 1;
        Expr *expr = parse_expr(lexer, index);
        if (!is_token(peek(lexer, *index), CLOSE_PARENTHESIS))
        {
            TODO("handle error right parens must match with left parens");
        }
        *index += 1;
        Expr *alt_expr = parse_alt_expr(lexer, index);
        if (alt_expr != nullptr)
        {
            alt_expr->left = expr;
            root = alt_expr;
        }
        else
        {
            root = expr;
        }
    }
    else if (is_token(peek(lexer, *index), NUMBER))
    {
        Expr *number = parse_number(lexer, index);

        Expr *alt_expr = parse_alt_expr(lexer, index);

        if (alt_expr != nullptr)
        {
            alt_expr->left = number;
            root = alt_expr;
        }
        else
        {
            root = number;
        }

    }
    else
    {
        assert(false && "unreachable");
    }

    return root;
}


static void print_expr(Expr *expr)
{
    switch (expr->expr_type)
    {
        case EXPR:
        {
            if (expr->left != nullptr) print_expr(expr->left);
            if (expr->right != nullptr) print_expr(expr->right);
        } break;
        case EXPR_NUMBER:
        {
            Number *num = (Number *)expr;
            printf("%lld", num->val);
        } break;
        case EXPR_OPERATOR:
        {
            BinOperator *op = (BinOperator *)expr;
            printf("(");
            print_expr(op->expr.left);
            printf(" %s ", op->opt == PLUS ? "+" : op->opt == MINUS ? "-" : op->opt == MULTIPLY ? "*" : op->opt == DIVIDE ? "/" : nullptr);
            print_expr(op->expr.right);
            printf(")");
        } break;
    
        default: assert(false);
    }
}

static I64 eval_expr(Expr *e)
{
    switch (e->expr_type)
    {
        case EXPR:
        {
            assert(false);
        } break; 
        case EXPR_NUMBER:
        {
            return ((Number *)e)->val;
        } break;   
        case EXPR_OPERATOR:
        {
            BinOperator *b_op = (BinOperator *)e;
            switch (b_op->opt)
            {
                case PLUS:
                {
                    return eval_expr(b_op->expr.left) + eval_expr(b_op->expr.right);
                } break;
                case MINUS:
                {
                    return eval_expr(b_op->expr.left) - eval_expr(b_op->expr.right);
                } break;
                case MULTIPLY:
                {
                    return eval_expr(b_op->expr.left) * eval_expr(b_op->expr.right);
                } break;
                case DIVIDE:
                {
                    return eval_expr(b_op->expr.left) / eval_expr(b_op->expr.right);
                } break;
            }
        } break; 
    
        default: assert(false);
    }
}


static void create_image_from_exprtree(Expr *tree)
{
    {
        FILE *f = fopen("input.dot", "w");

        fprintf(f, "digraph G {\n");

        {
            Expr *stack[512] = {0};
            Usize stack_index = ARRAY_COUNT(stack);
            stack[--stack_index] = tree;
            while (stack_index < ARRAY_COUNT(stack))
            {
                Expr *poped_tree = stack[stack_index++];
                assert(poped_tree != nullptr);
                fprintf(f, "n%llu [label=\"%s\"];\n", (Usize)poped_tree, expr_to_str(poped_tree));

                if (poped_tree->left != nullptr)
                {
                    stack[--stack_index] = poped_tree->left;
                    fprintf(f, "n%llu -> n%llu;\n", (Usize)poped_tree, (Usize)poped_tree->left);
                }
                if (poped_tree->right != nullptr)
                {
                    stack[--stack_index] = poped_tree->right;
                    fprintf(f, "n%llu -> n%llu;\n", (Usize)poped_tree, (Usize)poped_tree->right);
                }
            }
        }
        fprintf(f, "}");
        fclose(f);
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        TODO("display usage when no argument is provided");
    }

    // const char *source = " ( 55+ 5) * (4 +4)";
    //const char *source = "((1 + (1)) + (1) + 1)";
    const char *source = argv[1];

    tokenize(&g_lexer, source);

    print_tokens(&g_lexer);

    Usize index = 0;
    Expr *expression_tree = parse_expr(&g_lexer, &index);

    create_image_from_exprtree(expression_tree);
    printf("-------------\n");

    print_expr(expression_tree);
    printf(" = %lld", eval_expr(expression_tree));



    return 0;
}