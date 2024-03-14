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


#define todo() assert(false && "TODO")

struct [[nodiscard]] Errcode
{
    int code;
    operator int() const { return code; }
    Errcode(int c): code(c) {}
};


enum class Token_Kind
{
    INVALID,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    CARET,
    EQUAL,
    NUMBER,
    OPEN_PAREN,
    CLOSE_PAREN,
    COMMA,
    SEMICOLON,
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
        case Token_Kind::EQUAL: return "EQUAL";
        case Token_Kind::OPEN_PAREN: return "OPEN_PAREN";
        case Token_Kind::CLOSE_PAREN: return "CLOSE_PAREN";
        case Token_Kind::COMMA: return "COMMA";
        case Token_Kind::SEMICOLON: return "SEMICOLON";
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
            case '=':
            {
                lex->tokens[lex->count++] = {Token_Kind::EQUAL, i, 1};
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
            case ';':
            {
                lex->tokens[lex->count++] = {Token_Kind::SEMICOLON, i, 1};
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
    FUNCTION,
    FUNCTIONDEF,
    VARIABLE,
    PARAM,
    VARIABLEDEF,
    PROGRAM,
    EXPR,
    NUMBER
};

struct Node
{
    Node_Kind kind;

    union
    {
        struct
        {
            union
            {
                Node *left;
                Node *next;
            };
            Node *right;

        };
        Node *nodes[8];
    };



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
    /* FUNCTION */ {110, false},
    /* FUNCTIONDEF */ {0, false},
    /* VARIABLE */ {0, false},
    /* PARAM */ {0, false},
    /* VARIABLEDEF */ {0, false},
    /* PROGRAM */ {0, false},
    /* EXPR */ {0, false},
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
        case Node_Kind::FUNCTION: return "FUNCTION";
        case Node_Kind::FUNCTIONDEF: return "FUNCTIONDEF";
        case Node_Kind::VARIABLE: return "VARIABLE";
        case Node_Kind::PARAM: return "PARAM";
        case Node_Kind::VARIABLEDEF: return "VARIABLEDEF";
        case Node_Kind::PROGRAM: return "PROGRAM";
        case Node_Kind::EXPR: return "EXPR";
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
        switch (top->kind)
        {
            case Node_Kind::OPEN_PAREN: todo();
            case Node_Kind::INVALID:
            case Node_Kind::POSITIVE:
            case Node_Kind::NEGATE:
            case Node_Kind::ADD:
            case Node_Kind::SUB:
            case Node_Kind::MUL:
            case Node_Kind::PROGRAM:
            case Node_Kind::EXPR:
            {
                // do nothing
            } break; 
            case Node_Kind::DIV: todo();
            case Node_Kind::POW: todo();
            case Node_Kind::FUNCTIONDEF:
            case Node_Kind::FUNCTION:
            case Node_Kind::VARIABLE:
            case Node_Kind::PARAM:
            case Node_Kind::VARIABLEDEF:
            {
                Token t = lex->tokens[top->token_index];
                fprintf(f, "\n%.*s", t.count, lex->data + t.data_index);
            } break;
            case Node_Kind::NUMBER:
            {
                fprintf(f, "\n%.2f", top->num);
            } break;
        }
        fprintf(f, "\"];\n");

        for (u32 i = 0; i < ARRAY_SIZE(top->nodes); ++i)
        {
            Node *n = top->nodes[i];
            if (n != nullptr)
            {
                fprintf(f, "n%llu -- n%llu\n", (usize)top, (usize)n);
                stack[stack_count++] = n;
            }
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

static Token peek_token(Lexer *lex)
{
    Token t = lex->tokens[lex->index];
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
        case Node_Kind::ADD:
        case Node_Kind::SUB:
        case Node_Kind::MUL:
        case Node_Kind::DIV:
        case Node_Kind::POW:
        {
            return true;
        } break;
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
        else assert(false);
    }
    return kind;
}

static Errcode make_node_from_output(Node *op, Node **output_stack, u32 *output_count, const Lexer *lex)
{
    if (op->kind == Node_Kind::POSITIVE || op->kind == Node_Kind::NEGATE)
    {
        init_unary_node(op, output_stack, output_count);
        output_stack[(*output_count)++] = op;
    }
    else if (is_binary(op->kind))
    {
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
static Errcode parse_arithmetic(Lexer *lex, Node **output_stack, u32 *output_count, Control_Flag *flag_stack, u32 *flag_count)
{
    Node **operator_stack = alloc(Node *, 32);
    u32 operator_count = 0;


    Token last = {};
    Token curr = next_token(lex);
    Token next = lex->tokens[lex->index];

    while (curr.kind != Token_Kind::SEMICOLON && curr.kind != Token_Kind::END)
    {
        assert(curr.kind != Token_Kind::INVALID);

        switch (curr.kind)
        {
            case Token_Kind::PLUS:
            case Token_Kind::MINUS:
            case Token_Kind::STAR:
            case Token_Kind::SLASH:
            case Token_Kind::CARET:
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
                        if (make_node_from_output(op, output_stack, output_count, lex))
                        {
                            return 1;
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
                    return 1;
                }
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                node->token_index = lex->index - 1;
                operator_stack[operator_count++] = node; 
            } break;
            case Token_Kind::CLOSE_PAREN:
            {
                if (operator_count == 0)
                {
                    fprintf(stderr, "ERROR: unmatched parenthesis\n");
                    print_error_here(lex->data, lex->data_length, curr.data_index);
                    return 1;
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
                        return 1;
                    }

                    if (make_node_from_output(op, output_stack, output_count, lex))
                    {
                        return 1;
                    }
                }

            } break;
            case Token_Kind::EQUAL:
            {
                fprintf(stderr, "ERROR: equals cannot be in arithmetic expressions\n");
                print_error_here(lex->data, lex->data_length, curr.data_index);
                return 1;
            } break;
            case Token_Kind::NUMBER:
            {
                Node *node = alloc(Node, 1);
                node->kind = Node_Kind::NUMBER;
                node->num = atof(lex->data + curr.data_index);
                node->token_index = lex->index - 1;
                output_stack[*output_count] = node;
                *output_count += 1;
            } break;
            case Token_Kind::IDENTIFIER:
            {
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                node->token_index = lex->index - 1;
                if (node->kind == Node_Kind::FUNCTION)
                {
                    operator_stack[operator_count++] = node;
                    flag_stack[*flag_count] = Control_Flag::in_function;
                    *flag_count += 1;
                }
                else /* if VARIABLE */
                {
                    output_stack[*output_count] = node;
                    *output_count += 1;
                }
            } break;

            default: 
                assert(false);
        }
        last = curr;
        curr = next_token(lex);
        next = lex->tokens[lex->index];
    }
    while (operator_count > 0)
    {
        Node *op = operator_stack[--operator_count];

        if (op->kind == Node_Kind::OPEN_PAREN)
        {
            fprintf(stderr, "ERROR: Missing closing paren for this opening\n");
            print_error_here_token(lex, op->token_index);
            return 1;
        }


        if (make_node_from_output(op, output_stack, output_count, lex))
        {
            return 1;
        }
    }
    return 0;
}


static Node *parse(Lexer *lex)
{
    Node *output_stack[32] = {};
    u32 output_count = 0;

    Control_Flag flag_stack[32] = {};
    u32 flag_count = 0;

    u32 max_parameter_count = ARRAY_SIZE(output_stack[0]->nodes) - 1;

    while (lex->tokens[lex->index].kind != Token_Kind::END)
    {
        s64 equal_index = -1;
        for (s64 i = lex->index; i < lex->count && lex->tokens[i].kind != Token_Kind::SEMICOLON; ++i) 
        {
            Token *t = lex->tokens + i;
            if (t->kind == Token_Kind::EQUAL)
            {
                equal_index = i;
                break;
            }
        }


        if (equal_index != -1)
        {
            
            // parse func/var definition
            Token id = peek_token(lex);

            if (id.kind != Token_Kind::IDENTIFIER)
            {
                todo();
            }
            u32 token_index = lex->index;
            lex->index += 1;

            Token equal_or_open = peek_token(lex);

            if (equal_or_open.kind == Token_Kind::EQUAL)
            {
                lex->index += 1;
                int err = parse_arithmetic(lex, output_stack, &output_count, flag_stack, &flag_count);
                if (err)
                    return nullptr;

                Node *def = alloc(Node, 1);


                def->kind = Node_Kind::VARIABLEDEF;
                def->token_index = token_index;
                def->next = output_stack[--output_count];
                output_stack[output_count++] = def;
            }
            else if (equal_or_open.kind == Token_Kind::OPEN_PAREN)
            {
                u32 func_params = 0;
                while (true)
                {
                    lex->index += 1;
                    Token var = peek_token(lex);

                    if (var.kind != Token_Kind::IDENTIFIER)
                        todo();
                    if (func_params > max_parameter_count)
                        todo();


                    Node *n = alloc(Node, 1);
                    n->kind = Node_Kind::PARAM;
                    n->token_index = lex->index;

                    output_stack[output_count++] = n;

                    func_params += 1;
                    lex->index += 1;
                    Token comma_or_close = peek_token(lex);
                    if (comma_or_close.kind == Token_Kind::COMMA)
                    {
                        continue;
                    }
                    else if (comma_or_close.kind == Token_Kind::CLOSE_PAREN)
                    {
                        break;
                    }
                    else
                    {
                        fprintf(stderr, "ERROR: expected , or )\n");
                        print_error_here_token(lex, lex->index);
                        return nullptr;
                    }
                }

                Node *def = alloc(Node, 1);
                def->kind = Node_Kind::FUNCTIONDEF;
                def->token_index = token_index;
                for (u32 i = max_parameter_count; i-- > 0 && output_count > 0;)
                {   
                    Node *n = output_stack[output_count - 1];
                    if (n->kind != Node_Kind::PARAM)
                        break;
                    output_count -= 1;
                    def->nodes[i] = n;
                }

                lex->index += 1;
                if (peek_token(lex).kind != Token_Kind::EQUAL)
                {
                    fprintf(stderr, "ERROR: expected =\n");
                    print_error_here_token(lex, lex->index);
                    return nullptr;

                }
                lex->index += 1;

                int err = parse_arithmetic(lex, output_stack, &output_count, flag_stack, &flag_count);
                if (err)
                    return nullptr;

                def->nodes[max_parameter_count] = output_stack[--output_count];
                output_stack[output_count++] = def;

            }
            else
            {
                todo();
            }
        }
        else
        {
            Node *expr = alloc(Node, 1);
            expr->kind = Node_Kind::EXPR;
            int err = parse_arithmetic(lex, output_stack, &output_count, flag_stack, &flag_count);
            if (err)
                return nullptr;
            
            Node *arith = output_stack[--output_count];
            expr->next = arith;
            output_stack[output_count++] = expr;

        }

    }
    assert(output_count > 0);
    assert(output_count < 8);

    Node *n = alloc(Node, 1);
    n->kind = Node_Kind::PROGRAM;

    for (u32 i = ARRAY_SIZE((*output_stack)->nodes) - 1; i-- > 0;)
    {
        if (output_count == 0)
            break;

        n->nodes[i] = output_stack[--output_count];
    }

    output_stack[output_count++] = n;


    assert(output_count == 1);
    return output_stack[0];
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


struct Function
{
    f64 (*func)(...);
    const char *func_str;
    u32 args;
};


Function g_funcs[] = {
    {(f64 (*)(...))sin, "sin", 1},
    {(f64 (*)(...))cos, "cos", 1},
    {(f64 (*)(...))tan, "tan", 1},
    //
    {(f64 (*)(...))asin, "asin", 1},
    {(f64 (*)(...))acos, "acos", 1},
    {(f64 (*)(...))atan, "atan", 1},
    //
    {(f64 (*)(...))sqrt, "sqrt", 1},
    {(f64 (*)(...))cbrt, "cbrt", 1},
    //
    {(f64 (*)(...))exp, "exp", 1},
};


struct Variables
{
    f64 num;
    const char *var_str;
};

Variables g_predefined_vars[] = {
    {2.718281828459045, "e"},
    {3.141592653589793, "pi"},
};


enum class Op_Type
{
    INVALID,
    ADD,
    SUB,
    NEGATE,
    DIV,
    MUL,
    POW,

    BUILTIN_FUNC,
    BUILTIN_VAR,
    CALL,
    PUSH,
    PUSHI,
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
        case Op_Type::BUILTIN_FUNC: return "BUILTIN_FUNC";
        case Op_Type::BUILTIN_VAR: return "BUILTIN_VAR";
        case Op_Type::CALL: return "CALL";
        case Op_Type::PUSH: return "PUSH";
        case Op_Type::PUSHI: return "PUSHI";
        case Op_Type::POP: return "POP";
        case Op_Type::RETURN: return "RETURN";
    }
    assert(false);
    return nullptr;
}


struct Op
{
    Op_Type type;
    union 
    {
        f64 val;
        struct
        {
            u32 index;
            u32 arg_count;
        };
    };
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

enum class Symbol_Type
{
    INVALID,
    FUNCTION,
    VARIABLE,
    EXPR,
    BUILTIN_FUNC,
    BUILTIN_VAR,
};


struct Symbol
{
    Symbol_Type type;
    const char *name;
    u32 name_len;
    u32 scope;
    u32 index;
    u32 arg_count;
};

static void init_symbols_with_predefined(Symbol *syms, u32 *sym_count)
{
    for (u32 i = 0; i < ARRAY_SIZE(g_funcs); ++i)
    {
        Symbol sym = {};
        sym.type = Symbol_Type::BUILTIN_FUNC;
        sym.name = g_funcs[i].func_str;
        sym.name_len = (u32)strlen(g_funcs[i].func_str);
        sym.scope = 0;
        sym.index = i;
        sym.arg_count = g_funcs[i].args;
        syms[*sym_count] = sym;
        *sym_count += 1;
    }
    for (u32 i = 0; i < ARRAY_SIZE(g_predefined_vars); ++i)
    {
        Symbol sym = {};
        sym.type = Symbol_Type::BUILTIN_VAR;
        sym.name = g_predefined_vars[i].var_str;
        sym.name_len = (u32)strlen(g_predefined_vars[i].var_str);
        sym.scope = 0;
        sym.index = i;
        syms[*sym_count] = sym;
        *sym_count += 1;
    }
}


static s64 get_symbol_index_by_name(const char *name, u32 name_len, Symbol *symbols, u32 symbol_count, u32 active_scope)
{
    s64 index = -1;
    for (s64 j = 0; j < symbol_count; ++j)
    {
        if (symbols[j].scope != active_scope && symbols[j].scope != 0)
            continue;

        if (strequal(name, name_len, symbols[j].name, symbols[j].name_len))
        {
            index = j;
            if (symbols[j].scope == active_scope)
                break;
        }
    }
    return index;
}

struct Ops
{
    Op *data;
    u32 len;
    u32 entry;
};

static Ops bytecode_from_tree(Node *tree, const Lexer *lex)
{
    usize op_size = 8192;
    Op *ops = alloc(Op, op_size);
    u32 ops_count = 0;


    Symbol *symbols = alloc(Symbol, 2048);
    u32 symbol_count = 0;
    init_symbols_with_predefined(symbols, &symbol_count);
    

    u32 stack_count = 0;


    u32 active_scope = 0;
    u32 scope_count = 1;

    u32 active_index = 0;

    u32 param_count = 0;

    {
        Node **post_list = alloc(Node *, 1024);
        usize post_list_count = 0;

        Node **pre_list = alloc(Node *, 1024);
        usize pre_list_count = 0;

        Node **parse_stack = alloc(Node *, 1024);
        usize parse_stack_count = 0;

        parse_stack[parse_stack_count++] = tree;

        while (parse_stack_count > 0)
        {
            Node *active = parse_stack[parse_stack_count - 1];


            bool end = false;
            while (true)
            {
                end = true;

                if (!is_visited(pre_list, pre_list_count, active))
                {
                    pre_list[pre_list_count++] = active;
                    // printf("0x%llX preorder %s\n", (u64)active, str_from_node_kind(active->kind));

                    switch (active->kind)
                    {
                        case Node_Kind::FUNCTIONDEF: 
                        case Node_Kind::VARIABLEDEF:   
                        {
                            active_scope = scope_count++;
                            // fallthrough
                        }
                        case Node_Kind::EXPR: 
                        {
                            active_index = ops_count;
                        } break;
                        case Node_Kind::INVALID:
                        case Node_Kind::OPEN_PAREN:
                        case Node_Kind::POSITIVE:
                        case Node_Kind::NEGATE:
                        case Node_Kind::ADD:
                        case Node_Kind::SUB:
                        case Node_Kind::MUL:
                        case Node_Kind::DIV:
                        case Node_Kind::POW:
                        case Node_Kind::FUNCTION:
                        case Node_Kind::VARIABLE:
                        case Node_Kind::PARAM:
                        case Node_Kind::NUMBER:
                        case Node_Kind::PROGRAM:
                        {
                            // do nothing
                        } break;
                    }
                }


                for (u32 i = 0; i < ARRAY_SIZE(active->nodes); ++i)
                {

                    if (active->nodes[i] != nullptr && !is_visited(post_list, post_list_count, active->nodes[i]))
                    {
                        parse_stack[parse_stack_count++] = active->nodes[i];
                        active = active->nodes[i];
                        end = false;
                        break;
                    }
                }
                if (end)
                {
                    post_list[post_list_count++] = active;
                    // printf("0x%llX postorder %s\n", (u64)active, str_from_node_kind(active->kind));

                    break;
                }
            }

            // completed visiting everything under active
            Node *node = active;
            Token *token = lex->tokens + node->token_index;
            switch (node->kind)
            {
                case Node_Kind::INVALID: assert(false);
                case Node_Kind::OPEN_PAREN: assert(false);
                case Node_Kind::POSITIVE:
                case Node_Kind::PROGRAM:
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
                    stack_count -= 1;
                } break;
                case Node_Kind::SUB:
                {
                    Op op = {};
                    op.type = Op_Type::SUB;
                    ops[ops_count++] = op;
                    stack_count -= 1;
                } break;
                case Node_Kind::MUL:
                {
                    Op op = {};
                    op.type = Op_Type::MUL;
                    ops[ops_count++] = op;
                    stack_count -= 1;
                } break;
                case Node_Kind::DIV:
                {
                    Op op = {};
                    op.type = Op_Type::DIV;
                    ops[ops_count++] = op;
                    stack_count -= 1;
                } break;
                case Node_Kind::POW:
                {
                    Op op = {};
                    op.type = Op_Type::POW;
                    ops[ops_count++] = op;
                    stack_count -= 1;
                } break;
                case Node_Kind::FUNCTION:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);
                    if (index == -1)
                    {
                        // function not found
                        todo();
                    }

                    Op op = {};
                    op.type = Op_Type::CALL;
                    op.index = symbols[index].index;
                    op.arg_count = symbols[index].arg_count;
                    ops[ops_count++] = op;
                } break;
                case Node_Kind::FUNCTIONDEF:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);

                    if (index != -1)
                    {
                        // function already defined
                        todo();
                    }

                    Symbol s = {};

                    s.type = Symbol_Type::FUNCTION;
                    s.name = lex->data + token->data_index;
                    s.name_len = token->count;
                    s.scope = 0;
                    s.index = active_index;

                    u32 count = 0;
                    for (u32 i = ARRAY_SIZE(active->nodes) - 1; i-- > 0;)
                    {
                        if (active->nodes[i] != nullptr)
                            count += 1;
                    }

                    s.arg_count = count;

                    symbols[symbol_count++] = s;

                    active_scope = 0;
                    Op op = {};
                    op.type = Op_Type::RETURN;
                    ops[ops_count++] = op;
                    stack_count = 0;
                    param_count = 0;
                } break;
                case Node_Kind::VARIABLE:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);
                    if (index == -1)
                    {
                        // could not find variable
                        todo();
                    }

                    Op op = {};

                    if (symbols[index].scope == active_scope)
                    {
                        op.type = Op_Type::POP;
                        op.index = symbols[index].index;
                    }
                    else if (symbols[index].type == Symbol_Type::VARIABLE)
                    {
                        op.type = Op_Type::CALL;
                        op.index = symbols[index].index;
                        op.arg_count = 0;
                    }
                    else /*if BUILTIN_VAR*/
                    {
                        op.type = Op_Type::PUSHI;
                        op.val = g_predefined_vars[symbols[index].index].num;
                    }

                    ops[ops_count++] = op;
                    stack_count += 1;

                } break;
                case Node_Kind::PARAM:
                {
                    Symbol s = {};

                    s.type = Symbol_Type::VARIABLE;
                    Token *t = lex->tokens + node->token_index;
                    s.name = lex->data + t->data_index;
                    s.name_len = t->count;
                    s.scope = active_scope;
                    s.index = param_count++;

                    symbols[symbol_count++] = s;

                } break;
                case Node_Kind::VARIABLEDEF:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);

                    if (index != -1)
                    {
                        // trying to use already defined variable in global scope
                        todo();
                    }
                    

                    Symbol s = {};

                    s.type = Symbol_Type::VARIABLE;
                    s.name = lex->data + token->data_index;
                    s.name_len = token->count;
                    s.scope = 0;
                    s.index = active_index;

                    active_scope = 0;

                    symbols[symbol_count++] = s;

                    Op op = {};
                    op.type = Op_Type::RETURN;
                    ops[ops_count++] = op;
                    stack_count = 0;
                } break;
                case Node_Kind::EXPR:
                {
                    Symbol s = {};
                    s.type = Symbol_Type::EXPR;
                    s.scope = 0;
                    s.index = active_index; 

                    symbols[symbol_count++] = s;

                    Op op = {};
                    op.type = Op_Type::RETURN;
                    ops[ops_count++] = op;
                    stack_count = 0;
                } break;
                case Node_Kind::NUMBER:
                {
                    Op op = {};
                    op.type = Op_Type::PUSHI;
                    op.val = node->num;
                    ops[ops_count++] = op;
                    stack_count += 1;
                } break;
            }
            parse_stack_count -= 1;
        }
    }


    u32 entry = ops_count;

    for (u32 i = 0;  i < symbol_count; ++i)
    {
        if (symbols[i].type != Symbol_Type::EXPR)
            continue;
        if (symbols[i].scope != 0)
            continue;
        
        Op op = {};
        op.type = Op_Type::CALL;
        op.index = symbols[i].index;
        op.arg_count = 0;
        ops[ops_count++] = op;
    }
    

    return {ops, ops_count, entry};
}


