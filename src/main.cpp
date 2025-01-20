#include <stdio.h>

#include <raylib.h>
#include <string>

#include "common.cpp"
#include "arena.cpp"

#include "generated.cpp"


struct Token {
    TokenType type;
    u64 start;
    // end exclusive
    u64 end;
};

struct String {
    u8 *dat;
    u64 count;
};

struct Lexer {
    Token *tokens;
    u64 token_count;

    String src;

    u64 iter;
};








String str_from_cstr(const char *cstr) {

    u64 len = strlen(cstr); 

    u8 *dat = (u8 *)malloc(len);

    memcpy(dat, cstr, len);

    return String {dat, len};
}


void insert_token(Lexer *lex, Token t) {
    lex->tokens[lex->token_count++] = t;
}


bool is_whitespace(u8 c) {
    switch (c) {
        case ' ': return true;
        case '\r': return true;
        case '\n': return true;
        case '\t': return true;
    }
    return false;
}


bool is_digit(u8 c) {
    return c >= '0' && c <= '9';
}

bool is_alpha(u8 c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void tokenize(Lexer *lex, String src) {
    lex->tokens = (Token *)calloc(4096, sizeof(*lex->tokens));
    lex->src = src;

    while (lex->iter < src.count) {
        if (is_whitespace(src.dat[lex->iter])) {
            lex->iter += 1;
            continue;
        }

        if (is_digit(src.dat[lex->iter])) {
            u64 index_start = lex->iter;
            lex->iter += 1;
            while (is_digit(src.dat[lex->iter])) {
                lex->iter += 1;
            }
            if (is_alpha(src.dat[lex->iter])) {
                todo();
            }
            u64 index_end = lex->iter;

            Token t = Token {TOKEN_NUMBER, index_start, index_end};
            insert_token(lex, t);
        }

        // 1 character tokens
        {
            switch (src.dat[lex->iter]) {
                case '+': insert_token(lex, Token {TOKEN_PLUS, lex->iter, lex->iter + 1}); lex->iter += 1; break;
                case '-': insert_token(lex, Token {TOKEN_MINUS, lex->iter, lex->iter + 1}); lex->iter += 1; break;
                case '*': insert_token(lex, Token {TOKEN_STAR, lex->iter, lex->iter + 1}); lex->iter += 1; break;
                case '/': insert_token(lex, Token {TOKEN_SLASH, lex->iter, lex->iter + 1}); lex->iter += 1; break;
            }
        }
    }


}


struct Node {
    NodeType type;
    union {
        f64 num;
    };

    u64 token_index;

    Node **nodes;
    u64 node_count;
};

struct Parse_Context {
    Node *node_stack[32];
    u64 stack_count;

    Node *op_stack[32];
    u64 op_count;

    Lexer *lex;
    u64 iter;
};

bool is_token(Parse_Context *ctx, TokenType t, s64 offset) {
    if ((s64)ctx->iter + offset < 0) return false;
    if ((s64)ctx->iter + offset > (s64)ctx->lex->token_count) return false;

    return ctx->lex->tokens[(s64)ctx->iter + offset].type == t; 
}

u64 consume(Parse_Context *ctx) {
    assert(ctx->iter < ctx->lex->token_count);
    u64 token_index = ctx->iter;
    ctx->iter += 1;
    return token_index;
}

void push_node(Parse_Context *ctx, Node *n) {
    assert(ctx->stack_count < ARRAY_SIZE(ctx->node_stack));
    ctx->node_stack[ctx->stack_count++] = n;
}

Node *pop_node(Parse_Context *ctx) {
    assert(ctx->stack_count > 0);
    return ctx->node_stack[--ctx->stack_count];
}

void push_op(Parse_Context *ctx, Node *n) {
    assert(ctx->op_count < ARRAY_SIZE(ctx->op_stack));
    ctx->op_stack[ctx->op_count++] = n;
}

Node *pop_op(Parse_Context *ctx) {
    assert(ctx->op_count > 0);
    return ctx->op_stack[--ctx->op_count];
}

Node *make_number(Parse_Context *ctx, u64 token_index) {
    Node *n = (Node *)calloc(1, sizeof(*n));

    Token t = ctx->lex->tokens[token_index];

    n->type = NODE_NUMBER;
    n->token_index = token_index;
    n->num = atof((const char *)ctx->lex->src.dat + t.start);
    return n;
}

bool is_binop(Parse_Context *ctx, s64 offset) {
    if (is_token(ctx, TOKEN_PLUS, offset)) return true;
    if (is_token(ctx, TOKEN_MINUS, offset)) return true;
    if (is_token(ctx, TOKEN_STAR, offset)) return true;
    if (is_token(ctx, TOKEN_SLASH, offset)) return true;


    return false;
}

s64 get_precedence(NodeType t) {
    switch (t) {
        case NODE_INVALID:
        case NODE_NUMBER:
            assert(false && "cannot get precedence of non operator");
        case NODE_ADD: return 1;
        case NODE_SUB: return 1;
        case NODE_MUL: return 2;
        case NODE_DIV: return 2;
        case NODE_UNARYADD: return 100;
        case NODE_UNARYSUB: return 101;
    }
    assert(false && "cannot get precedence of non operator");
    return 0;
}

bool is_left_associative(NodeType t) {
    switch (t) {
        case NODE_INVALID:
        case NODE_NUMBER:
            assert(false && "cannot get associativity of non operator");
        case NODE_ADD: return true;
        case NODE_SUB: return true;
        case NODE_MUL: return true;
        case NODE_DIV: return true;
        case NODE_UNARYADD: return false;
        case NODE_UNARYSUB: return false;
    }
    assert(false && "cannot get associativity of non operator");
    return false;
}

Node *make_node_from_stacks(Parse_Context *ctx) {
    Node *top = pop_op(ctx);


    switch (top->type) {
        case NODE_INVALID:
        case NODE_NUMBER: 
            assert(false);
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_UNARYADD:
        case NODE_UNARYSUB: {
            for (u64 i = top->node_count; i-- > 0;) {
                top->nodes[i] = pop_node(ctx);
            }
        } break;
    }

    return top;
}

void make_all_nodes_from_operator_ctx(Parse_Context *ctx) {
    while (ctx->op_count > 0) {
        Node *n = make_node_from_stacks(ctx);
        push_node(ctx, n);
    }
}

void parse_expr(Parse_Context *ctx) {

    while (ctx->iter < ctx->lex->token_count) {

        if (ctx->op_count >= 2) {
            s64 p_top = get_precedence(ctx->op_stack[ctx->op_count - 1]->type);
            bool left_associative_top = is_left_associative(ctx->op_stack[ctx->op_count - 1]->type);

            s64 p_prev = get_precedence(ctx->op_stack[ctx->op_count - 2]->type);




            if (p_top < p_prev || (p_top == p_prev && left_associative_top)) {

                // ignore top
                {
                    Node *top = pop_op(ctx);
                    make_all_nodes_from_operator_ctx(ctx);
                    push_op(ctx, top);
                }
            }

        }



        if (is_token(ctx, TOKEN_NUMBER, 0)) {
            u64 token_index = consume(ctx);
            Node *n = make_number(ctx, token_index);
            
            push_node(ctx, n);
        } else if (is_token(ctx, TOKEN_PLUS, 0)) {

            if (is_binop(ctx, -1)) {
                u64 plus_index = consume(ctx);
                // unary add

                Node *unary_add = (Node *)calloc(1, sizeof(*unary_add));
                unary_add->token_index = plus_index;
                unary_add->type = NODE_UNARYADD;
                unary_add->node_count = 1;
                unary_add->nodes = (Node **)calloc(unary_add->node_count, sizeof(Node *));

                push_op(ctx, unary_add);
            } else {
                u64 plus_index = consume(ctx);
                
                Node *add = (Node *)calloc(1, sizeof(*add));
                add->token_index = plus_index;
                add->type = NODE_ADD;
                add->node_count = 2;
                add->nodes = (Node **)calloc(add->node_count, sizeof(Node *));

                push_op(ctx, add);
            }
        } else if (is_token(ctx, TOKEN_MINUS, 0)) {
            if (is_binop(ctx, -1)) {
                u64 plus_index = consume(ctx);
                // unary sub

                Node *unary_sub = (Node *)calloc(1, sizeof(*unary_sub));
                unary_sub->token_index = plus_index;
                unary_sub->type = NODE_UNARYSUB;
                unary_sub->node_count = 1;
                unary_sub->nodes = (Node **)calloc(unary_sub->node_count, sizeof(Node *));

                push_op(ctx, unary_sub);
            } else {
                u64 minus_index = consume(ctx);
                
                Node *sub = (Node *)calloc(1, sizeof(*sub));
                sub->token_index = minus_index;
                sub->type = NODE_SUB;
                sub->node_count = 2;
                sub->nodes = (Node **)calloc(sub->node_count, sizeof(Node *));

                push_op(ctx, sub);
            }
        } else if (is_token(ctx, TOKEN_STAR, 0)) {
            u64 star_index = consume(ctx);
            
            Node *mul = (Node *)calloc(1, sizeof(*mul));
            mul->token_index = star_index;
            mul->type = NODE_MUL;
            mul->node_count = 2;
            mul->nodes = (Node **)calloc(mul->node_count, sizeof(Node *));

            push_op(ctx, mul);
        } else if (is_token(ctx, TOKEN_SLASH, 0)) {
            u64 slash_index = consume(ctx);
            
            Node *div = (Node *)calloc(1, sizeof(*div));
            div->token_index = slash_index;
            div->type = NODE_DIV;
            div->node_count = 2;
            div->nodes = (Node **)calloc(div->node_count, sizeof(Node *));

            push_op(ctx, div);
        }
    }
    make_all_nodes_from_operator_ctx(ctx);
}

void parse_statement(Parse_Context *ctx) {
    parse_expr(ctx);
}


Node *parse(Parse_Context *ctx) {


    while (ctx->iter < ctx->lex->token_count) {
        parse_statement(ctx);
    }


    return ctx->node_stack[0];
}


void display_node(Parse_Context *ctx, Node *tree) {
    FILE *f = fopen("./input.dot", "wb");
    fprintf(f, "graph G {\n");

    Node **stack = (Node **)calloc(4096, sizeof(*stack));
    u64 count = 0; 

    stack[count++] = tree;

    while (count != 0) {
        Node *top = stack[--count];
        Token t = ctx->lex->tokens[top->token_index];
        u64 len = t.end - t.start;
        fprintf(f, "n%llu [label=\"%.*s\"]\n", (u64)top, (int)len, ctx->lex->src.dat + t.start);

        for (u64 i = 0; i < top->node_count; ++i) {
            todo();
        }

    }


    fprintf(f, "}");
    fclose(f);
}



int main(void) {

    Lexer lex = {};

    tokenize(&lex, str_from_cstr("1 - 2 * 3 / 4"));

    Parse_Context context = {};
    context.lex = &lex;

    Node *tree = parse(&context);

    display_node(&context, tree);

    return 0;

    InitWindow(1000, 1000, "Para");

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();


        if (IsWindowResized()) {
            printf("resized window\n");
        }

        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context

    return 0;
}



