#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



struct Err
{
    u32 error_index;
    String msg;
};

static u32 g_err_count = 0;
static Err g_err[512] = {};

struct Error
{
    u32 err_count;
    Err *errs;   
};

static Error get_error()
{
    return {g_err_count, g_err};
}

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
    u32 index;
    u32 data_length;
    char *data;
    u32 token_count;
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

static void report_error_here(String err_msg, u32 error_index)
{
    g_err[g_err_count++] = {error_index, err_msg};
}


static Errcode tokenize(Arena *arena, Lexer *lex, char *input, u32 input_length)
{
    memset(lex, 0, sizeof(*lex));
    memset(g_err, 0 , sizeof(g_err));
    g_err_count = 0;

    u64 token_capacity = 1 + 2 * input_length;
    lex->tokens = alloc(arena, Token, token_capacity);

    lex->data = input;
    lex->data_length = input_length;

    for (u32 i = 0; i < input_length; )
    {
        switch (input[i])
        {
            case '+':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::PLUS, i, 1};
                i += 1;
            } break;
            case '-':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::MINUS, i, 1};
                i += 1;
            } break;
            case '*':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::STAR, i, 1};
                i += 1;
            } break;
            case '/':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::SLASH, i, 1};
                i += 1;
            } break;
            case '^':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::CARET, i, 1};
                i += 1;
            } break;
            case '=':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::EQUAL, i, 1};
                i += 1;
            } break;
            case '(':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::OPEN_PAREN, i, 1};
                i += 1;
            } break;
            case ')':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::CLOSE_PAREN, i, 1};
                i += 1;
            } break;
            case ',':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::COMMA, i, 1};
                i += 1;
            } break;
            case ';':
            {
                lex->tokens[lex->token_count++] = {Token_Kind::SEMICOLON, i, 1};
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
                    String err_msg = string_from_cstr(arena, "invalid numeric literal");
                    report_error_here(err_msg, i);
                    return 1;
                }
                lex->tokens[lex->token_count++] = token;
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
                lex->tokens[lex->token_count++] = token;
            }
            else
            {
                String err_msg = tprintf_string(arena, "unrecognized input %c", input[i]);
                report_error_here(err_msg, i);
                return 1;
            }
        }
    }
    
    lex->tokens[lex->token_count++] = {Token_Kind::END, 0, 0};

    assert(lex->token_count <= token_capacity);

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



    u32 node_count;
    Node **nodes;


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


