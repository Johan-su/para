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
    EXPR_NUMBER,  
    EXPR_ALT,
    EXPR_BIN_OPERATOR,
    EXPR_UNARY_OPERATOR,
    EXPR_EMPTY,
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


enum class BinOperatorType
{
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    EXPONENT,
};

struct BinOperator
{
    Expr expr;
    BinOperatorType opt;
};


enum class UnaryOperatorType
{
    PLUS,
    MINUS,
};

struct UnaryOperator
{
    Expr expr;
    UnaryOperatorType uot;
};


static const char *expr_to_str(Expr *e)
{
    //TODO(Johan): clean up num buffer
    Usize num_buffer_len = 32;
    char *num_buffer = alloc<char>(num_buffer_len);
    memset(num_buffer, 0, num_buffer_len);
    switch (e->expr_type)
    { 
        case EXPR_NUMBER:
        {
            snprintf(num_buffer, num_buffer_len, "%lld", ((Number *)e)->val);
            return num_buffer;
        } break;
        case EXPR_BIN_OPERATOR: return ((BinOperator *)e)->opt == BinOperatorType::PLUS ? "+" : ((BinOperator *)e)->opt == BinOperatorType::MINUS ? "-" : ((BinOperator *)e)->opt == BinOperatorType::MULTIPLY ? "*" : ((BinOperator *)e)->opt == BinOperatorType::DIVIDE ? "/" : ""; 

        
        case EXPR_UNARY_OPERATOR: return ((UnaryOperator *)e)->uot == UnaryOperatorType::PLUS ? "+" : ((UnaryOperator *)e)->uot == UnaryOperatorType::MINUS ? "-" : "";

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


namespace TopDownParser
{

static Expr *parse_operator(Lexer *lexer, Usize *index)
{
    BinOperator *boperator = alloc<BinOperator>(1);
    boperator->expr.expr_type = EXPR_BIN_OPERATOR;


    if (is_token(peek(lexer, *index), OPERATOR_PLUS)) boperator->opt = BinOperatorType::PLUS;
    else if (is_token(peek(lexer, *index), OPERATOR_MINUS)) boperator->opt = BinOperatorType::MINUS;
    else if (is_token(peek(lexer, *index), OPERATOR_MULTIPLY)) boperator->opt = BinOperatorType::MULTIPLY;
    else if (is_token(peek(lexer, *index), OPERATOR_DIVIDE)) boperator->opt = BinOperatorType::DIVIDE;
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
            assert(alt_expr->expr_type == EXPR_BIN_OPERATOR);
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

} // namespace TopDownParser


namespace TopDownParser2
{


enum ParseState
{
    ERROR = 0,
    START,
    EXPR,
    EXPR_ALT,
    UNARY,
    BINARY_OPERATOR,
    NUMBER,
};


enum OperatorLevel
{
    SENTINEL = 0,
    OPEN_PARENTHESIS,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    UNARY_PLUS,
    UNARY_MINUS,
    EXPONENT,
    CLOSE_PARENTHESIS = 100000,
};


#define STACK_SIZE 256




struct NumOrExpr 
{
    bool is_num;
    union 
    {
        I64 n;
        Expr *e;
    };
};



struct NumberStack
{
    NumOrExpr stack[STACK_SIZE];
    Usize index;
};

struct OperatorStack
{
    OperatorLevel stack[STACK_SIZE];
    Usize index;
};


static NumberStack n_stack = {
    .stack = {},
    .index = STACK_SIZE,
};

static OperatorStack o_stack = {
    .stack = {},
    .index = STACK_SIZE,
};

static Usize g_token_index = 0;

static void next_token(Lexer *lexer)
{
    if (g_token_index < lexer->count)
    {
        g_token_index += 1;
    }
}

static Token *peek(Lexer *lexer)
{
    return &lexer->tokens[g_token_index]; 
}

static Token *peek_before(Lexer *lexer)
{
    if (g_token_index <= 0)
    {
        return nullptr;
    }
    return &lexer->tokens[g_token_index - 1];
}

//TODO(Johan): https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm


static OperatorLevel read_stack(OperatorStack *st)
{
    if (st->index >= STACK_SIZE)
    {
        return OperatorLevel::SENTINEL;
    }

    return st->stack[st->index];
}


static void make_binoperator_expr(NumberStack *nst, OperatorStack *ost, BinOperatorType b_type)
{
    BinOperator *b_op = alloc<BinOperator>(1);
    b_op->opt = b_type;
    b_op->expr.expr_type = EXPR_BIN_OPERATOR;


    if (nst->stack[nst->index].is_num == true)
    {
        Number *num_right = alloc<Number>(1);
        memset(num_right, 0, sizeof(*num_right));
        num_right->expr.expr_type = EXPR_NUMBER;
        num_right->val = nst->stack[nst->index].n;
        b_op->expr.right = (Expr *)num_right;
    }
    else
    {
        b_op->expr.right = nst->stack[nst->index].e;
    }
    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));


    if (nst->stack[nst->index].is_num == true)
    {
        Number *num_left = alloc<Number>(1);
        memset(num_left, 0, sizeof(*num_left));
        num_left->expr.expr_type = EXPR_NUMBER;
        num_left->val = nst->stack[nst->index].n;
        b_op->expr.left = (Expr *)num_left;
    }
    else
    {
        b_op->expr.left = nst->stack[nst->index].e;
    }
    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));

    nst->stack[--nst->index] = NumOrExpr {.is_num = false, .e = (Expr *)b_op};

    ost->stack[ost->index++] = TopDownParser2::SENTINEL;
}


