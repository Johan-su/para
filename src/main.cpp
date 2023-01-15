#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "helper.hpp"



struct Expr;



struct MapEntry
{
    bool active;
    U64 key;
    Expr *e;
};

struct HashMap
{
    MapEntry data[4096];
};


static HashMap g_map = {
    .data = {},
};


static U64 hash_string(const char *str, Usize len)
{
    Usize p = 53;
    Usize hash = (Usize)str[0];
    for (Usize i = 1; i < len; ++i)
    {
        Usize term = (Usize)str[i];        
        for (Usize j = i; j != 0; --j)
        {
            term *= j;
        }
        hash += term;
    }
    return hash;
}


static void hashmap_add(HashMap *map, const char *name, Usize name_len, Expr *e)
{
    U64 hash = hash_string(name, name_len);

    MapEntry me = {
        .active = true,
        .key = hash,
        .e = e,
    };

    Usize pos = hash % ARRAY_SIZE(map->data);

    for (Usize i = 0; i < ARRAY_SIZE(map->data); ++i)
    {
        if (!map->data[pos].active || map->data[pos].key == hash)
        {
            map->data[pos] = me; 
            return;
        }
        else
        {
            pos = (hash % ARRAY_SIZE(map->data) + (i + 1)) % ARRAY_SIZE(map->data);
        }
    }
    assert(false);
}


static Expr *hashmap_get(HashMap *map, const char *name, Usize name_len)
{
    U64 hash = hash_string(name, name_len);

    Usize pos = hash % ARRAY_SIZE(map->data);
    for (Usize i = 0; i < ARRAY_SIZE(map->data); ++i)
    {
        if (map->data[pos].active && map->data[pos].key == hash)
        { 
            return map->data[pos].e;
        }
        else
        {
            pos = (hash % ARRAY_SIZE(map->data) + (i + 1)) % ARRAY_SIZE(map->data);
        }
    }
    return nullptr;
}




enum class TokenType
{
    INVALID_TOKEN = 0,
    OPERATOR_PLUS,    
    OPERATOR_MINUS,    
    OPERATOR_MULTIPLY,    
    OPERATOR_DIVIDE,

    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,

    NUMBER,

    IDENTIFIER,
    EQUAL,

    END_TOKEN,

    TOKEN_COUNT
};


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
        case '\n': return true;
    }
    return false;
}

static bool is_letter(char n)
{
    return (n >= 'A' && n <= 'Z') || (n >= 'a' && n <= 'z');
}


static void tokenize(Lexer *lexer, const char *source)
{
    for (Usize i = 0; source[i] != '\0'; ++i)
    {
             if (source[i] == '(') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPEN_PARENTHESIS});
        else if (source[i] == ')') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::CLOSE_PARENTHESIS});
        else if (source[i] == '+') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_PLUS});
        else if (source[i] == '-') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_MINUS});
        else if (source[i] == '*') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_MULTIPLY});
        else if (source[i] == '/') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_DIVIDE});
        else if (source[i] == '=') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::EQUAL});
        else if (is_digit(source[i]))
        {
            Token number_token = {};
            {
                number_token.str_val = &source[i];
                number_token.token_type = TokenType::NUMBER;

                Usize count = 1;
                while (is_digit(source[i + count]))
                {
                    count += 1;
                }
                if (source[i + count] == '.')
                {
                    count += 1;
                    while (is_digit(source[i + count]))
                    {
                        count += 1;
                    }
                }
                number_token.str_count = count;
                i += count - 1;
            }
            push_token(lexer, number_token);
        }
        else if (is_letter(source[i]))
        {
            Token id_token = {};
            id_token.str_val = &source[i];
            id_token.token_type = TokenType::IDENTIFIER;

            Usize count = 1;
            while (is_letter(source[i + count]))
            {
                count += 1;
            }
            id_token.str_count = count;
            i += count - 1;

            push_token(lexer, id_token);
        }
        else if (is_whitespace(source[i])) /*do nothing*/;
        else
        {
            fprintf(stderr, "ERROR: unhandled character %c\n", source[i]);
            assert(false && "unhandled character");
        }
    }
    push_token(lexer, Token {.str_val = nullptr, .str_count = 0, .token_type = TokenType::END_TOKEN});
}