static void graphviz_from_tree(Arena *arena, Node *tree, Lexer *lex)
{
    FILE *f = fopen("./input.dot", "wb");


    fprintf(f, "graph G {\n");
    
    Node **stack = alloc(arena, Node *, 2000);
    u32 stack_count = 0;


    stack[stack_count++] = tree;


    while (stack_count > 0)
    {
        Node *top = stack[--stack_count];
        
        fprintf(f, "n%llu [label=\"%s", (usize)top, str_from_node_kind(top->kind));
        switch (top->kind)
        {
            case Node_Kind::OPEN_PAREN: assert(false);
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
            case Node_Kind::DIV:;
            case Node_Kind::POW:;
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

        for (u32 i = 0; i < top->node_count; ++i)
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



static void init_bin_node(Node *bin, Node **stack, u32 *stack_len, Arena *arena)
{
    u32 node_count = 2;
    bin->nodes = alloc(arena, Node *, node_count);
    bin->node_count = node_count;


    bin->nodes[1] = pop_sub_node(stack, stack_len);
    bin->nodes[0] = pop_sub_node(stack, stack_len);
}


static void init_unary_node(Node *bin, Node **stack, u32 *stack_len, Arena *arena)
{
    u32 node_count = 1;
    bin->nodes = alloc(arena, Node *, node_count);
    bin->node_count = node_count;


    bin->nodes[0] = pop_sub_node(stack, stack_len);
}


static void init_function_node(Node *bin, Node **stack, u32 *stack_len, Arena *arena)
{
    u32 node_count = 0;

    for (u32 i = *stack_len; i-- > 0;)
    {
        switch (stack[i]->kind)
        {
            case Node_Kind::INVALID: todo();
            case Node_Kind::OPEN_PAREN: todo();
            case Node_Kind::POSITIVE: todo();
            case Node_Kind::NEGATE: todo();
            case Node_Kind::ADD: todo();
            case Node_Kind::SUB: todo();
            case Node_Kind::MUL: todo();
            case Node_Kind::DIV: todo();
            case Node_Kind::POW: todo();
            case Node_Kind::FUNCTION: todo();
            case Node_Kind::PROGRAM: todo();
            case Node_Kind::FUNCTIONDEF:
            case Node_Kind::VARIABLEDEF:
            goto end;
            case Node_Kind::PARAM: todo();
            case Node_Kind::EXPR: todo();
            case Node_Kind::NUMBER:
            case Node_Kind::VARIABLE:
            {}
        }
        node_count += 1;
    }
    end:;

    bin->nodes = alloc(arena, Node *, node_count);
    bin->node_count = node_count;

    for (u32 i = 0; i < node_count; ++i)
    {
        bin->nodes[node_count - 1 - i] = pop_sub_node(stack, stack_len);
    }
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

static Errcode make_node_from_output(Node *op, Node **output_stack, u32 *output_count, Arena *arena, const Lexer *lex)
{
    if (op->kind == Node_Kind::POSITIVE || op->kind == Node_Kind::NEGATE)
    {
        init_unary_node(op, output_stack, output_count, arena);
        output_stack[(*output_count)++] = op;
    }
    else if (is_binary(op->kind))
    {
        if (*output_count < 2)
        {

            String err_msg = tprintf_string(arena, "expected 2 args but got %u", *output_count);
            report_error_here(err_msg, lex->tokens[op->token_index].data_index);
            return 1;
        }
        init_bin_node(op, output_stack, output_count, arena);
        output_stack[(*output_count)++] = op;
    }
    else if (op->kind == Node_Kind::FUNCTION)
    {
        init_function_node(op, output_stack, output_count, arena);
        output_stack[(*output_count)++] = op;
        // pop is_function
    }
    else
    {

        String err_msg = tprintf_string(arena, "unhandled operator %s", str_from_node_kind(op->kind));
        report_error_here(err_msg, lex->tokens[op->token_index].data_index);
        return 1;
    }
    return 0;
}



// https://www.youtube.com/watch?v=fIPO4G42wYE
static Errcode parse_arithmetic(Arena *arena, Lexer *lex, Node **output_stack, u32 *output_count, Control_Flag *flag_stack, u32 *flag_count)
{
    Node **operator_stack = alloc(arena, Node *, 32);
    u32 operator_count = 0;


    Token last = {};
    Token curr = next_token(lex);
    Token next = lex->tokens[lex->index];


    while (true)
    {
        switch (curr.kind)
        {
            case Token_Kind::INVALID: assert(false);
            case Token_Kind::SEMICOLON:
            case Token_Kind::END:
            {
                goto end;
            } break;
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
                        if (make_node_from_output(op, output_stack, output_count, arena, lex))
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
                Node *node = alloc(arena, Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                node->token_index = lex->index - 1;
                operator_stack[operator_count++] = node; 


            } break;
            case Token_Kind::OPEN_PAREN:
            {
                if (next.kind == Token_Kind::CLOSE_PAREN)
                {
                    String err_msg = string_from_cstr(arena, "cannot have empty parenthesis pair");
                    report_error_here(err_msg, curr.data_index);
                    return 1;
                }
                Node *node = alloc(arena, Node, 1);
                node->kind = node_kind_from_token(next, curr, last);
                node->token_index = lex->index - 1;
                operator_stack[operator_count++] = node; 
            } break;
            case Token_Kind::COMMA:
            {
                //TODO(Johan) fix f(x,y)=x;f(1,1) not parsing arithmetic expressions in functions correctly
                if (operator_count == 0)
                {
                    String err_msg = string_from_cstr(arena, "Empty expr infront of comma");
                    report_error_here(err_msg, curr.data_index);
                    return 1;
                }
                while (operator_count != 0)
                {
                    Node *op = operator_stack[--operator_count];


                    if (op->kind == Node_Kind::OPEN_PAREN)
                    {
                        operator_count +=1;
                        break;
                    }

                    if (make_node_from_output(op, output_stack, output_count, arena, lex))
                    {
                        return 1;
                    }
                }

            } break;
            case Token_Kind::CLOSE_PAREN:
            {
                if (operator_count == 0)
                {
                    String err_msg = string_from_cstr(arena, "unmatched parenthesis");
                    report_error_here(err_msg, curr.data_index);
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
                        String err_msg = string_from_cstr(arena, "unmatched parenthesis");
                        report_error_here(err_msg, curr.data_index);
                        return 1;
                    }

                    if (make_node_from_output(op, output_stack, output_count, arena, lex))
                    {
                        return 1;
                    }
                }

            } break;
            case Token_Kind::EQUAL:
            {
                String err_msg = string_from_cstr(arena, "equals cannot be in arithmetic expressions");
                report_error_here(err_msg, curr.data_index);
                return 1;
            } break;
            case Token_Kind::NUMBER:
            {
                Node *node = alloc(arena, Node, 1);
                node->kind = Node_Kind::NUMBER;
                node->num = atof(lex->data + curr.data_index);
                node->token_index = lex->index - 1;
                output_stack[*output_count] = node;
                *output_count += 1;
            } break;
            case Token_Kind::IDENTIFIER:
            {
                Node *node = alloc(arena, Node, 1);
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

            #ifndef _DEBUG
            default: 
                assert(false);
            #endif
        }
        last = curr;
        curr = next_token(lex);
        next = lex->tokens[lex->index];
    }
    end:;
    while (operator_count > 0)
    {
        Node *op = operator_stack[--operator_count];

        if (op->kind == Node_Kind::OPEN_PAREN)
        {
            String err_msg = string_from_cstr(arena, "Missing closing paren for this opening");
            report_error_here(err_msg, lex->tokens[op->token_index].data_index);
            return 1;
        }


        if (make_node_from_output(op, output_stack, output_count, arena, lex))
        {
            return 1;
        }
    }
    return 0;
}


static Node *parse(Arena *arena, Lexer *lex)
{
    Node *output_stack[32] = {};
    u32 output_count = 0;

    Control_Flag flag_stack[32] = {};
    u32 flag_count = 0;

    while (lex->tokens[lex->index].kind != Token_Kind::END)
    {
        s64 equal_index = -1;
        for (s64 i = lex->index; i < lex->token_count && lex->tokens[i].kind != Token_Kind::SEMICOLON; ++i) 
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
                String s = string_from_cstr(arena, "Cannot assign to non identifiers");
                report_error_here(s, id.data_index);
                return nullptr;
            }
            u32 token_index = lex->index;
            lex->index += 1;

            Token equal_or_open = peek_token(lex);

            if (equal_or_open.kind == Token_Kind::EQUAL)
            {
                lex->index += 1;
                int err = parse_arithmetic(arena, lex, output_stack, &output_count, flag_stack, &flag_count);
                if (err)
                    return nullptr;

                Node *def = alloc(arena, Node, 1);


                def->kind = Node_Kind::VARIABLEDEF;
                def->token_index = token_index;
                def->node_count = 1;
                def->nodes = alloc(arena, Node *, 1);
                def->nodes[0] = output_stack[--output_count];
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
                    {
                        String s = string_from_cstr(arena, "Expected identifiers in function declaration parameters");
                        report_error_here(s, var.data_index);
                        return nullptr;
                    }

                    Node *n = alloc(arena, Node, 1);
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
                        String err_msg = string_from_cstr(arena, "expected , or )");
                        report_error_here(err_msg, lex->tokens[lex->index].data_index);
                        return nullptr;
                    }
                }
                // make func def
                Node *def = alloc(arena, Node, 1);
                def->kind = Node_Kind::FUNCTIONDEF;
                def->node_count = func_params + 1;
                def->nodes = alloc(arena, Node *, def->node_count);
                def->token_index = token_index;
                for (u32 i = func_params; i-- > 0;)
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
                    u32 error_index = lex->tokens[lex->index].data_index;
                    String err_msg = string_from_cstr(arena, "expected =");
                    report_error_here(err_msg, error_index);

                    return nullptr;

                }
                lex->index += 1;

                int err = parse_arithmetic(arena, lex, output_stack, &output_count, flag_stack, &flag_count);
                if (err)
                    return nullptr;

                def->nodes[def->node_count - 1] = output_stack[--output_count];
                output_stack[output_count++] = def;

            }
            else
            {
                u32 error_index = lex->tokens[lex->index].data_index;
                String err_msg = string_from_cstr(arena, "Operators cannot preceed '=' in declaration");
                report_error_here(err_msg, error_index);

                return nullptr;
            }
        }
        else
        {
            Token t = peek_token(lex);
            if (t.kind == Token_Kind::SEMICOLON)
            {
                lex->index += 1;
                Node *expr = alloc(arena, Node, 1);
                expr->kind = Node_Kind::EXPR;
                output_stack[output_count++] = expr;
                continue;
            }

            Node *expr = alloc(arena, Node, 1);
            expr->kind = Node_Kind::EXPR;
            int err = parse_arithmetic(arena, lex, output_stack, &output_count, flag_stack, &flag_count);
            if (err)
                return nullptr;
            
            Node *arith = output_stack[--output_count];
            expr->node_count = 1;
            expr->nodes = alloc(arena, Node *, 1);
            expr->nodes[0] = arith;
            output_stack[output_count++] = expr;

        }

    }

    Node *n = alloc(arena, Node, 1);
    n->kind = Node_Kind::PROGRAM;
    n->nodes = alloc(arena, Node *, output_count);
    n->node_count = output_count;

    for (u32 i = n->node_count; i-- > 0;)
    {
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
    CALL,
    PUSHI,
    MOVE,
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
        case Op_Type::CALL: return "CALL";
        case Op_Type::PUSHI: return "PUSHI";
        case Op_Type::MOVE: return "MOVE";
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
    String name;
    u32 scope;
    u32 index;
    u32 arg_count;
};