static f64 execute_ops(Ops ops)
{

    f64 *val_stack = alloc(f64, 1024);
    u32 val_count = 0;

    u32 *call_stack = alloc(u32, 1024);
    u32 call_count = 0;

    bool running = true;
    for (u32 i = 0; running; ++i)
    {
        Op *op = ops.data + i;


        switch (op->type)
        {
            case Op_Type::INVALID: assert(false);
            case Op_Type::ADD:
            {
                f64 n1 = val_stack[--val_count];
                f64 n2 = val_stack[--val_count];
                val_stack[val_count++] = n1 + n2;
            } break;
            case Op_Type::SUB:
            {
                f64 n1 = val_stack[--val_count];
                f64 n2 = val_stack[--val_count];
                val_stack[val_count++] = n1 - n2;
            } break;
            case Op_Type::NEGATE:
            {
                f64 n1 = val_stack[--val_count];
                val_stack[val_count++] = -n1;
            } break;
            case Op_Type::DIV:
            {
                f64 n1 = val_stack[--val_count];
                f64 n2 = val_stack[--val_count];
                val_stack[val_count++] = n1 / n2;
            } break;
            case Op_Type::MUL:
            {
                f64 n1 = val_stack[--val_count];
                f64 n2 = val_stack[--val_count];
                val_stack[val_count++] = n1 * n2;
            } break;
            case Op_Type::POW:
            {
                f64 n1 = val_stack[--val_count];
                f64 n2 = val_stack[--val_count];

                val_stack[val_count++] = pow(n1, n2);
            } break;

            case Op_Type::BUILTIN_FUNC:
            {
                Function *f = g_funcs + op->index;

                switch (f->args)
                {
                    case 1:
                    {
                        f64 n1 = val_stack[--val_count];
                        val_stack[val_count++] = f->func(n1);
                    } break;
                    default: todo();
                }
                
            } break;
            case Op_Type::BUILTIN_VAR: todo();
            {
                val_stack[val_count++] = g_predefined_vars[op->index].num;
            } break;
            case Op_Type::CALL: todo();
            {
                call_stack[call_count++] = i;
                i = op->index;
            } break;
            case Op_Type::PUSH: assert(false);
            {
            } break;
            case Op_Type::PUSHI:
            {
                val_stack[val_count++] = op->val;
            } break;
            // case Op_Type::PUSH_RET: assert(false);   
            // {
            // } break;
            case Op_Type::POP: assert(false);
            case Op_Type::RETURN:
            {
                // last return stops program
                if (call_count == 0)
                {
                    running = false;
                }
                else
                {
                    i = call_stack[--call_count];
                }
            } break;

            // default:
            // {
            //     fprintf(stderr, "ERROR: Illegal operator\n");
            //     exit(1);
            // } break;
        }
    }


    assert(val_count == 1);
    return val_stack[0];
}