static void make_unoperator_expr(NumberStack *nst, OperatorStack *ost, UnaryOperatorType u_type)
{
    UnaryOperator *u_op = alloc<UnaryOperator>(1);
    memset(u_op, 0, sizeof(*u_op));

    u_op->uot = u_type;
    u_op->expr.expr_type = EXPR_UNARY_OPERATOR;

    if (nst->stack[nst->index].is_num == true)
    {
        Number *num = alloc<Number>(1);
        memset(num, 0, sizeof(*num));
        num->expr.expr_type = EXPR_NUMBER;
        num->val = nst->stack[nst->index].n;
        u_op->expr.right = (Expr *)num;
    }
    else
    {
        u_op->expr.right = nst->stack[nst->index].e;
    }

    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));
    nst->stack[--nst->index] = NumOrExpr {.is_num = false, .e = (Expr *)u_op};
    ost->stack[ost->index++] = TopDownParser2::SENTINEL;

}

static void make_tree(NumberStack *nst, OperatorStack *ost)
{
    switch (read_stack(ost))
    {
        case SENTINEL:
        {
            assert(false);
        } break;
        case PLUS:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::PLUS);
        } break;
        case MINUS:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::MINUS);
        } break;
        case MULTIPLY:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::MULTIPLY);
        } break;
        case DIVIDE:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::DIVIDE);
        } break;
        case UNARY_PLUS:
        {
            make_unoperator_expr(nst, ost, UnaryOperatorType::PLUS);
        } break;
        case UNARY_MINUS:
        {
            make_unoperator_expr(nst, ost, UnaryOperatorType::MINUS);
        } break;
        case EXPONENT:
        {
            TODO("implement");
        } break;
        case OPEN_PARENTHESIS:
        {
            ost->stack[ost->index++] = TopDownParser2::SENTINEL;
        } break;
        case CLOSE_PARENTHESIS:
        {
            TODO("implement");
        } break;
        default: assert(false && "unreachable");
    }
}