static void init_symbols_with_predefined(Arena *arena, Symbol *syms, u32 *sym_count)
{
    for (u32 i = 0; i < ARRAY_SIZE(g_funcs); ++i)
    {
        Symbol sym = {};
        sym.type = Symbol_Type::BUILTIN_FUNC;
        sym.name = string_from_cstr(arena, g_funcs[i].func_str);
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
        sym.name = string_from_cstr(arena, g_predefined_vars[i].var_str);
        sym.scope = 0;
        sym.index = i;
        syms[*sym_count] = sym;
        *sym_count += 1;
    }
}


static s64 get_symbol_index_by_name(char *name, u32 name_len, Symbol *symbols, u32 symbol_count, u32 active_scope)
{
    s64 index = -1;
    for (s64 j = 0; j < symbol_count; ++j)
    {
        if (symbols[j].scope != active_scope && symbols[j].scope != 0)
            continue;

        if (strequal(name, name_len, symbols[j].name.data, (u32)symbols[j].name.len))
        {
            index = j;
            if (symbols[j].scope == active_scope)
                break;
        }
    }
    return index;
}

struct Program
{
    Op *data;
    u32 data_len;
    Symbol *syms;
    u32 sym_len;
    u32 predefined_end;
};

static Errcode bytecode_from_tree(Arena *arena, Program *out_program, Node *tree, const Lexer *lex)
{
    if (out_program == nullptr)
        return 1;
     
    usize op_size = 8192;
    Op *ops = alloc(arena, Op, op_size);
    u32 ops_count = 0;


    Symbol *symbols = alloc(arena, Symbol, 2048);
    u32 symbol_count = 0;
    init_symbols_with_predefined(arena, symbols, &symbol_count);
    
    u32 predefined_end = symbol_count;

    u32 active_scope = 0;
    u32 scope_count = 1;

    u32 active_index = 0;

    u32 param_count = 0;

    {
        Node **post_list = alloc(arena, Node *, 1024);
        usize post_list_count = 0;

        Node **pre_list = alloc(arena, Node *, 1024);
        usize pre_list_count = 0;

        Node **parse_stack = alloc(arena, Node *, 1024);
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


                for (u32 i = 0; i < active->node_count; ++i)
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
                case Node_Kind::FUNCTION:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);
                    if (index == -1)
                    {
                        String s = tprintf_string(arena, "Undeclared function %.*s", token->count, lex->data + token->data_index);
                        report_error_here(s, token->data_index);
                        return 1;
                    }

                    if (node->node_count != symbols[index].arg_count)
                    {
                        // called function with more arguments than allowed
                        String s = tprintf_string(arena, "Function %.*s takes in %u args not %u", token->count, lex->data + token->data_index, symbols[index].arg_count, node->node_count);
                        report_error_here(s, token->data_index);
                        return 1;
                    }

                    if (symbols[index].type == Symbol_Type::BUILTIN_FUNC)
                    {
                        Op op = {};
                        op.type = Op_Type::BUILTIN_FUNC;
                        op.index = symbols[index].index;
                        op.arg_count = symbols[index].arg_count;
                        ops[ops_count++] = op;
                    } /*if Function*/
                    else
                    {
                        Op op = {};
                        op.type = Op_Type::CALL;
                        op.index = symbols[index].index;
                        op.arg_count = symbols[index].arg_count;
                        ops[ops_count++] = op;
                    }
                } break;
                case Node_Kind::FUNCTIONDEF:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);

                    if (index != -1)
                    {
                        // function already defined
                        String s = tprintf_string(arena, "Function %.*s already defined", token->count, lex->data + token->data_index);
                        report_error_here(s, token->data_index);
                        return 1;
                    }

                    Symbol s = {};

                    s.type = Symbol_Type::FUNCTION;
                    s.name.data = lex->data + token->data_index;
                    s.name.len = token->count;
                    s.scope = 0;
                    s.index = active_index;

                    u32 count = 0;
                    for (u32 i = active->node_count - 1; i-- > 0;)
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
                    param_count = 0;
                } break;
                case Node_Kind::VARIABLE:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);
                    if (index == -1)
                    {
                        // could not find variable
                        String s = tprintf_string(arena, "Undeclared variable %.*s", (int)token->count, lex->data + token->data_index);
                        report_error_here(s, token->data_index);
                        return 1;
                    }

                    Op op = {};

                    if (symbols[index].scope == active_scope && symbols[index].scope != 0)
                    {
                        op.type = Op_Type::MOVE;
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

                } break;
                case Node_Kind::PARAM:
                {
                    Symbol s = {};

                    s.type = Symbol_Type::VARIABLE;
                    Token *t = lex->tokens + node->token_index;
                    s.name.data = lex->data + t->data_index;
                    s.name.len = t->count;
                    s.scope = active_scope;
                    s.index = param_count++;

                    symbols[symbol_count++] = s;

                } break;
                case Node_Kind::VARIABLEDEF:
                {
                    s64 index = get_symbol_index_by_name(lex->data + token->data_index, token->count, symbols, symbol_count, active_scope);

                    if (index != -1)
                    {
                        // trying to define already defined variable in global scope
                        String s = tprintf_string(arena, "%.*s is already defined", (int)token->count, lex->data + token->data_index);
                        report_error_here(s, token->data_index);
                        return 1;
                    }
                    

                    Symbol s = {};

                    s.type = Symbol_Type::VARIABLE;
                    s.name.data = lex->data + token->data_index;
                    s.name.len = token->count;
                    s.scope = 0;
                    s.index = active_index;

                    active_scope = 0;

                    symbols[symbol_count++] = s;

                    Op op = {};
                    op.type = Op_Type::RETURN;
                    ops[ops_count++] = op;
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
                } break;
                case Node_Kind::NUMBER:
                {
                    Op op = {};
                    op.type = Op_Type::PUSHI;
                    op.val = node->num;
                    ops[ops_count++] = op;
                } break;
            }
            parse_stack_count -= 1;
        }
    }


    // const char *str = "print_val";
    // s64 index_ = get_symbol_index_by_name(str, (u32)strlen(str), symbols, symbol_count, 0);
    // assert(index_ != -1);
    // u32 index = (u32)index_;

    // u32 index = 0;
    // u32 entry = ops_count;
    // for (u32 i = 0;  i < symbol_count; ++i)
    // {
    //     if (symbols[i].scope != 0)
    //         continue;

    //     switch (symbols[i].type)
    //     {

    //         case Symbol_Type::BUILTIN_FUNC:
    //         case Symbol_Type::BUILTIN_VAR:
    //         case Symbol_Type::INVALID:
    //         {
    //             // do nothing    
    //         } break;
    //         case Symbol_Type::FUNCTION:
    //         {
    //             index += 1;
    //         } break;
    //         case Symbol_Type::VARIABLE:
    //         case Symbol_Type::EXPR:
    //         {


    //             Op op = {};
    //             op.type = Op_Type::CALL;
    //             op.index = symbols[i].index;
    //             op.arg_count = 0;
    //             ops[ops_count++] = op;
                
    //             op = {};

    //             op.type = Op_Type::APPEND_TO_RESULT;
    //             op.index = index;
    //             ops[ops_count++] = op;
    //             index += 1;
    //         } break;
    //     }
    //     // op = {};
    //     // op.type = Op_Type::BUILTIN_FUNC;
    //     // op.index = index;
    //     // op.arg_count = 1;
    //     // ops[ops_count++] = op;

    // }
    
    // Op op = {};
    // op.type = Op_Type::RETURN;
    // ops[ops_count++] = op;

    Program program = {};
    program.data = ops;
    program.data_len = ops_count;
    program.syms = symbols;
    program.sym_len = symbol_count;
    program.predefined_end = predefined_end;

    *out_program = program;
    return 0;
}