static void fprint_ops(Ops ops, FILE *f)
{
    for (usize i = 0; i < ops.len; ++i)
    {
        switch (ops.data[i].type)
        {
            case Op_Type::INVALID:
            case Op_Type::ADD:
            case Op_Type::SUB:
            case Op_Type::NEGATE:
            case Op_Type::DIV:
            case Op_Type::MUL:
            case Op_Type::POW:
            case Op_Type::RETURN:
            {
                fprintf(f, "index %llu: %s\n", i, str_from_op_type(ops.data[i].type));
            } break;
            case Op_Type::CALL:
            {
                fprintf(f, "index %llu: %s, 0x%X, args %u\n", i, str_from_op_type(ops.data[i].type), ops.data[i].index, ops.data[i].arg_count);
            } break;
            case Op_Type::PUSH:
            case Op_Type::POP:
            case Op_Type::BUILTIN_FUNC:
            case Op_Type::BUILTIN_VAR:
            {
                fprintf(f, "index %llu: %s, 0x%X\n", i, str_from_op_type(ops.data[i].type), ops.data[i].index);
            } break;
            case Op_Type::PUSHI:
            {
                fprintf(f, "index %llu: %s, %g\n", i, str_from_op_type(ops.data[i].type), ops.data[i].val);
            } break;
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

        char input[] = 
            "x = 5;\n"
            "y(x) = 5;\n"
            "f(x)=y(5+5)*(x*e);\n"
            "f(x) + 5;"
        ;
        // char input[] = "5 * 5";
        // char input[] = "x = 5 * y";
        // char input[] = "sqrt(5+5+5+5*5*5^5)";
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



        Ops ops = bytecode_from_tree(tree, &lexer);
        if (ops.len == 0)
        {
            continue;
        }

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
