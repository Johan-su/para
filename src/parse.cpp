#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


typedef float f32;
typedef double f64;



#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

#define alloc(type, amount) (type*)(calloc((amount), sizeof(type)))

#define assert(condition)                                                                    \
do                                                                                                  \
{                                                                                                   \
    if (!(condition))                                                                               \
    {                                                                                               \
        fprintf(stderr, "ERROR: assertion failed [%s] at %s:%d\n", #condition, __FILE__, __LINE__); \
        exit(1);                                                                                    \
    }                                                                                               \
} while (0)

struct [[nodiscard]] Errcode
{
    int code;
    operator int() const { return code; }
    Errcode(int c): code(c) {}
};

// static char *file_to_str(const char *file_path)
// {
//     char *str = nullptr;
//     FILE *f = fopen(file_path, "rb");
//     if (f == nullptr)
//     {
//         fprintf(stderr, "ERROR: %s\n", strerror(errno));
//         return nullptr;
//     }

//     long file_size = -1;
//     if (fseek(f, 0, SEEK_END) != 0)
//     {
//         fprintf(stderr, "ERROR: failed to seek file %s\n", file_path);
//         goto end_close;
//     }
//     file_size = ftell(f);
//     if (file_size < 0)
//     {
//         goto end_close;
//     }
//     if (fseek(f, 0, SEEK_SET) != 0)
//     {
//         fprintf(stderr, "ERROR: failed to seek file %s\n", file_path);
//         goto end_close;
//     }
//     {
//         usize buf_size = (usize)file_size + 1; 
//         str = alloc(char, buf_size);
//     }
//     if (fread(str, sizeof(*str), (usize)file_size, f) != (usize)file_size)
//     {
//         fprintf(stderr, "ERROR: failed to read data from file %s\n", file_path);
//         free(str);
//         str = nullptr;
//     }

//     end_close:
//     if (fclose(f) == EOF)
//     {
//         fprintf(stderr, "ERROR: failed to close file %s\n", file_path);
//     }
//     return str;
// }


enum class Token_Kind
{
    INVALID,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    CARET,
    NUMBER,
    OPEN_PAREN,
    CLOSE_PAREN,
    COMMA,
    IDENTIFIER,
    END,
};





static const char *str_from_token_kind(Token_Kind kind)
{
    switch (kind)
    {
        case Token_Kind::INVALID: return "INVALID";
        case Token_Kind::PLUS: return "PLUS";
        case Token_Kind::MINUS: return "MINUS";
        case Token_Kind::STAR: return "STAR";
        case Token_Kind::SLASH: return "SLASH";
        case Token_Kind::NUMBER: return "NUMBER";
        case Token_Kind::CARET: return "CARET";
        case Token_Kind::OPEN_PAREN: return "OPEN_PAREN";
        case Token_Kind::CLOSE_PAREN: return "CLOSE_PAREN";
        case Token_Kind::COMMA: return "COMMA";
        case Token_Kind::IDENTIFIER: return "IDENTIFIER";
        case Token_Kind::END: return "END";
    }
    assert(false);
    return nullptr;
}

struct Token
{
    Token_Kind kind;
    u32 data_index;
    u32 count;
};


struct Lexer
{
    u32 count;
    char *data;
    u32 data_length;
    u32 index;
    Token *tokens;
};










static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}