static void push_f64(u8 *stack, u32 *stack_top, f64 val)
{
    u32 index = *stack_top;
    *stack_top += sizeof(val);

    f64 *stac = (f64 *)(stack + index);
    *stac = val;
}

static void push_u32(u8 *stack, u32 *stack_top, u32 val)
{
    u32 index = *stack_top;
    *stack_top += sizeof(val);

    u32 *stac = (u32 *)(stack + index);
    *stac = val;
}

static f64 pop_f64(u8 *stack, u32 *stack_top)
{
    assert(*stack_top >= sizeof(f64));
    u32 index = *stack_top - sizeof(f64);    
    f64 *stac = (f64 *)(stack + index);

    f64 val = *stac;

    *stack_top -= sizeof(f64);
    return val;
}

static u32 pop_u32(u8 *stack, u32 *stack_top)
{
    assert(*stack_top >= sizeof(u32));
    u32 index = *stack_top - sizeof(u32);    
    u32 *stac = (u32 *)(stack + index);

    u32 val = *stac;

    *stack_top -= sizeof(u32);
    return val;
}

struct Result
{
    f64 *result;
    u32 result_count;
};


static Result execute_ops(Arena *arena, Program program, u32 entry, f64 *inputs, u32 input_count)
{

    u8 *val_stack = alloc(arena, u8, 8192);
    u32 val_stack_top = 0;

    u8 *call_stack = alloc(arena, u8, 8192);
    u32 call_count = 0;


    u32 frame_index = 0;


    for (u32 i = 0; i < input_count; ++i)
    {
        push_f64(call_stack, &call_count, inputs[i]);
    }

    bool running = true;
    for (u32 i = entry; running;)
    {
        Op *op = program.data + i;

        switch (op->type)
        {
            case Op_Type::INVALID: assert(false);
            case Op_Type::ADD:
            {
                f64 n2 = pop_f64(val_stack, &val_stack_top);
                f64 n1 = pop_f64(val_stack, &val_stack_top);
                push_f64(val_stack, &val_stack_top, n1 + n2);
                i += 1;
            } break;
            case Op_Type::SUB:
            {
                f64 n2 = pop_f64(val_stack, &val_stack_top);
                f64 n1 = pop_f64(val_stack, &val_stack_top);
                push_f64(val_stack, &val_stack_top, n1 - n2);
                i += 1;
            } break;
            case Op_Type::NEGATE:
            {
                f64 n1 = pop_f64(val_stack, &val_stack_top);
                push_f64(val_stack, &val_stack_top, -n1);
                i += 1;
            } break;
            case Op_Type::DIV:
            {
                f64 n2 = pop_f64(val_stack, &val_stack_top);
                f64 n1 = pop_f64(val_stack, &val_stack_top);
                push_f64(val_stack, &val_stack_top, n1 / n2);
                i += 1;
            } break;
            case Op_Type::MUL:
            {
                f64 n2 = pop_f64(val_stack, &val_stack_top);
                f64 n1 = pop_f64(val_stack, &val_stack_top);
                push_f64(val_stack, &val_stack_top, n1 * n2);
                i += 1;
            } break;
            case Op_Type::POW:
            {
                f64 n2 = pop_f64(val_stack, &val_stack_top);
                f64 n1 = pop_f64(val_stack, &val_stack_top);
                push_f64(val_stack, &val_stack_top, pow(n1, n2));
                i += 1;
            } break;
            case Op_Type::BUILTIN_FUNC:
            {
                Function *f = g_funcs + op->index;

                switch (f->args)
                {
                    case 1:
                    {
                        f64 n1 = pop_f64(val_stack, &val_stack_top);
                        push_f64(val_stack, &val_stack_top, f->func(n1));
                    } break;
                    default: assert(false && "add extra params for builtins");
                }
                
                i += 1;
            } break;

            case Op_Type::CALL:
            {
                u32 old_frame_index = frame_index;
                push_u32(call_stack, &call_count, i + 1);
                frame_index = call_count + 4;
                push_u32(call_stack, &call_count, old_frame_index);

                for (u32 j = 0; j < op->arg_count; ++j)
                {
                    u8 *u_args = val_stack + val_stack_top;
                    f64 *args = (f64 *)u_args - op->arg_count;

                    push_f64(call_stack, &call_count, args[j]);
                }
                val_stack_top -= op->arg_count * sizeof(f64);



                i = op->index;
            } break;
            case Op_Type::PUSHI:
            {
                push_f64(val_stack, &val_stack_top, op->val);
                i += 1;
            } break;
            case Op_Type::MOVE:
            {
                f64 n1 = *(f64 *)(call_stack + frame_index + op->index * sizeof(f64));
                // printf("move\n");

                push_f64(val_stack, &val_stack_top, n1);
                i += 1;
            } break;
            case Op_Type::RETURN:
            {
                // last return stops program
                if (frame_index == 0)
                {
                    running = false;
                    break;
                }
                call_count = frame_index;
                frame_index = pop_u32(call_stack, &call_count);
                u32 ret = pop_u32(call_stack, &call_count);

                i = ret;
            } break;


            #ifndef _DEBUG
            default:
            {
                fprintf(stderr, "ERROR: Illegal operator\n");
                assert(false);
            } break;
            #endif
        }   
    }

    // for (u32 i = 0; i < val_stack_top; i += sizeof(f64))
    // {
    //     printf("%g\n", *(f64 *)(val_stack + i));
    // }

    Result r = {};
    r.result = (f64 *)val_stack;
    r.result_count = val_stack_top / sizeof(f64);
    return r;
}


