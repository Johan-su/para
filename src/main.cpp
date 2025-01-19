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

            Token t = Token {TOKEN_NUMBER, index_start, index_start};
            insert_token(lex, t);
        }

        // 1 character tokens
        {
            switch (src.dat[lex->iter]) {
                case '+': insert_token(lex, Token {TOKEN_PLUS, lex->iter, lex->iter + 1}); break;
            }
            lex->iter += 1;
        }
    }
    insert_token(lex, Token {TOKEN_END, lex->iter, lex->iter + 1});


}


struct Node {
    NodeType type;
    union {
        f64 num;
    };

    u64 token_index;

    Node *nodes;
    u64 node_count;
};

struct Parse_Context {
    Node top_tree;
    Node *node_stack[512];
    u64 stack_count;


    Lexer *lex;
    u64 iter;
};

bool is_token(Parse_Context *ctx, TokenType t) {
   return ctx->lex->tokens[ctx->iter].type == t; 
}

u64 consume(Parse_Context *ctx) {
    assert(ctx->iter < ctx->lex->token_count);
    return ctx->iter;
}

void push_node(Parse_Context *ctx, Node *n) {
    assert(ctx->stack_count < ARRAY_SIZE(ctx->node_stack));
    ctx->node_stack[ctx->stack_count++] = n;
}

Node *pop_node(Parse_Context *ctx) {
    assert(ctx->stack_count > 0);
    return ctx->node_stack[--ctx->stack_count];
}

Node *make_number(Parse_Context *ctx, u64 token_index) {
    Node *n = (Node *)calloc(1, sizeof(*n));

    Token t = ctx->lex->tokens[token_index];

    n->type = NODE_NUMBER;
    n->token_index = token_index;
    n->num = atof((const char *)ctx->lex->src.dat + t.start);
}

bool is_prev_node(Parse_Context *ctx, NodeType t) {

    if (ctx->stack_count == 0) return false;
    else return ctx->node_stack[ctx->stack_count - 1]->type == t; 
}

bool is_prev_binop(Parse_Context *ctx) {
    if (is_prev_node(ctx, NODE_ADD)) return true;


    return false;
}

void parse_expr(Parse_Context *ctx) {

    if (is_token(ctx, TOKEN_NUMBER)) {
        u64 token_index = consume(ctx);
        Node *n = make_number(ctx, token_index);
        
        push_node(ctx, n);
    } else if (is_token(ctx, TOKEN_PLUS)) {
        ASFA
        todo();
    }

}

void parse_statement(Parse_Context *ctx) {
    parse_expr(ctx);
}


Node parse(Lexer *lex) {

    Parse_Context context = {};
    context.lex = lex;

    while (context.iter < lex->token_count) {
        parse_statement(&context);
    }


    return context.top_tree;
}



int main(void) {

    printf("test\n");

    Lexer lex = {};

    tokenize(&lex, str_from_cstr("1 + 1"));

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



