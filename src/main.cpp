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
    Token tokens[4096];
    u32 count;
    char *data;
    u32 data_length;
    u32 index;
};










static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
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





static Errcode tokenize(Lexer *lex, char *input, u32 input_length)
{
    memset(lex, 0, sizeof(*lex));

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
            default:
            if (is_digit(input[i]))
            {
                Token token = {};
                token.kind = Token_Kind::NUMBER;
                token.data_index = i;

                while (is_digit(input[i]))
                {
                    token.count += 1;
                    i += 1;
                }
                lex->tokens[lex->count++] = token;
            }
            else
            {
                fprintf(stderr, "ERROR: unrecognized input %c\n", input[i]);
                return 1;
            }
        }


    }
    
    lex->tokens[lex->count++] = {Token_Kind::END, 0, 0};

    return 0;
}


enum Node_Kind 
{
    INVALID,
    OPEN_PAREN,
    // EXPR,
    POSITIVE,
    NEGATE,
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    NUMBER
};


struct Node
{
    Node_Kind kind;

    Node *right;
    Node *left;

    union
    {
        f64 num;
    };
};


struct Operator_Data
{
    s64 precendence;
    bool right_associative;
};


const Operator_Data g_data[] = {
    /* INVALID */  {0, false},
    /* OPEN_PAREN */ {0, false},
    // /* EXPR */     {0, false},
    /* POSITIVE */ {100, true},
    /* NEGATE */   {100, true},
    /* ADD */      {10, false},
    /* SUB */      {10, false},
    /* MUL */      {20, false},
    /* DIV */      {20, false},
    /* POW */      {30, true},
    /* NUMBER */   {0, false},
};

static const char *str_from_node_kind(Node_Kind kind)
{
    switch (kind)
    {
        case Node_Kind::INVALID: return "INVALID";
        case Node_Kind::OPEN_PAREN: return "OPEN_PAREN";
        // case Node_Kind::EXPR return "EXPR";
        case Node_Kind::POSITIVE: return "POSITIVE";
        case Node_Kind::NEGATE: return "NEGATE";
        case Node_Kind::ADD: return "ADD";
        case Node_Kind::SUB: return "SUB";
        case Node_Kind::MUL: return "MUL";
        case Node_Kind::DIV: return "DIV";
        case Node_Kind::POW: return "POW";
        case Node_Kind::NUMBER: return "NUMBER";
    }
    assert(false);
    return nullptr;
}


static void graphviz_from_tree(Node *tree)
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