static void fprint_ops(Program ops, FILE *f)
{
    for (usize i = 0; i < ops.data_len; ++i)
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
            case Op_Type::MOVE:
            case Op_Type::BUILTIN_FUNC:
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

static Result execute_function(Arena *arena, Program program, String sym_name, f64 *inputs, u32 input_count)
{
    Result r = {};
    for (u32 i = 0; i < program.sym_len; ++i)
    {
        Symbol *s = program.syms + i; 
        if (s->scope != 0)
            continue;
        if (!String_equal(sym_name, s->name))
            continue;

        if (s->arg_count == input_count)
        {
            r = execute_ops(arena, program, s->index, inputs, input_count);
        }
        break;
    }
    return r;
}

Errcode compile(Arena *arena, char *input, u32 input_len, Program *out_ops)
{
    int err = 0;
    Lexer lexer = {};
    err = tokenize(arena, &lexer, input, input_len);
    if (err)
        return err;


    Node *tree = parse(arena, &lexer);
    if (tree == nullptr)
    {
        return 1;
    }

    graphviz_from_tree(arena, tree, &lexer);


    err = bytecode_from_tree(arena, out_ops, tree, &lexer);
    if (err)
        return err;

    fprint_ops(*out_ops, stdout);

    return err;
}


static void print_lexer(const Lexer *lex)
{
    printf("index: %u\n", lex->index);
    printf("data: %.*s\n", (int)lex->data_length, lex->data);

    for (u32 i = 0;  i < lex->token_count; ++i)
    {
        Token t = lex->tokens[i];
        printf("token: %s from %.*s\n", str_from_token_kind(t.kind), (int)t.count, lex->data + t.data_index);
    }
}


int test(void)
{
    Arena temp_arena; init_arena(&temp_arena, 1000000);
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
            "x = 5;"
            "f(x, y) = x*y;"
        ;
        // char input[] = "5 * 5";
        // char input[] = "x = 5 * y";
        // char input[] = "sqrt(5+5+5+5*5*5^5)";
        // char input[] = "f(1,2+2)";
        usize input_len = strlen(input);
        #endif
        

        Lexer lexer;

        if (tokenize(&temp_arena, &lexer, input, (u32) input_len))
        {
            continue;
        }

        print_lexer(&lexer);

        Node *tree = parse(&temp_arena, &lexer);
        if (tree == nullptr)
        {
            continue;
        }

        graphviz_from_tree(&temp_arena, tree, &lexer);


        Program ops;
        if (bytecode_from_tree(&temp_arena, &ops, tree, &lexer))
        {
            continue;
        }
        fprint_ops(ops, stdout);

        // execute_ops(&temp_arena, ops);
        // printf("%.*s = %g\n", (int)lexer.data_length, lexer.data, val);




        // bool err = false;
        // f64 val = exe_expression(tree, &lexer, &err);
        // if (!err)
        // {
        //     printf("val: %g\n", val);
        // }

    }
    return 0;
}