static const char *tokentype_to_str(TokenType tt)
{
    switch (tt)
    {
        case TokenType::INVALID_TOKEN: return "INVALID_TOKEN";
        case TokenType::OPERATOR_PLUS: return "OPERATOR_PLUS";    
        case TokenType::OPERATOR_MINUS: return "OPERATOR_MINUS";    
        case TokenType::OPERATOR_MULTIPLY: return "OPERATOR_MULTIPLY";    
        case TokenType::OPERATOR_DIVIDE: return "OPERATOR_DIVIDE";
        case TokenType::OPEN_PARENTHESIS: return "OPEN_PARENTHESIS";
        case TokenType::CLOSE_PARENTHESIS: return "CLOSE_PARENTHESIS";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::END_TOKEN: return "END_TOKEN";
        case TokenType::TOKEN_COUNT: return "TOKEN_COUNT"; 
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



enum class ExprType
{
    INVALID = 0,
    NUMBER,  
    BIN_OPERATOR,
    UNARY_OPERATOR,
    INTERNAL_FUNCTION,
    FUNCTION,
    VARIABLE,
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
    F64 val;
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



enum class FunctionType
{
    INVALID,
    COS,
    SIN,
    TAN,
    SQUARE_ROOT,
    ABSOLUTE,
    LOG,
    E_EXPONENTIAL,
};

struct Function // TODO(Johan): change to INTERNAL_FUNCTION and USER_FUNCTION or similar
{
    Expr expr;
    FunctionType ft;
};




struct Variable
{
    Expr expr;
    Token *context_token;
};


static const char *expr_to_str(Expr *e)
{
    Usize char_buffer_len = 64;
    char *char_buffer = alloc<char>(char_buffer_len);
    memset(char_buffer, 0, char_buffer_len);
    switch (e->expr_type)
    { 
        case ExprType::NUMBER:
        {
            snprintf(char_buffer, char_buffer_len, "%g", ((Number *)e)->val);
            return char_buffer;
        } break;
        case ExprType::BIN_OPERATOR: return ((BinOperator *)e)->opt == BinOperatorType::PLUS ? "+" : ((BinOperator *)e)->opt == BinOperatorType::MINUS ? "-" : ((BinOperator *)e)->opt == BinOperatorType::MULTIPLY ? "*" : ((BinOperator *)e)->opt == BinOperatorType::DIVIDE ? "/" : nullptr; 
        case ExprType::UNARY_OPERATOR: return ((UnaryOperator *)e)->uot == UnaryOperatorType::PLUS ? "+" : ((UnaryOperator *)e)->uot == UnaryOperatorType::MINUS ? "-" : nullptr;
        case ExprType::FUNCTION:
        {
            switch (((Function *)e)->ft)
            {
                case FunctionType::INVALID: assert(false);
                case FunctionType::COS: return "cos";
                case FunctionType::SIN: return "sin";
                case FunctionType::TAN: return "tan";
                case FunctionType::SQUARE_ROOT: return "sqrt";
                case FunctionType::ABSOLUTE: return "abs";
                case FunctionType::LOG: return "log";
                case FunctionType::E_EXPONENTIAL: return "exp";
            
                default: assert(false);
            }
        } break;
        case ExprType::VARIABLE:
        {
            Variable *v = (Variable *)e;
            snprintf(char_buffer, char_buffer_len, "%.*s", (int)v->context_token->str_count, v->context_token->str_val);
            return char_buffer;
        } break;



        default: assert(false);
    }
    assert(false);
    return nullptr;
}


enum OperatorLevel
{
    SENTINEL = 0,
    EQUAL,
    OPEN_PARENTHESIS,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    UNARY_PLUS,
    UNARY_MINUS,
    EXPONENT,
    FUNCTION,
    CLOSE_PARENTHESIS = 100000,
};


struct Operator
{
    OperatorLevel ol;
    Token *context_token;
};

#define STACK_SIZE 256






struct ExprStack
{
    Expr *stack[STACK_SIZE];
    Usize index;
};

struct OperatorStack
{
    Operator stack[STACK_SIZE];
    Usize index;
};


static ExprStack n_stack = {
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


static Token *peek_amount(Lexer *lexer, I64 amount)
{
    if (g_token_index >= lexer->count)
    {
        return nullptr;
    }
    return &lexer->tokens[(I64)g_token_index + amount];
}


//TODO(Johan): https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm


static Operator read_stack(OperatorStack *st)
{
    if (st->index >= STACK_SIZE)
    {
        return Operator {};
    }

    return st->stack[st->index];
}


static void make_binoperator_expr(ExprStack *nst, OperatorStack *ost, BinOperatorType b_type)
{
    BinOperator *b_op = alloc<BinOperator>(1);
    memset(b_op, 0, sizeof(*b_op));
    b_op->opt = b_type;
    b_op->expr.expr_type = ExprType::BIN_OPERATOR;

    b_op->expr.right = nst->stack[nst->index];
    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));

    b_op->expr.left = nst->stack[nst->index];
    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));

    nst->stack[--nst->index] = (Expr *)b_op;

    ost->stack[ost->index++] = Operator {OperatorLevel::SENTINEL, nullptr};
}

