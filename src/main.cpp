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


enum Token_Kind
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


struct Operator_Data
{
    s64 precendence;
    bool right_associative;
};


const Operator_Data g_data[] = {
    /* INVALID */     {0, false},
    /* PLUS */        {10, false},
    /* MINUS */       {20, false},
    /* TIMES */       {30, false},
    /* DIVIDE */      {40, false},
    /* EXPONENT */    {50, true},
    /* NUMBER */      {0, false},
    /* OPEN_PAREN */  {99999, false},
    /* CLOSE_PAREN */ {0, false},
    /* END */         {0, false},
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


enum class Node_Kind 
{
    INVALID,
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



static const char *str_from_node_kind(Node_Kind kind)
{
    switch (kind)
    {
        case Node_Kind::INVALID: return "INVALID";
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



static void init_bin_node(Node *bin, Token_Kind kind, Node **stack, u32 *stack_len)
{
    if (kind == Token_Kind::PLUS) bin->kind = Node_Kind::ADD;
    else if (kind == Token_Kind::MINUS) bin->kind = Node_Kind::SUB;
    else if (kind == Token_Kind::STAR) bin->kind = Node_Kind::MUL;
    else if (kind == Token_Kind::SLASH) bin->kind = Node_Kind::DIV;
    else if (kind == Token_Kind::CARET) bin->kind = Node_Kind::POW;
    else assert(false);


    bin->right = get_sub_node(stack, stack_len);
    bin->left = get_sub_node(stack, stack_len);
}


static void init_unary_node(Node *bin, Token_Kind kind, Node **stack, u32 *stack_len)
{
    if (kind == Token_Kind::PLUS) bin->kind = Node_Kind::POSITIVE;
    else if (kind == Token_Kind::MINUS) bin->kind = Node_Kind::NEGATE;
    else assert(false);


    bin->right = get_sub_node(stack, stack_len);
}



// https://www.youtube.com/watch?v=fIPO4G42wYE
static Node *parse_expression(Lexer *lex)
{
    Token *stack = alloc(Token, 2048);
    u32 token_count = 0;


    Node *node_stack[32] = {};
    u32 node_count = 0;


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

                while (token_count != 0)
                {
                    Token op = stack[token_count - 1];
                    if (op.kind == Token_Kind::OPEN_PAREN)
                    {
                        break;
                    }
                    s64 p_next = g_data[next.kind].precendence;
                    s64 p_op = g_data[op.kind].precendence; 
                    if (p_next < p_op || (p_next == p_op && !g_data[next.kind].right_associative))
                    {

                        if (node_count == 1)
                        {
                            Node *node = alloc(Node, 1);
                            init_unary_node(node, op.kind, node_stack, &node_count);
                            node_stack[node_count++] = node;
                            token_count -= 1;
                        }
                        else if (node_count == 2)
                        {
                            Node *node = alloc(Node, 1);
                            init_bin_node(node, op.kind, node_stack, &node_count);
                            node_stack[node_count++] = node;
                            token_count -= 1;
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: expected 1 or 2 expressions but got %u\n", node_count);
                            print_error_here(lex->data, lex->data_length, op.data_index);
                            return nullptr;
                        }
                    }
                    else
                    {
                        break;
                    }

                }
                stack[token_count++] = next;


            } break;
            case Token_Kind::OPEN_PAREN:
            {
                stack[token_count++] = next;
            } break;
            case Token_Kind::CLOSE_PAREN:
            {
                if (token_count == 0)
                {
                    fprintf(stderr, "ERROR: unmatched parenthesis\n");
                    return nullptr;
                }
                while (token_count != 0)
                {
                    Token op = stack[--token_count];


                    if (op.kind == Token_Kind::OPEN_PAREN)
                    {
                        break;
                    }
                    else if (token_count == 0)
                    {
                        fprintf(stderr, "ERROR: unmatched parenthesis\n");
                        return nullptr;
                    }


                    if (node_count < 2)
                    {
                        fprintf(stderr, "ERROR: expected 1 or 2 expressions but got %u\n", node_count);
                        print_error_here(lex->data, lex->data_length, op.data_index);
                        return nullptr;
                    }
                    Node *node = alloc(Node, 1);
                    init_bin_node(node, op.kind, node_stack, &node_count);

                    node_stack[node_count++] = node;
                }

            } break;
            case Token_Kind::NUMBER:
            {
                Node *node = alloc(Node, 1);
                node->kind = Node_Kind::NUMBER;
                node->num = atof(lex->data + next.data_index);
                node_stack[node_count++] = node;
            } break;

            default: 
                assert(false);
        }                
        next = next_token(lex);
    }
    while (token_count > 0)
    {
        Token op = stack[--token_count];

        if (op.kind == Token_Kind::OPEN_PAREN)
        {
            fprintf(stderr, "ERROR: Missing closing parenthesis\n");
            return nullptr;
        }


        if (node_count < 2)
        {
            fprintf(stderr, "ERROR: expected 1 or 2 expressions but got %u\n", node_count);
            print_error_here(lex->data, lex->data_length, op.data_index);
            return nullptr;
        }

        Node *node = alloc(Node, 1);
        init_bin_node(node, op.kind, node_stack, &node_count);

        node_stack[node_count++] = node;
    }

    return node_stack[0];
}


int main(void)
{
    // char input[256];
    // fgets(input, sizeof(input), stdin)

    // usize input_len = strlen(input) - 1;


    char input[] = "++5+4+3+2+1";
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