static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_alphanumeric(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static void print_error_here(const char *data, u32 data_length, u32 index)
{
    assert(index < data_length);

    s64 start_index = (s64) index - 100 < 0 ? 0 : index - 100;
    for (u32 i = index; i != 0; --i)
    {
        if (data[i] == '\n')
        {
            start_index = i;
            break;
        }
    }
    s64 end_index = index + 100 > data_length ? data_length - 1 : index + 100;

    for (u32 i = index; i < data_length; ++i)
    {
        if (data[i] == '\n')
        {
            end_index = i;
            break;
        }
    }

    fprintf(stderr, "ERROR: here\n");

    s64 print_len = end_index - start_index + 1; 
    fprintf(stderr, "%.*s\n", (int) print_len, data + start_index);
    
    for (s64 i = 0; i < print_len; ++i)
    {
        if (i == index)
        {
            fputc('^', stderr);
        }
        fputc(' ', stderr);
    }
    fputc('\n', stderr);

}

static void print_error_here_token(const Lexer *lex, u32 token_index)
{
    print_error_here(lex->data, lex->data_length, lex->tokens[token_index].data_index);
}



static bool is_whitespace(char c)
{
    switch (c)
    {
        case ' ': return true;
        case '\n': return true;
        case '\t': return true;
        case '\r': return true;
    }
    return false;
}


static Errcode tokenize(Lexer *lex, char *input, u32 input_length)
{
    memset(lex, 0, sizeof(*lex));

    u64 token_capacity = 1 + 2 * input_length;
    lex->tokens = alloc(Token, token_capacity);

    lex->data = input;
    lex->data_length = input_length;

    for (u32 i = 0; i < input_length; )
    {
        switch (input[i])
        {
            case '+':
            {
                lex->tokens[lex->count++] = {Token_Kind::PLUS, i, 1};
                i += 1;
            } break;
            case '-':
            {
                lex->tokens[lex->count++] = {Token_Kind::MINUS, i, 1};
                i += 1;
            } break;
            case '*':
            {
                lex->tokens[lex->count++] = {Token_Kind::STAR, i, 1};
                i += 1;
            } break;
            case '/':
            {
                lex->tokens[lex->count++] = {Token_Kind::SLASH, i, 1};
                i += 1;
            } break;
            case '^':
            {
                lex->tokens[lex->count++] = {Token_Kind::CARET, i, 1};
                i += 1;
            } break;
            case '(':
            {
                lex->tokens[lex->count++] = {Token_Kind::OPEN_PAREN, i, 1};
                i += 1;
            } break;
            case ')':
            {
                lex->tokens[lex->count++] = {Token_Kind::CLOSE_PAREN, i, 1};
                i += 1;
            } break;
            case ',':
            {
                lex->tokens[lex->count++] = {Token_Kind::COMMA, i, 1};
                i += 1;
            } break;
            default:
            if (is_whitespace(input[i]))
            {
                i += 1;
            }
            else if (is_digit(input[i]))
            {
                Token token = {};
                token.kind = Token_Kind::NUMBER;
                token.data_index = i;

                while (is_digit(input[i]))
                {
                    token.count += 1;
                    i += 1;
                }
                if (is_alpha(input[i]))
                {
                    fprintf(stderr, "ERROR: invalid numeric literal\n");
                    print_error_here(lex->data, lex->data_length, i);
                    return 1;
                }
                lex->tokens[lex->count++] = token;
            }
            else if (is_alpha(input[i]))
            {
                Token token = {};
                token.kind = Token_Kind::IDENTIFIER;
                token.data_index = i;
                while (is_alphanumeric(input[i]))
                {
                    token.count += 1;
                    i += 1;
                }
                lex->tokens[lex->count++] = token;
            }
            else
            {
                fprintf(stderr, "ERROR: unrecognized input %c\n", input[i]);
                print_error_here(lex->data, lex->data_length, i);
                return 1;
            }
        }


    }
    
    lex->tokens[lex->count++] = {Token_Kind::END, 0, 0};

    assert(lex->count < token_capacity);

    return 0;
}


enum Node_Kind 
{
    INVALID,
    OPEN_PAREN,
    POSITIVE,
    NEGATE,
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    COMMA,
    FUNCTION,
    VARIABLE,
    NUMBER
};

struct Node
{
    Node_Kind kind;

    union
    {
        Node *left;
        Node *next;
    };
    Node *right;


    u32 token_index;
    f64 num;
};


struct Operator_Data
{
    s64 precendence;
    bool right_associative;
};


const Operator_Data g_data[] = {
    /* INVALID */  {0, false},
    /* OPEN_PAREN */ {0, false},
    /* POSITIVE */ {100, true},
    /* NEGATE */   {100, true},
    /* ADD */      {10, false},
    /* SUB */      {10, false},
    /* MUL */      {20, false},
    /* DIV */      {20, false},
    /* POW */      {30, true},
    /* COMMA */      {-9999, false},
    /* FUNCTION */ {110, false},
    /* VARIABLE */ {0, false},
    /* NUMBER */   {0, false},
};

static const char *str_from_node_kind(Node_Kind kind)
{
    switch (kind)
    {
        case Node_Kind::INVALID: return "INVALID";
        case Node_Kind::OPEN_PAREN: return "OPEN_PAREN";
        case Node_Kind::POSITIVE: return "POSITIVE";
        case Node_Kind::NEGATE: return "NEGATE";
        case Node_Kind::ADD: return "ADD";
        case Node_Kind::SUB: return "SUB";
        case Node_Kind::MUL: return "MUL";
        case Node_Kind::DIV: return "DIV";
        case Node_Kind::POW: return "POW";
        case Node_Kind::COMMA: return "COMMA";
        case Node_Kind::FUNCTION: return "FUNCTION";
        case Node_Kind::VARIABLE: return "VARIABLE";
        case Node_Kind::NUMBER: return "NUMBER";
    }
    assert(false);
    return nullptr;
}


static void graphviz_from_tree(Node *tree, Lexer *lex)
{
    FILE *f = fopen("./input.dot", "wb");


    fprintf(f, "graph G {\n");
    
    Node **stack = alloc(Node *, 2000);
    u32 stack_count = 0;


    stack[stack_count++] = tree;


    while (stack_count > 0)
    {
        Node *top = stack[--stack_count];
        
        fprintf(f, "n%llu [label=\"%s", (usize)top, str_from_node_kind(top->kind));
        {
            if (top->kind == Node_Kind::NUMBER)
            {
                fprintf(f, "\n%.2f", top->num);
            }
            else if (top->kind == Node_Kind::FUNCTION || top->kind == Node_Kind::VARIABLE)
            {
                Token t = lex->tokens[top->token_index];
                fprintf(f, "\n%.*s", t.count, lex->data + t.data_index);
            }
        }
        fprintf(f, "\"];\n");

        if (top->left != nullptr)
        {
            fprintf(f, "n%llu -- n%llu\n", (usize)top, (usize)top->left);

            stack[stack_count++] = top->left;
        }

        if (top->right != nullptr)
        {
            fprintf(f, "n%llu -- n%llu\n", (usize)top, (usize)top->right);
            stack[stack_count++] = top->right;
        }
    }

    fprintf(f, "}\n");
    fclose(f);
}


enum class Control_Flag : u8
{
    in_function = 1 << 0,
};


static Token next_token(Lexer *lex)
{
    Token t = lex->tokens[lex->index];
    if (t.kind != Token_Kind::END)
    {
        lex->index += 1;
    }
    return t;
}


static Node *parse_leaf(Lexer *lex)
{
    Token next = next_token(lex);


    if (next.kind == Token_Kind::NUMBER)
    {
        Node *node = alloc(Node, 1);
        node->kind = Node_Kind::NUMBER;
        return node;
    }

    fprintf(stderr, "ERROR: got %s expected NUMBER\n", str_from_token_kind(next.kind));
    return nullptr;
}



static void print_node_stack(Node **s, u32 size)
{
    for (u32 i = 0; i < size; ++i)
    {

        switch (s[i]->kind)
        {

            case Node_Kind::NUMBER:
            {
                printf("[%p], %s, %f\n", (void *)s[i], str_from_node_kind(s[i]->kind), s[i]->num);
            } break;

            default:
                printf("[%p], %s\n", (void *)s[i], str_from_node_kind(s[i]->kind));
        }
    }
}


static Node *pop_sub_node(Node **s, u32 *size)
{
    u32 index = *size - 1;
    Node *data = s[index];

    for (u32 i = index + 1; i < *size; ++i)
    {
        s[i - 1] = s[i];
    }
    *size -= 1;
    return data;
}



static void init_bin_node(Node *bin, Node **stack, u32 *stack_len)
{
    bin->right = pop_sub_node(stack, stack_len);
    bin->left = pop_sub_node(stack, stack_len);
}


static void init_unary_node(Node *bin, Node **stack, u32 *stack_len)
{
    bin->next = pop_sub_node(stack, stack_len);
}


static void init_function_node(Node *bin, Node **stack, u32 *stack_len)
{
    bin->next = pop_sub_node(stack, stack_len);
}


static bool is_binary(Node_Kind kind)
{
    switch (kind)
    {
        case Node_Kind::ADD: return true;
        case Node_Kind::SUB: return true;
        case Node_Kind::MUL: return true;
        case Node_Kind::DIV: return true;
        case Node_Kind::POW: return true;
        case Node_Kind::COMMA: return true;
        default: return false;
    }
}


static bool is_unary_operator(Token next, Token curr, Token last)
{
    if (last.kind == Token_Kind::NUMBER) return false;
    if (last.kind == Token_Kind::IDENTIFIER) return false;
    if (last.kind == Token_Kind::CLOSE_PAREN) return false;


    if (curr.kind == Token_Kind::STAR) return false;
    if (curr.kind == Token_Kind::SLASH) return false;
    if (curr.kind == Token_Kind::CARET) return false;
    if (curr.kind == Token_Kind::NUMBER) return false;
    if (curr.kind == Token_Kind::OPEN_PAREN) return false;
    if (curr.kind == Token_Kind::CLOSE_PAREN) return false;
    if (curr.kind == Token_Kind::COMMA) return false;
    if (curr.kind == Token_Kind::IDENTIFIER) return false;
    if (curr.kind == Token_Kind::END) return false;



    if (next.kind == Token_Kind::STAR) return false;
    if (next.kind == Token_Kind::SLASH) return false;
    if (next.kind == Token_Kind::CARET) return false;
    if (next.kind == Token_Kind::CLOSE_PAREN) return false;
    if (next.kind == Token_Kind::COMMA) return false;
    if (next.kind == Token_Kind::END) return false;

    return true;
}


static Node_Kind node_kind_from_token(Token next, Token curr, Token last)
{
    Node_Kind kind = Node_Kind::INVALID;



    if (curr.kind == Token_Kind::IDENTIFIER)
    {
        if (next.kind == Token_Kind::OPEN_PAREN) kind = Node_Kind::FUNCTION;
        else kind = Node_Kind::VARIABLE;
    }
    else if (curr.kind == Token_Kind::OPEN_PAREN)
    {
        kind = Node_Kind::OPEN_PAREN;
    }
    else if (is_unary_operator(next, curr, last))
    {
        if (curr.kind == Token_Kind::PLUS) kind = Node_Kind::POSITIVE;
        else if (curr.kind == Token_Kind::MINUS) kind = Node_Kind::NEGATE;
        else assert(false);
    }
    else /*if binary*/
    {
        if (curr.kind == Token_Kind::PLUS) kind = Node_Kind::ADD;
        else if (curr.kind == Token_Kind::MINUS) kind = Node_Kind::SUB;
        else if (curr.kind == Token_Kind::STAR) kind = Node_Kind::MUL;
        else if (curr.kind == Token_Kind::SLASH) kind = Node_Kind::DIV;
        else if (curr.kind == Token_Kind::CARET) kind = Node_Kind::POW;
        else if (curr.kind == Token_Kind::OPEN_PAREN) kind = Node_Kind::OPEN_PAREN;
        else if (curr.kind == Token_Kind::COMMA) kind = Node_Kind::COMMA;
        else assert(false);
    }
    return kind;
}

static Errcode make_node_from_output(Node *op, Node **output_stack, u32 *output_count, Control_Flag *flag_stack, u32 *flag_count, Lexer *lex)
{
    if (op->kind == Node_Kind::POSITIVE || op->kind == Node_Kind::NEGATE)
    {
        init_unary_node(op, output_stack, output_count);
        output_stack[(*output_count)++] = op;
    }
    else if (is_binary(op->kind))
    {
        if (op->kind == Node_Kind::COMMA)
        {
            if (*flag_count == 0 || flag_stack[*flag_count - 1] != Control_Flag::in_function)
            {
                fprintf(stderr, "ERROR: Commas can only be inside functions\n");
                print_error_here_token(lex, op->token_index);
                return 1;
            }
        }
        if (*output_count < 2)
        {
            fprintf(stderr, "ERROR: expected 2 args but got %u\n", *output_count);
            print_error_here_token(lex, op->token_index);
            return 1;
        }
        init_bin_node(op, output_stack, output_count);
        output_stack[(*output_count)++] = op;
    }
    else if (op->kind == Node_Kind::FUNCTION)
    {
        init_function_node(op, output_stack, output_count);
        output_stack[(*output_count)++] = op;
        // pop is_function
        *flag_count -= 1;
    }
    else
    {
        fprintf(stderr, "ERROR: unhandled operator %s\n", str_from_node_kind(op->kind));
        print_error_here_token(lex, op->token_index);
        return 1;
    }
    return 0;
}



// https://www.youtube.com/watch?v=fIPO4G42wYE
static Node *parse_arithmetic(Lexer *lex)
{
    Node **operator_stack = alloc(Node *, 32);
    u32 operator_count = 0;

    Node *output_stack[8] = {};
    u32 output_count = 0;


    Control_Flag flag_stack[8] = {};
    u32 flag_count = 0;


    Token last = {};
    Token curr = next_token(lex);
    Token next = next_token(lex);

    while (curr.kind != Token_Kind::END)
    {
        assert(curr.kind != Token_Kind::INVALID);

        switch (curr.kind)
        {
            case Token_Kind::PLUS:
            case Token_Kind::MINUS:
            case Token_Kind::STAR:
            case Token_Kind::SLASH:
            case Token_Kind::CARET:
            case Token_Kind::COMMA:
            {

                while (operator_count != 0)
                {
                    Node *op = operator_stack[operator_count - 1];
                    if (op->kind == Node_Kind::OPEN_PAREN)
                    {
                        break;
                    }

                    const Operator_Data *p_next = g_data + node_kind_from_token(next, curr, last);
                    const Operator_Data *p_op = g_data + op->kind;

                    if (p_next->precendence < p_op->precendence || 
                        (p_next->precendence == p_op->precendence && !p_next->right_associative)
                    )
                    {
                        if (make_node_from_output(op, output_stack, &output_count, flag_stack, &flag_count, lex))
                        {
                            return nullptr;
                        }
                        operator_count -= 1;
                    }
                    else
                    {
                        break;
                    }

                }
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                node->token_index = lex->index - 1;
                operator_stack[operator_count++] = node; 


            } break;
            case Token_Kind::OPEN_PAREN:
            {
                if (next.kind == Token_Kind::CLOSE_PAREN)
                {
                    fprintf(stderr, "ERROR: cannot have empty parenthesis pair\n");
                    print_error_here(lex->data, lex->data_length, curr.data_index);
                    return nullptr;
                }
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                operator_stack[operator_count++] = node; 
            } break;
            case Token_Kind::CLOSE_PAREN:
            {
                if (operator_count == 0)
                {
                    fprintf(stderr, "ERROR: unmatched parenthesis\n");
                    print_error_here(lex->data, lex->data_length, curr.data_index);
                    return nullptr;
                }
                while (operator_count != 0)
                {
                    Node *op = operator_stack[--operator_count];


                    if (op->kind == Node_Kind::OPEN_PAREN)
                    {
                        break;
                    }
                    else if (operator_count == 0)
                    {
                        fprintf(stderr, "ERROR: unmatched parenthesis\n");
                        print_error_here(lex->data, lex->data_length, curr.data_index);
                        return nullptr;
                    }

                    if (make_node_from_output(op, output_stack, &output_count, flag_stack, &flag_count, lex))
                    {
                        return nullptr;
                    }
                }

            } break;
            case Token_Kind::NUMBER:
            {
                Node *node = alloc(Node, 1);
                node->kind = Node_Kind::NUMBER;
                node->num = atof(lex->data + curr.data_index);
                node->token_index = lex->index - 1;
                output_stack[output_count++] = node;
            } break;
            case Token_Kind::IDENTIFIER:
            {
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                node->token_index = lex->index - 1;
                if (node->kind == Node_Kind::FUNCTION)
                {
                    operator_stack[operator_count++] = node;
                    flag_stack[flag_count++] = Control_Flag::in_function;
                }
                else /* if VARIABLE */
                {
                    output_stack[output_count++] = node;
                }
            } break;

            default: 
                assert(false);
        }
        last = curr;
        curr = next;
        next = next_token(lex);
    }
    while (operator_count > 0)
    {
        Node *op = operator_stack[--operator_count];

        if (op->kind == Node_Kind::OPEN_PAREN)
        {
            fprintf(stderr, "ERROR: Missing closing parenthesis\n");
            print_error_here_token(lex, op->token_index);
            return nullptr;
        }


        if (make_node_from_output(op, output_stack, &output_count, flag_stack, &flag_count, lex))
        {
            return nullptr;
        }
    }

    return output_stack[0];
}


static Node *parse(Lexer *lex)
{
    return parse_arithmetic(lex);
}

#include <math.h>


static bool strequal(const char *d0, u32 l0, const char *d1, u32 l1)
{
    if (l0 != l1) return false;

    for (u32 i = 0; i < l0; ++i)
    {
        if (d0[i] != d1[i]) return false;
    }
    return true;
}


struct Functions
{
    f64 (*func)(...);
    const char *func_str;
    u32 len;
    u32 args;
};


Functions g_funcs[] = {
    {(f64 (*)(...))sin, "sin", 3, 1},
    {(f64 (*)(...))cos, "cos", 3, 1},
    {(f64 (*)(...))tan, "tan", 3, 1},
    //
    {(f64 (*)(...))asin, "asin", 4, 1},
    {(f64 (*)(...))acos, "acos", 4, 1},
    {(f64 (*)(...))atan, "atan", 4, 1},
    //
    {(f64 (*)(...))sqrt, "sqrt", 4, 1},
    {(f64 (*)(...))cbrt, "cbrt", 4, 1},
    //
    {(f64 (*)(...))exp, "exp", 3, 1},
};


struct Variables
{
    f64 num;
    const char *var_str;
    u32 len;
};

Variables g_vars[] = {
    {2.718281828459045, "e", 1},
    {3.141592653589793, "pi", 2},
};

static f64 exe_expression(Node *tree, Lexer *lex, bool *err);

static f64 call_func(Node **arg_stack, s64 func_index, Lexer *lex, bool *err)
{
    Node **s = arg_stack;
    s64 i = func_index;
    u32 a = g_funcs[i].args;
    f64 (*f)(...) = g_funcs[i].func;

    switch (a)
    {
        case 0: assert(false);
        case 1: return f(exe_expression(s[a - 1], lex, err));
        case 2: return f(exe_expression(s[a - 1], lex, err), exe_expression(s[a - 2], lex, err));
        case 3: return f(exe_expression(s[a - 1], lex, err), exe_expression(s[a - 2], lex, err), exe_expression(s[a - 3], lex, err));
        case 4: return f(exe_expression(s[a - 1], lex, err), exe_expression(s[a - 2], lex, err), exe_expression(s[a - 3], lex, err), exe_expression(s[a - 4], lex, err));


        default: assert(false);
    }
}


static f64 exe_expression(Node *tree, Lexer *lex, bool *err)
{
    switch (tree->kind)
    {
        case Node_Kind::INVALID: assert(false);
        case Node_Kind::OPEN_PAREN: assert(false);

        case Node_Kind::POSITIVE: return exe_expression(tree->next, lex, err);
        case Node_Kind::NEGATE: return -exe_expression(tree->next, lex, err);

        case Node_Kind::ADD: return exe_expression(tree->left, lex, err) + exe_expression(tree->right, lex, err);
        case Node_Kind::SUB: return exe_expression(tree->left, lex, err) - exe_expression(tree->right, lex, err);
        case Node_Kind::MUL: return exe_expression(tree->left, lex, err) * exe_expression(tree->right, lex, err);
        case Node_Kind::DIV: return exe_expression(tree->left, lex, err) / exe_expression(tree->right, lex, err);
        case Node_Kind::POW: return pow(exe_expression(tree->left, lex, err), exe_expression(tree->right, lex, err));

        case Node_Kind::COMMA: assert(false);
        case Node_Kind::FUNCTION:
        {

            Token t = lex->tokens[tree->token_index];
            s64 func_index = -1;
            u32 expected_args = 0;
            for (u32 i = 0; i < ARRAY_SIZE(g_funcs); ++i)
            {
                if (strequal(g_funcs[i].func_str, g_funcs[i].len, lex->data + t.data_index, t.count))
                {
                    expected_args = g_funcs[i].args;
                    func_index = i;
                    break;
                }
            }


            if (func_index == -1)
            {
                // could not find function 
                fprintf(stderr, "Undefined function %.*s\n", t.count, lex->data + t.data_index);
                *err = true;
                return NAN;
            }
            assert(expected_args > 0);

            Node *arg_stack[64] = {};
            u32 args_count = 0;
            Node *sub_tree = tree->next;
            while (sub_tree->kind == Node_Kind::COMMA)
            {
                arg_stack[args_count++] = sub_tree->right;
                sub_tree = sub_tree->left;
            }
            arg_stack[args_count++] = sub_tree;


            if (args_count > expected_args)
            {
                fprintf(stderr, "ERROR: Expected %u args but got more\n", expected_args);
                print_error_here_token(lex, arg_stack[args_count - expected_args - 1]->token_index);
                *err = true;
                return NAN;
            }


            return call_func(arg_stack, func_index, lex, err);

        } break;
        case Node_Kind::VARIABLE:
        {
            Token t = lex->tokens[tree->token_index];
            for (u32 i = 0; i < ARRAY_SIZE(g_vars); ++i)
            {
                if (strequal(g_vars[i].var_str, g_vars[i].len, lex->data + t.data_index, t.count))
                {
                    return g_vars[i].num;
                }
            }

            fprintf(stderr, "Undefined variable %.*s\n", t.count, lex->data + t.data_index);
            *err = true;
            return NAN;

        } break;
        case Node_Kind::NUMBER:    
        {
            return tree->num;
        } break;
    }
}


enum class Op_Type
{
    INVALID,
    ADD,
    SUB,
    NEGATE,
    DIV,
    MUL,
    POW,

    PUSH,
    POP,
    RETURN,
};


const char *str_from_op_type(Op_Type t)
{
    switch (t)
    {

        case Op_Type::INVALID: return "INVALID";
        case Op_Type::ADD: return "ADD";
        case Op_Type::SUB: return "SUB";
        case Op_Type::NEGATE: return "NEGATE";
        case Op_Type::DIV: return "DIV";
        case Op_Type::MUL: return "MUL";
        case Op_Type::POW: return "POW";
        case Op_Type::PUSH: return "PUSH";
        case Op_Type::POP: return "POP";
        case Op_Type::RETURN: return "RETURN";
    }
    assert(false);
    return nullptr;
}


struct Op
{
    Op_Type type;
    f64 val;
};



static bool is_visited(Node **list, usize list_count, Node *n)
{
    for (usize i = 0; i < list_count; ++i)
    {
        if (list[i] == n)
        {
            return true;   
        }
    }
    return false;
}

static Op *bytecode_from_tree(Node *tree, Lexer *lex)
{
    usize op_size = 8192;
    Op *ops = alloc(Op, op_size);
    u32 ops_count = 0;

    Node **list = alloc(Node *, 1024);
    usize list_count = 0;

    Node **stack = alloc(Node *, 1024);
    usize stack_count = 0;
    
    Node **op_order_stack = alloc(Node *, 1024);
    usize order_count = 0;


    stack[stack_count++] = tree;


    while (stack_count > 0)
    {
        Node *active = stack[stack_count - 1];

        bool end = false;
        while (!end)
        {
            end = true;
            if (active->left != nullptr && !is_visited(list, list_count, active->left))
            {
                stack[stack_count++] = active->left;
                list[list_count++] = active->left;
                active = active->left;
                end = false;
            }
            else if (active->right != nullptr && !is_visited(list, list_count, active->right))
            {
                stack[stack_count++] = active->right;
                list[list_count++] = active->right;
                active = active->right;
                end = false;
            }
        }


        op_order_stack[order_count++] = active;
        stack_count -= 1;
    }


    for (u32 i = 0; i < order_count; ++i)
    {
        Node *node = op_order_stack[i];
        switch (node->kind)
        {
            case Node_Kind::INVALID: assert(false);
            case Node_Kind::OPEN_PAREN: assert(false);
            case Node_Kind::POSITIVE:
            {
                // do nothing
            } break;
            case Node_Kind::NEGATE:
            {
                Op op = {};
                op.type = Op_Type::NEGATE;
                op.val = 0;
                ops[ops_count++] = op;
            } break;
            case Node_Kind::ADD:
            {
                Op op = {};
                op.type = Op_Type::ADD;
                ops[ops_count++] = op;
            } break;
            case Node_Kind::SUB:
            {
                Op op = {};
                op.type = Op_Type::SUB;
                ops[ops_count++] = op;
            } break;
            case Node_Kind::MUL:
            {
                Op op = {};
                op.type = Op_Type::MUL;
                ops[ops_count++] = op;
            } break;
            case Node_Kind::DIV:
            {
                Op op = {};
                op.type = Op_Type::DIV;
                ops[ops_count++] = op;
            } break;
            case Node_Kind::POW:
            {
                Op op = {};
                op.type = Op_Type::POW;
                ops[ops_count++] = op;
            } break;
            case Node_Kind::COMMA: assert(false);
            case Node_Kind::FUNCTION: assert(false);
            {
                
            } break;
            case Node_Kind::VARIABLE: assert(false);
            case Node_Kind::NUMBER:
            {
                Op op = {};
                op.type = Op_Type::PUSH;
                op.val = node->num;
                ops[ops_count++] = op;
            } break;
        }
    }

    ops[ops_count++] = {.type = Op_Type::RETURN, .val = 0};


    return ops;
}


static f64 execute_ops(Op *ops)
{

    f64 *val_stack = alloc(f64, 1024);
    u32 stack_count = 0;

    bool running = true;
    for (u32 i = 0; running; ++i)
    {
        Op *op = ops + i;


        switch (op->type)
        {
            case Op_Type::INVALID: assert(false);
            case Op_Type::ADD:
            {
                f64 n1 = val_stack[--stack_count];
                f64 n2 = val_stack[--stack_count];
                val_stack[stack_count++] = n1 + n2;
            } break;
            case Op_Type::SUB:
            {
                f64 n1 = val_stack[--stack_count];
                f64 n2 = val_stack[--stack_count];
                val_stack[stack_count++] = n1 - n2;
            } break;
            case Op_Type::NEGATE: assert(false);
            case Op_Type::DIV:
            {
                f64 n1 = val_stack[--stack_count];
                f64 n2 = val_stack[--stack_count];
                val_stack[stack_count++] = n1 / n2;
            } break;
            case Op_Type::MUL:
            {
                f64 n1 = val_stack[--stack_count];
                f64 n2 = val_stack[--stack_count];
                val_stack[stack_count++] = n1 * n2;
            } break;
            case Op_Type::POW:
            {
                f64 n1 = val_stack[--stack_count];
                f64 n2 = val_stack[--stack_count];

                val_stack[stack_count++] = pow(n1, n2);
            } break;

            case Op_Type::PUSH:
            {
                val_stack[stack_count++] = op->val;
            } break;
            case Op_Type::POP: assert(false);
            case Op_Type::RETURN:
            {
                running = false;
            } break;

            default:
            {
                fprintf(stderr, "ERROR: Illegal operator\n");
                exit(1);
            } break;
        }
    }


    assert(stack_count == 1);
    return val_stack[0];
}


static void fprint_ops(Op *ops, FILE *f)
{
    for (usize i = 0; ; ++i)
    {
        fprintf(f, "index %llu: %s, %g\n", i, str_from_op_type(ops[i].type), ops[i].val);
        if (ops[i].type == Op_Type::RETURN)
        {
            break;
        }
    }
}

int main(void)
{
    bool running = true;
    while (running)
    {

        #if 0
        char input[1<<16];
        fgets(input, sizeof(input), stdin);
        if (input[0] == 'q') break;

        usize input_len = strlen(input) - 1;

        #else
        running = false;
        // char input[] = "1*2/(3^4-5)-+6+(7+8++9)";
        // char input[] = "1+(2)--3";

        char input[] = "cos(5^5)";
        // char input[] = "f(1,2+2)";
        usize input_len = strlen(input);
        #endif
        

        Lexer lexer;

        if (tokenize(&lexer, input, (u32) input_len))
        {
            continue;
        }

        Node *tree = parse(&lexer);
        if (tree == nullptr)
        {
            continue;
        }

        graphviz_from_tree(tree, &lexer);



        Op *ops = bytecode_from_tree(tree, &lexer);


        fprint_ops(ops, stdout);

        f64 val = execute_ops(ops);
        printf("%.*s = %g\n", (int)lexer.data_length, lexer.data, val);




        // bool err = false;
        // f64 val = exe_expression(tree, &lexer, &err);
        // if (!err)
        // {
        //     printf("val: %g\n", val);
        // }

    }
    return 0;
}