static void make_equal_declaration(ExprStack *nst, OperatorStack *ost)
{
    Expr *right = nst->stack[nst->index];


    Variable *left_var = (Variable *)nst->stack[nst->index + 1];
    assert(left_var->expr.expr_type == ExprType::VARIABLE);
    
    hashmap_add(&g_map, left_var->context_token->str_val, left_var->context_token->str_count, right);

    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));


    ost->stack[ost->index++] = Operator {OperatorLevel::SENTINEL, nullptr};
}



static void make_unoperator_expr(ExprStack *nst, OperatorStack *ost, UnaryOperatorType u_type)
{
    UnaryOperator *u_op = alloc<UnaryOperator>(1);
    memset(u_op, 0, sizeof(*u_op));

    u_op->uot = u_type;
    u_op->expr.expr_type = ExprType::UNARY_OPERATOR;


    u_op->expr.right = nst->stack[nst->index];
    memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));

    nst->stack[--nst->index] = (Expr *)u_op;
    ost->stack[ost->index++] = Operator {OperatorLevel::SENTINEL, nullptr};

}





static double my_abs(double n)
{
    if (n < 0.0 || n == -0.0) return -n;
    return n;
}



static bool is_str(const char *str, Usize str_count, const char *str2)
{
    Usize str2_len = strlen(str2);

    if (str_count != str2_len)
    {
        return false;
    }
    
    for (Usize count = 0; count < str_count; ++count)
    {
        if (str[count] != str2[count])
        {
            return false;
        }
    }
    return true;
}

struct StrToFunc
{
    const char *str;
    FunctionType ft;
};

static StrToFunc function_list[] = {
    {"cos", FunctionType::COS},
    {"sin", FunctionType::SIN},
    {"tan", FunctionType::TAN},
    {"sqrt", FunctionType::SQUARE_ROOT},
    {"abs", FunctionType::ABSOLUTE},
    {"log", FunctionType::LOG},
    {"exp", FunctionType::E_EXPONENTIAL},
};

static FunctionType func_from_str(const char *str, Usize str_count)
{
    for (Usize i = 0; i < ARRAY_SIZE(function_list); ++i)
    {
        if (is_str(str, str_count, function_list[i].str))
        {
            return function_list[i].ft;
        }
    }
    return FunctionType::INVALID;
}