static Expr *parse_expr(Lexer *lexer)
{
    while (true)
    {
        if (peek(lexer)->token_type == TokenType::END_TOKEN)
        {
            while (n_stack.index < STACK_SIZE && o_stack.index != STACK_SIZE)
            {
                make_tree(&n_stack, &o_stack);
            }
            break;
        }
        else if (peek(lexer)->token_type == TokenType::OPEN_PARENTHESIS)
        {
            o_stack.stack[--o_stack.index] = OperatorLevel::OPEN_PARENTHESIS;
            next_token(lexer);
        }
        else if (peek(lexer)->token_type == TokenType::CLOSE_PARENTHESIS)
        {
            while (read_stack(&o_stack) != OperatorLevel::OPEN_PARENTHESIS)
            {
                make_tree(&n_stack, &o_stack);
            }
            next_token(lexer);
        }
        else if (peek(lexer)->token_type == TokenType::NUMBER)
        {
            NumOrExpr nor = {
                .is_num = true,
                .n = atoll(peek(lexer)->str_val),
            };
            n_stack.stack[--n_stack.index] = nor;
            next_token(lexer);
        }
        else if (peek(lexer)->token_type == OPERATOR_PLUS)
        {
            if (peek_before(lexer) == nullptr ||
                (peek_before(lexer)->token_type != TokenType::NUMBER && peek_before(lexer)->token_type != TokenType::CLOSE_PARENTHESIS))
            {
                if (read_stack(&o_stack) <= UNARY_PLUS)
                {
                    o_stack.stack[--o_stack.index] = UNARY_PLUS;
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
            else
            {
                if (read_stack(&o_stack) < PLUS)
                {
                    o_stack.stack[--o_stack.index] = PLUS;
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
        }
        else if (peek(lexer)->token_type == OPERATOR_MINUS)
        {
            if (peek_before(lexer) == nullptr ||
                (peek_before(lexer)->token_type != TokenType::NUMBER && peek_before(lexer)->token_type != TokenType::CLOSE_PARENTHESIS))
            {
                if (read_stack(&o_stack) <= UNARY_MINUS)
                {
                    o_stack.stack[--o_stack.index] = UNARY_MINUS;
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
            else
            {
                if (read_stack(&o_stack) < MINUS)
                {
                    o_stack.stack[--o_stack.index] = MINUS;
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
        }
        else if (peek(lexer)->token_type == OPERATOR_MULTIPLY)
        {
            if (read_stack(&o_stack) < MULTIPLY)
            {
                o_stack.stack[--o_stack.index] = MULTIPLY;
                next_token(lexer);
            }
            else
            {
                make_tree(&n_stack, &o_stack);
            }
        }
        else if (peek(lexer)->token_type == OPERATOR_DIVIDE)
        {
            if (read_stack(&o_stack) < DIVIDE)
            {
                o_stack.stack[--o_stack.index] = DIVIDE;
                next_token(lexer);
            }
            else
            {
                make_tree(&n_stack, &o_stack);
            }
        }
        else
        {
            assert(false);
        }
    }
    return n_stack.stack[n_stack.index].e;
}
    
} // namespace TopDownParser2



static void print_expr(Expr *expr)
{
    switch (expr->expr_type)
    {
        case EXPR_NUMBER:
        {
            Number *num = (Number *)expr;
            printf("%lld", num->val);
        } break;
        case EXPR_BIN_OPERATOR:
        {
            BinOperator *op = (BinOperator *)expr;
            printf("(");
            print_expr(op->expr.left);
            printf(" %s ", expr_to_str(expr));
            print_expr(op->expr.right);
            printf(")");
        } break;
        case EXPR_UNARY_OPERATOR:
        {
            UnaryOperator *op = (UnaryOperator *)expr;

            printf("(");
            printf("%s", expr_to_str(expr));
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
        case EXPR_NUMBER:
        {
            return ((Number *)e)->val;
        } break;   
        case EXPR_BIN_OPERATOR:
        {
            BinOperator *b_op = (BinOperator *)e;
            switch (b_op->opt)
            {
                case BinOperatorType::PLUS:
                {
                    return eval_expr(b_op->expr.left) + eval_expr(b_op->expr.right);
                } break;
                case BinOperatorType::MINUS:
                {
                    return eval_expr(b_op->expr.left) - eval_expr(b_op->expr.right);
                } break;
                case BinOperatorType::MULTIPLY:
                {
                    return eval_expr(b_op->expr.left) * eval_expr(b_op->expr.right);
                } break;
                case BinOperatorType::DIVIDE:
                {
                    return eval_expr(b_op->expr.left) / eval_expr(b_op->expr.right);
                } break;
                default: assert(false);
            }
        } break; 
        case EXPR_UNARY_OPERATOR:
        {
            UnaryOperator *u_op = (UnaryOperator *)e;
            switch (u_op->uot)
            {
                case UnaryOperatorType::PLUS:
                {
                    return eval_expr(u_op->expr.right);
                } break;
                case UnaryOperatorType::MINUS:
                {
                    return -eval_expr(u_op->expr.right);
                } break;

                default: assert(false);
            }
        } break;
    
        default: assert(false);
    }
}


static void create_image_from_exprtree(Expr *tree)
{
    {
        FILE *f = fopen("input.dot", "w");
        if (f == nullptr)
        {
            TODO("handle failed to open file");
        }

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


namespace BottomUpParser
{



/*

    1:
    <S>     := . <Expr> $
    <Expr>  := . (<Expr>) <Expr'>
    <Expr>  := . <Number> <Expr'>
    <Expr'> := . <BinOperator> <Expr> <Expr'>
    <Expr'> := . <>


*/






// https://boxbase.org/entries/2019/oct/14/lr1-parsing-tables/
// https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf

#define PARSE_STACK_CAP 4096

struct ExprOrToken
{
    bool is_token;

    union
    {
        Expr *e;
        Token *t;
    };
};  


struct ParseStack
{
    ExprOrToken data[PARSE_STACK_CAP];
    Usize index;
};





static ExprOrToken pop(ParseStack *s)
{
    if (s->index >= PARSE_STACK_CAP)
    {
        TODO("handle error tried to pop from empty stack");
    }
    return s->data[s->index++];
}

static void push(ParseStack *s, ExprOrToken t)
{
    if (s->index <= 0)
    {
        TODO("handle error push to full stack");
    }
    s->data[--s->index] = t;
}



static Usize g_token_index = 0;


static void next_token(Lexer *lexer)
{
    if (g_token_index < lexer->count)
    {
        g_token_index += 1;
    }
}

static Token *peek(Lexer *lexer)
{
    return &lexer->tokens[g_token_index];
}



//static action[][]


static void shift(ParseStack *stack, Lexer *lexer, U64 k)
{
    Token *t = peek(lexer);
    next_token(lexer);
    ExprOrToken eot = {
        .is_token = true,
        .t = t,
    };
    push(stack, eot);
}


static void reduce()
{

}


static Expr *parse_expr(Lexer *lexer)
{
    ParseStack g_stack = {
        .data = {},
        .index = PARSE_STACK_CAP,
    };

    return nullptr;
}















} // namespace BottomUpParser



static void usage(const char *program)
{

    printf("USAGE: %s <expr>\n example %s (5 + 5) * 5\n only works with + - * ( )\n WILL CRASH AT ANY ERROR", program, program);
    exit(1);
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
    }

    // const char *source = " ( 55+ 5) * (4 +4)";
    // const char *source = "5 + 5 * 5 - 5";
    // const char *source = argv[1];
    const char *source;
    {
        Usize total_strlen = 0;
        {
            for (Usize i = 1; argv[i] != nullptr; ++i)
            {
                total_strlen += strlen(argv[i]);
            }
        }
        char *source_from_args = alloc<char>(total_strlen + 1);
        {
            Usize copy_index = 0;
            for (Usize i = 1; argv[i] != nullptr; ++i)
            {
                Usize str_len = strlen(argv[i]);
                memcpy(&source_from_args[copy_index], argv[i], str_len);
                copy_index += str_len;
            }
            source_from_args[copy_index] = '\0';
        }
        source = source_from_args;
    }


    tokenize(&g_lexer, source);

    print_tokens(&g_lexer);

    Usize index = 0;
    Expr *expression_tree = TopDownParser2::parse_expr(&g_lexer);

    create_image_from_exprtree(expression_tree);
    printf("-------------\n");

    print_expr(expression_tree);
    printf(" = %lld\n", eval_expr(expression_tree));



    return 0;
}