static Token next_token(Lexer *lex)
{
    Token t = lex->tokens[lex->index];
    lex->index += 1;
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


static Node *get_sub_node(Node **s, u32 *size)
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



static void init_bin_node(Node *bin, Node_Kind kind, Node **stack, u32 *stack_len)
{
    bin->kind = kind;
    bin->right = get_sub_node(stack, stack_len);
    bin->left = get_sub_node(stack, stack_len);
}


static void init_unary_node(Node *bin, Node_Kind kind, Node **stack, u32 *stack_len)
{
    bin->kind = kind;
    bin->right = get_sub_node(stack, stack_len);
}


static Node_Kind node_kind_from_token(Token next, Token last)
{
    Node_Kind kind = Node_Kind::INVALID;

    if (next.kind == Token_Kind::OPEN_PAREN)
    {
        kind = Node_Kind::OPEN_PAREN;
    }
    else if (last.kind != Token_Kind::NUMBER && last.kind != Token_Kind::CLOSE_PAREN)
    {
        if (next.kind == Token_Kind::PLUS) kind = Node_Kind::POSITIVE;
        else if (next.kind == Token_Kind::MINUS) kind = Node_Kind::NEGATE;
        else assert(false);
    }
    else /*if binary*/
    {
        if (next.kind == Token_Kind::PLUS) kind = Node_Kind::ADD;
        else if (next.kind == Token_Kind::MINUS) kind = Node_Kind::SUB;
        else if (next.kind == Token_Kind::STAR) kind = Node_Kind::MUL;
        else if (next.kind == Token_Kind::SLASH) kind = Node_Kind::DIV;
        else if (next.kind == Token_Kind::CARET) kind = Node_Kind::POW;
        else if (next.kind == Token_Kind::OPEN_PAREN) kind = Node_Kind::OPEN_PAREN;
        else assert(false);
    }
    return kind;
}


// https://www.youtube.com/watch?v=fIPO4G42wYE
static Node *parse_expression(Lexer *lex)
{
    Node **operator_stack = alloc(Node *, 2048);
    u32 operator_count = 0;


    Node *output_stack[8] = {};
    u32 output_count = 0;

    Token last = {};
    Token next = next_token(lex);

    while (next.kind != Token_Kind::END)
    {
        assert(next.kind != Token_Kind::INVALID);

        switch (next.kind)
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

                    // Node_Kind node_op = {};
                    // if (unary)
                    // {
                        
                    //     if (op->kind == Token_Kind::PLUS) node_op = Node_Kind::POSITIVE;
                    //     else if (op.kind == Token_Kind::MINUS) node_op = Node_Kind::NEGATE;
                    //     else assert(false);
                    // }
                    // else /*if binary*/
                    // {

                    //     if (op.kind == Token_Kind::PLUS) node_op = Node_Kind::ADD;
                    //     else if (op.kind == Token_Kind::MINUS) node_op = Node_Kind::SUB;
                    //     else if (op.kind == Token_Kind::STAR) node_op = Node_Kind::MUL;
                    //     else if (op.kind == Token_Kind::SLASH) node_op = Node_Kind::DIV;
                    //     else if (op.kind == Token_Kind::CARET) node_op = Node_Kind::POW;
                    //     else assert(false);
                    // }

                    const Operator_Data *p_next = g_data + node_kind_from_token(next, last);
                    const Operator_Data *p_op = g_data + op->kind;

                    if (p_next->precendence < p_op->precendence || 
                        (p_next->precendence == p_op->precendence && !p_next->right_associative)
                    )
                    {
                        
                        if (op->kind == Node_Kind::POSITIVE || op->kind == Node_Kind::NEGATE)
                        {
                            Node *node = alloc(Node, 1);
                            init_unary_node(node, op->kind, output_stack, &output_count);
                            output_stack[output_count++] = node;
                            operator_count -= 1;
                        }
                        else if (
                            op->kind == Node_Kind::ADD ||
                            op->kind == Node_Kind::SUB ||
                            op->kind == Node_Kind::MUL ||
                            op->kind == Node_Kind::DIV ||
                            op->kind == Node_Kind::POW
                        )
                        {
                            Node *node = alloc(Node, 1);
                            init_bin_node(node, op->kind, output_stack, &output_count);
                            output_stack[output_count++] = node;
                            operator_count -= 1;
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: expected 1 or 2 expressions but got %u\n", operator_count);
                            print_error_here(lex->data, lex->data_length, next.data_index);
                            return nullptr;
                        }
                    }
                    else
                    {
                        break;
                    }

                }
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, last);
                operator_stack[operator_count++] = node; 


            } break;
            case Token_Kind::OPEN_PAREN:
            {
                Node *node = alloc(Node, 1);
                node->kind = node_kind_from_token(next, last);
                operator_stack[operator_count++] = node; 
            } break;
            case Token_Kind::CLOSE_PAREN:
            {
                if (operator_count == 0)
                {
                    fprintf(stderr, "ERROR: unmatched parenthesis\n");
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
                        return nullptr;
                    }


                    if (op->kind == Node_Kind::POSITIVE || op->kind == Node_Kind::NEGATE)
                    {
                        Node *node = alloc(Node, 1);
                        init_unary_node(node, op->kind, output_stack, &output_count);
                        output_stack[output_count++] = node;
                    }
                    else if (
                        op->kind == Node_Kind::ADD ||
                        op->kind == Node_Kind::SUB ||
                        op->kind == Node_Kind::MUL ||
                        op->kind == Node_Kind::DIV ||
                        op->kind == Node_Kind::POW
                    )
                    {
                        Node *node = alloc(Node, 1);
                        init_bin_node(node, op->kind, output_stack, &output_count);
                        output_stack[output_count++] = node;
                    }
                    else
                    {
                        fprintf(stderr, "ERROR: expected 1 or 2 expressions but got %u\n", operator_count);
                        print_error_here(lex->data, lex->data_length, next.data_index);
                        return nullptr;
                    }
                }

            } break;
            case Token_Kind::NUMBER:
            {
                Node *node = alloc(Node, 1);
                node->kind = Node_Kind::NUMBER;
                node->num = atof(lex->data + next.data_index);
                output_stack[output_count++] = node;
            } break;

            default: 
                assert(false);
        }                
        last = next;
        next = next_token(lex);
    }
    while (operator_count > 0)
    {
        Node *op = operator_stack[--operator_count];

        if (op->kind == Node_Kind::OPEN_PAREN)
        {
            fprintf(stderr, "ERROR: Missing closing parenthesis\n");
            return nullptr;
        }

        if (op->kind == Node_Kind::POSITIVE || op->kind == Node_Kind::NEGATE)
        {
            Node *node = alloc(Node, 1);
            init_unary_node(node, op->kind, output_stack, &output_count);
            output_stack[output_count++] = node;
        }
        else if (
            op->kind == Node_Kind::ADD ||
            op->kind == Node_Kind::SUB ||
            op->kind == Node_Kind::MUL ||
            op->kind == Node_Kind::DIV ||
            op->kind == Node_Kind::POW
        )
        {
            Node *node = alloc(Node, 1);
            init_bin_node(node, op->kind, output_stack, &output_count);
            output_stack[output_count++] = node;
        }
        else
        {
            fprintf(stderr, "ERROR: expected 1 or 2 expressions but got %u\n", operator_count);
            print_error_here(lex->data, lex->data_length, next.data_index);
            return nullptr;
        }
    }

    return output_stack[0];
}


int main(void)
{
    // char input[256];
    // fgets(input, sizeof(input), stdin)

    // usize input_len = strlen(input) - 1;

    char input[] = "----5*(---5+5)/---5";
    usize input_len = strlen(input);

    

    Lexer lexer;

    if (tokenize(&lexer, input, (u32) input_len))
    {
        exit(1);
    }

    Node *tree = parse_expression(&lexer);
    if (tree == nullptr)
    {
        exit(1);
    }


    graphviz_from_tree(tree);


    return 0;
}