static void make_tree(ExprStack *nst, OperatorStack *ost)
{
    switch (read_stack(ost).ol)
    {
        case OperatorLevel::SENTINEL:
        {
            assert(false);
        } break;
        case OperatorLevel::EQUAL:
        {
            make_equal_declaration(nst, ost);
        } break;
        case OperatorLevel::OPEN_PARENTHESIS:
        {
            ost->stack[ost->index++] = Operator {OperatorLevel::SENTINEL, nullptr};
        } break;
        case OperatorLevel::PLUS:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::PLUS);
        } break;
        case OperatorLevel::MINUS:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::MINUS);
        } break;
        case OperatorLevel::MULTIPLY:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::MULTIPLY);
        } break;
        case OperatorLevel::DIVIDE:
        {
            make_binoperator_expr(nst, ost, BinOperatorType::DIVIDE);
        } break;
        case OperatorLevel::UNARY_PLUS:
        {
            make_unoperator_expr(nst, ost, UnaryOperatorType::PLUS);
        } break;
        case OperatorLevel::UNARY_MINUS:
        {
            make_unoperator_expr(nst, ost, UnaryOperatorType::MINUS);
        } break;
        case OperatorLevel::EXPONENT:
        {
            TODO("implement");
        } break;
        case OperatorLevel::FUNCTION:
        {
            Function *func = alloc<Function>(1);
            memset(func, 0, sizeof(*func));

            Token *context_token = read_stack(ost).context_token;
            
            assert(context_token != nullptr);

            func->ft = func_from_str(context_token->str_val, context_token->str_count);
            func->expr.expr_type = ExprType::FUNCTION;

            func->expr.right = nst->stack[nst->index];
            memset(&nst->stack[nst->index++], 0, sizeof(nst->stack[0]));

            nst->stack[--nst->index] = (Expr *)func;
            ost->stack[ost->index++] = Operator {OperatorLevel::SENTINEL, nullptr};

        } break;
        case OperatorLevel::CLOSE_PARENTHESIS:
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
        else if (peek(lexer)->token_type == TokenType::IDENTIFIER)
        {
            Token *id_token = peek(lexer);
            if (peek_amount(lexer, 1)->token_type == TokenType::OPEN_PARENTHESIS)
            {
                if (read_stack(&o_stack).ol <= OperatorLevel::FUNCTION)
                {
                    o_stack.stack[--o_stack.index] = Operator {OperatorLevel::FUNCTION, id_token};
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
            else
            {
                Variable *var = alloc<Variable>(1);
                memset(var, 0, sizeof(*var));
                
                var->expr.expr_type = ExprType::VARIABLE;
                var->context_token = id_token;

                n_stack.stack[--n_stack.index] = (Expr *)var;
                next_token(lexer);
            }
        }
        else if (peek(lexer)->token_type == TokenType::OPEN_PARENTHESIS)
        {
            o_stack.stack[--o_stack.index] = Operator {OperatorLevel::OPEN_PARENTHESIS, nullptr};
            next_token(lexer);
        }
        else if (peek(lexer)->token_type == TokenType::CLOSE_PARENTHESIS)
        {
            while (read_stack(&o_stack).ol != OperatorLevel::OPEN_PARENTHESIS)
            {
                make_tree(&n_stack, &o_stack);
            }
            memset(&o_stack.stack[o_stack.index++], 0, sizeof(o_stack.stack[0]));
            next_token(lexer);
        }
        else if (peek(lexer)->token_type == TokenType::NUMBER)
        {
            Number *num = alloc<Number>(1);
            memset(num, 0, sizeof(*num));
            
            num->expr.expr_type = ExprType::NUMBER;
            num->val = atof(peek(lexer)->str_val);

            n_stack.stack[--n_stack.index] = (Expr *)num;
            next_token(lexer);
        }
        else if(peek(lexer)->token_type == TokenType::EQUAL)
        {
            if (peek_amount(lexer, -1)->token_type != TokenType::IDENTIFIER)
            {
                fprintf(stderr, "Identifier has to be before equal sign\n");
                assert(false);
            }

            if (read_stack(&o_stack).ol <= OperatorLevel::EQUAL)
            {
                o_stack.stack[--o_stack.index] = Operator {OperatorLevel::EQUAL, nullptr};
                next_token(lexer);
            }
            else
            {
                make_tree(&n_stack, &o_stack);
            }

        }
        else if (peek(lexer)->token_type == TokenType::OPERATOR_PLUS)
        {
            if (peek_amount(lexer, -1) == nullptr ||
                (peek_amount(lexer, -1)->token_type != TokenType::NUMBER && peek_amount(lexer, -1)->token_type != TokenType::CLOSE_PARENTHESIS))
            {
                if (read_stack(&o_stack).ol <= UNARY_PLUS)
                {
                    o_stack.stack[--o_stack.index] = Operator {UNARY_PLUS, nullptr};
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
            else
            {
                if (read_stack(&o_stack).ol < PLUS)
                {
                    o_stack.stack[--o_stack.index] = Operator {PLUS, nullptr};
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
        }
        else if (peek(lexer)->token_type == TokenType::OPERATOR_MINUS)
        {
            if (peek_amount(lexer, -1) == nullptr ||
                (peek_amount(lexer, -1)->token_type != TokenType::NUMBER && peek_amount(lexer, -1)->token_type != TokenType::CLOSE_PARENTHESIS))
            {
                if (read_stack(&o_stack).ol <= UNARY_MINUS)
                {
                    o_stack.stack[--o_stack.index] = Operator {OperatorLevel::UNARY_MINUS, nullptr};
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
            else
            {
                if (read_stack(&o_stack).ol < MINUS)
                {
                    o_stack.stack[--o_stack.index] = Operator {OperatorLevel::MINUS, nullptr};
                    next_token(lexer);
                }
                else
                {
                    make_tree(&n_stack, &o_stack);
                }
            }
        }
        else if (peek(lexer)->token_type == TokenType::OPERATOR_MULTIPLY)
        {
            if (read_stack(&o_stack).ol < MULTIPLY)
            {
                o_stack.stack[--o_stack.index] = Operator {OperatorLevel::MULTIPLY, nullptr};
                next_token(lexer);
            }
            else
            {
                make_tree(&n_stack, &o_stack);
            }
        }
        else if (peek(lexer)->token_type == TokenType::OPERATOR_DIVIDE)
        {
            if (read_stack(&o_stack).ol < DIVIDE)
            {
                o_stack.stack[--o_stack.index] = Operator {OperatorLevel::DIVIDE, nullptr};
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

    return n_stack.stack[n_stack.index];
}



static void print_expr(Expr *expr)
{
    switch (expr->expr_type)
    {
        case ExprType::NUMBER:
        {
            Number *num = (Number *)expr;
            printf("%g", num->val);
        } break;
        case ExprType::BIN_OPERATOR:
        {
            BinOperator *op = (BinOperator *)expr;
            printf("(");
            print_expr(op->expr.left);
            printf(" %s ", expr_to_str(expr));
            print_expr(op->expr.right);
            printf(")");
        } break;
        case ExprType::UNARY_OPERATOR:
        {
            UnaryOperator *op = (UnaryOperator *)expr;

            printf("(");
            printf("%s", expr_to_str(expr));
            print_expr(op->expr.right);
            printf(")");

        } break;
        case ExprType::FUNCTION:
        {
            Function *op = (Function *)expr;

            printf("%s", expr_to_str(expr));
            printf("(");
            print_expr(op->expr.right);
            printf(")");
        } break;
        case ExprType::VARIABLE:
        {
            Variable *var = (Variable *)expr;
            printf("%.*s", (int)var->context_token->str_count, var->context_token->str_val);
        } break;

        default: assert(false);
    }
}

static F64 eval_expr(Expr *e)
{
    switch (e->expr_type)
    {
        case ExprType::NUMBER:
        {
            return ((Number *)e)->val;
        } break;   
        case ExprType::BIN_OPERATOR:
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
        case ExprType::UNARY_OPERATOR:
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
        case ExprType::FUNCTION:
        {
            Function *func = (Function *)e;
            switch (func->ft)
            {
                case FunctionType::INVALID:
                {
                    assert(false);
                } break;
                case FunctionType::COS:
                {
                    return cos(eval_expr(func->expr.right));
                } break;
                case FunctionType::SIN:
                {
                    return sin(eval_expr(func->expr.right));
                } break;
                case FunctionType::TAN:
                {
                    return tan(eval_expr(func->expr.right));
                } break;
                case FunctionType::SQUARE_ROOT:
                {
                    return sqrt(eval_expr(func->expr.right));
                } break;
                case FunctionType::ABSOLUTE:
                {
                    return my_abs(eval_expr(func->expr.right));
                } break;
                case FunctionType::LOG:
                {
                    return log(eval_expr(func->expr.right));
                } break;
                case FunctionType::E_EXPONENTIAL:
                {
                    return exp(eval_expr(func->expr.right));
                } break;
            
                default: assert(false);
            }
        }  break;
        case ExprType::VARIABLE:
        {
            Variable *var = (Variable *)e;
            Expr *expr = hashmap_get(&g_map, var->context_token->str_val, var->context_token->str_count);
            assert(expr != nullptr);
            return eval_expr(expr);
        } break;
        default: assert(false);
    }
    assert(false && "unreachable");
    return NAN;
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
            Usize stack_index = ARRAY_SIZE(stack);
            stack[--stack_index] = tree;
            while (stack_index < ARRAY_SIZE(stack))
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


static void usage(const char *program)
{

    printf("USAGE: %s (-g) <expr>\n"
    "-g starts interface, 'q' to quit in interface\n" 
    "define variables <name> = <expr>, example X = sqrt(5) then on the next line X * X will yield 5\n"
    "example %s (5 + 5) * 5 + sqrt(5 * 5)\n" 
    "operators + - * / ( )\n"
    "functions cos, sin tan, sqrt, abs, log, exp\n"
    "WILL CRASH AT ANY ERROR\n", program, program);
    exit(1);
}


static char command_buffer[2048] = {};


int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
    }

    if (!is_str(argv[1], 2, "-g"))
    {   
        const char *source = nullptr;
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

        Expr *expression_tree = parse_expr(&g_lexer);
        create_image_from_exprtree(expression_tree);
        printf("-------------\n");

        print_expr(expression_tree);
        printf(" = %g\n", eval_expr(expression_tree));
    }
    else
    {
        while (true)
        {
            fgets(command_buffer, ARRAY_SIZE(command_buffer), stdin);
            if (command_buffer[0] == 'q') break;

            const char *source = command_buffer;
            memset(&g_lexer, 0, sizeof(g_lexer));
            g_token_index = 0;
            tokenize(&g_lexer, source);
            print_tokens(&g_lexer);

            Expr *expr_tree = parse_expr(&g_lexer);
            print_expr(expr_tree);
            printf(" = %g\n", eval_expr(expr_tree));
        }
    }








    return 0;
}