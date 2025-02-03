#include <stdio.h>

#include <raylib.h>
#include <string>

#include "common.cpp"
#include "arena.cpp"

#include "generated.cpp"



/*

TODO:
add ability to actually run functions and expressions
separate work thread from ui thread for more heavy tasks for example, running functions in the graph view. 

make ui scale correctly according to window size
chained equality (checking for equality at every step in some list of expressions)

make panes resizable
add ability to snap with keyboard hotkeys instead of only mouse
add more math operators/functions like integrals, derivatives, sum.

add undo system
tooltips to evaluate expressions partially (maybe)

use arenas for memory allocation in the lexer/parser/compiler
improve color theme for the ui
add graph viewer similar to desmos
add snapping for panes


*/


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
        } else if (is_digit(src.dat[lex->iter])) {
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
        } else if (src.dat[lex->iter] == '+') { // 1 character tokens
            insert_token(lex, Token {TOKEN_PLUS, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == '-') {
            insert_token(lex, Token {TOKEN_MINUS, lex->iter, lex->iter + 1});
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == '*') {
            insert_token(lex, Token {TOKEN_STAR, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == '/') {
            insert_token(lex, Token {TOKEN_SLASH, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == '(') {
            insert_token(lex, Token {TOKEN_OPENPAREN, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == ')') {
            insert_token(lex, Token {TOKEN_CLOSEPAREN, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == ',') {
            insert_token(lex, Token {TOKEN_COMMA, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (is_alpha(src.dat[lex->iter])) {

            u64 index_start = lex->iter;
            lex->iter += 1;
            while (is_alpha(src.dat[lex->iter]) || is_digit(src.dat[lex->iter])) {
                lex->iter += 1;
            }
            u64 index_end = lex->iter;
            Token t = Token {TOKEN_IDENTIFIER, index_start, index_end};

            insert_token(lex, t);
        } else {
            printf("char %c\n", src.dat[lex->iter]);
            todo();
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

    Node *root;
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
        case NODE_FUNCTION:
        case NODE_VARIABLE:
            assert(false && "cannot get precedence of non operator");
        case NODE_ADD: return 1;
        case NODE_SUB: return 1;
        case NODE_MUL: return 2;
        case NODE_DIV: return 2;
        case NODE_UNARYADD: return 100;
        case NODE_UNARYSUB: return 101;
        case NODE_OPENPAREN: return 0;
    }
    assert(false && "cannot get precedence of non operator");
    return 0;
}

bool is_left_associative(NodeType t) {
    switch (t) {
        case NODE_INVALID:
        case NODE_NUMBER:
        case NODE_FUNCTION:
        case NODE_VARIABLE:
            assert(false && "cannot get associativity of non operator");
        case NODE_ADD: return true;
        case NODE_SUB: return true;
        case NODE_MUL: return true;
        case NODE_DIV: return true;
        case NODE_UNARYADD: return false;
        case NODE_UNARYSUB: return false;
        case NODE_OPENPAREN: return true;
    }
    return false;
}

Node *make_node_from_stacks(Parse_Context *ctx) {
    Node *top = pop_op(ctx);


    switch (top->type) {
        case NODE_INVALID:
        case NODE_NUMBER: 
        case NODE_OPENPAREN:
        case NODE_FUNCTION:
        case NODE_VARIABLE:
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
    while (ctx->op_count > 0 && ctx->op_stack[ctx->op_count - 1]->type != NODE_OPENPAREN) {
        Node *n = make_node_from_stacks(ctx);
        push_node(ctx, n);
    }
}

Node *make_function_call(Parse_Context *ctx, u64 token_index, u64 arg_count) {
    Node *func = (Node *)calloc(1, sizeof(*func));

    func->type = NODE_FUNCTION;
    func->token_index = token_index;
    func->node_count = arg_count;

    func->nodes = (Node **)calloc(func->node_count, sizeof(Node *));

    for (u64 i = func->node_count; i-- > 0;) {
        func->nodes[i] = pop_node(ctx);
    }


    return func;
}

void parse_expr(Parse_Context *ctx, u64 stop_token_types) {

    while (ctx->iter < ctx->lex->token_count) {

        if (ctx->lex->tokens[ctx->iter].type & stop_token_types) {
            break;
        }


        if (ctx->op_count >= 2 && ctx->op_stack[ctx->op_count - 1]->type != NODE_OPENPAREN) {
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

        if (is_token(ctx, TOKEN_IDENTIFIER, 0)) {

            u64 id_index = consume(ctx);

            if (is_token(ctx, TOKEN_OPENPAREN, 0)) {

                consume(ctx);

                u64 func_close_index = 0;
                {
                    u64 unclosed_paren_count = 1;
                    for (u64 i = ctx->iter; i < ctx->lex->token_count; ++i) {

                        if (ctx->lex->tokens[i].type == TOKEN_OPENPAREN) unclosed_paren_count += 1;

                        if (ctx->lex->tokens[i].type == TOKEN_CLOSEPAREN) {
                            if (unclosed_paren_count == 1) {
                                func_close_index = i;
                                goto skip;
                            } else {
                                unclosed_paren_count -= 1;
                            }
                        }
                    }
                    todo();
                }
                skip:;



                u64 arg_count = 0;
                {
                    u64 before_count = ctx->lex->token_count;
                    while (!is_token(ctx, TOKEN_CLOSEPAREN, 0)) {
                        
                        ctx->lex->token_count = func_close_index;
                        parse_expr(ctx, TOKEN_COMMA);

                        arg_count += 1;
                        if (is_token(ctx, TOKEN_CLOSEPAREN, 0)) {
                            break;
                        } else if (is_token(ctx, TOKEN_COMMA, 0)) {
                            consume(ctx);
                        } else {
                            // report error 
                            todo();
                        }
                    }
                    ctx->lex->token_count = before_count;
                    consume(ctx);
                }



                Node *function_call = make_function_call(ctx, id_index, arg_count); 
                push_node(ctx, function_call);
            } else {
                // var
                todo();
            }
        } else if (is_token(ctx, TOKEN_NUMBER, 0)) {
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
        } else if (is_token(ctx, TOKEN_OPENPAREN, 0)) {
            u64 open_paren_index = consume(ctx);

            Node *open_paren = (Node *)calloc(1, sizeof(*open_paren));

            open_paren->type = NODE_OPENPAREN;
            open_paren->token_index = open_paren_index;

            push_op(ctx, open_paren);
        } else if (is_token(ctx, TOKEN_CLOSEPAREN, 0)) {
            consume(ctx);

            while (true) {
                if (ctx->op_count == 0) {
                    // report mismatched parenthesis error here
                    todo();
                }
                if (ctx->op_stack[ctx->op_count - 1]->type == NODE_OPENPAREN) {
                    pop_op(ctx);
                    break;
                }
                Node *n = make_node_from_stacks(ctx); 
                push_node(ctx, n);
            }

        } else {
            assert(false);
        }
    }
    make_all_nodes_from_operator_ctx(ctx);
}

void parse_statement(Parse_Context *ctx) {
    parse_expr(ctx, TOKEN_INVALID);
}


void parse(Parse_Context *ctx) {

    while (ctx->iter < ctx->lex->token_count) {
        parse_statement(ctx);
    }

    ctx->root = ctx->node_stack[0]; 
}


void graphviz_out(Parse_Context *ctx) {
    FILE *f = fopen("./input.dot", "wb");
    fprintf(f, "graph G {\n");

    Node **stack = (Node **)calloc(4096, sizeof(*stack));
    u64 count = 0; 

    stack[count++] = ctx->root;

    while (count != 0) {
        Node *top = stack[--count];
        Token t = ctx->lex->tokens[top->token_index];
        u64 len = t.end - t.start;
        fprintf(f, "n%llu [label=\"%.*s\"]\n", (u64)top, (int)len, ctx->lex->src.dat + t.start);

        for (u64 i = 0; i < top->node_count; ++i) {
            fprintf(f, "n%llu -- n%llu\n", (u64)top, (u64)top->nodes[i]);
            stack[count++] = top->nodes[i];
        }

    }


    fprintf(f, "}");
    fclose(f);
}

f32 execute_tree(Node *n) {
    f32 r = 0;
    switch (n->type) {
        case NODE_INVALID: todo();
        case NODE_NUMBER: r = n->num; break;
        case NODE_FUNCTION: todo();
        case NODE_VARIABLE: todo();
        case NODE_ADD: r = execute_tree(n->nodes[0]) + execute_tree(n->nodes[1]); break;
        case NODE_SUB: r = execute_tree(n->nodes[0]) - execute_tree(n->nodes[1]); break;
        case NODE_MUL: r = execute_tree(n->nodes[0]) * execute_tree(n->nodes[1]); break;
        case NODE_DIV: r = execute_tree(n->nodes[0]) / execute_tree(n->nodes[1]); break;
        case NODE_UNARYADD: r = execute_tree(n->nodes[0]); break;
        case NODE_UNARYSUB: r = -execute_tree(n->nodes[0]); break;
        case NODE_OPENPAREN: todo();
    }

    return r;
}



// UI System
bool mouse_collides(f32 x, f32 y, f32 w, f32 h) {
    f32 mx = (f32)GetMouseX();
    f32 my = (f32)GetMouseY();

    bool x_intercept = mx >= x && mx <= x + w;
    bool y_intercept = my >= y && my <= y + h;

    return x_intercept && y_intercept;
}

struct Pane {
    f32 x, y;
    f32 w, h;
};



void shift_right_resize(char *buf, u64 *count, u64 start, u64 amount) {

    for (u64 k = 0; k < amount; ++k) {
        *count += 1;
        // end - 1 to skip last element remember i-- also decrements at first iteration
        for (u64 i = *count - 1; i-- > start;) {
            buf[i + 1] = buf[i];
        } 
    }
}

void shift_left_resize(char *buf, u64 *count, u64 start, u64 amount) {
    assert(*count >= amount);
    for (u64 k = 0; k < amount; ++k) {
        for (u64 i = start; i < *count - 1; ++i) {
            buf[i] = buf[i + 1];
        } 
        *count -= 1;

    }
    // end - 1 to not go out of bounds
}



struct UI_State {

    bool active;
    u64 active_id;


    bool dragging;
    u64 drag_id;
    f32 drag_x_offset;
    f32 drag_y_offset;


    bool text_cursor;
    u64 cursor_pos;


    bool selecting;
    u64 selection_anchor;
    u64 selection_start;
    u64 selection_end;

};





int main(void) {

    Lexer lex = {};

    const char *src = "(6 * 5 + 4) * 4 * 3 * 2 * 1";

    tokenize(&lex, str_from_cstr(src));

    Parse_Context context = {};
    context.lex = &lex;

    parse(&context);

    graphviz_out(&context);

    f32 r = execute_tree(context.root);

    int screen_w = 1366;
    int screen_h = 768;


    InitWindow(screen_w, screen_h, "Para");

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);


    Pane p1 = {
        0, 0,
        400, 50,
    };

    Pane p2 = {
        1366-800, 0,
        800, 400,
    };


    char text_buf[5][64] = {};
    u64 text_count[5] = {};

    UI_State ui = {};


    while (!WindowShouldClose()) {
        int mx = GetMouseX();
        int my = GetMouseY();


        int chars_pressed[16];
        u64 char_count = 0;
        while (true) {
            int tmp = GetCharPressed();
            if (tmp == 0) break;

            chars_pressed[char_count++] = tmp;
        }

        bool ctrl_key_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        bool shift_key_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        int keys_pressed[16];
        u64 key_count = 0;
        while (true) {
            int tmp = GetKeyPressed();
            if (tmp == 0) break;

            keys_pressed[key_count++] = tmp;
        }

        u64 pane_id = -1;
        f32 h_offset;

        BeginDrawing();

#define TEXT_INPUT_FONT_SIZE 20
#define TEXT_INPUT_HEIGHT 35
#define TEXT_INPUT_MARGIN 5
#define TEXT_INPUT_CURSOR_COLOR BLACK
#define TEXT_INPUT_SELECTION_COLOR BLUE
#define DRAG_BAR_HEIGHT 15

        pane_id += 1;
        h_offset = p1.y;
        {
            if (ui.dragging && ui.drag_id == pane_id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                ui.dragging = false;
            }

            if (ui.dragging && ui.drag_id == pane_id) {
                p1.x = (f32)mx - ui.drag_x_offset;
                p1.y = (f32)my - ui.drag_y_offset;
            }

            {
                if (mouse_collides(p1.x, h_offset, p1.w, DRAG_BAR_HEIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ui.dragging = true;
                    ui.drag_id = pane_id;
                    ui.drag_x_offset = (f32)mx - p1.x;
                    ui.drag_y_offset = (f32)my - p1.y;
                }
                DrawRectangleV(Vector2 {p1.x, h_offset}, Vector2 {p1.w, DRAG_BAR_HEIGHT}, GREEN);
            }
            h_offset += DRAG_BAR_HEIGHT;


            // text input
            for (u64 i = 0; i < 5; ++i) {

                if (mouse_collides(p1.x, h_offset, p1.w, TEXT_INPUT_HEIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (ui.active && ui.active_id == i && ui.text_cursor) {
                        // TODO: set text cursor relative to mouse cursor
                        todo();
                    } else {
                        // TODO: set text cursor relative to mouse cursor
                        ui.selecting = false;
                        ui.text_cursor = true;
                        ui.cursor_pos = 0;
                        ui.active_id = i;
                        ui.active = true;
                    }
                }

                if (ui.active && ui.active_id == i && ui.text_cursor) {
                    
                    for (u64 j = 0; j < key_count; ++j) {

                        u64 prev_cursor_pos = ui.cursor_pos;

                        if (keys_pressed[j] == KEY_BACKSPACE) {

                            if (text_count[i] > 0) {
                                if (ui.selecting) {
                                    ui.selecting = false;

                                    shift_left_resize(text_buf[i], text_count + i, ui.selection_start, ui.selection_end - ui.selection_start);
                                    ui.cursor_pos = ui.selection_start;
                                } else {
                                    if (ui.cursor_pos > 0) {
                                        shift_left_resize(text_buf[i], text_count + i, ui.cursor_pos - 1, 1);
                                        ui.cursor_pos -= 1;
                                    }
                                }
                            }

                        } else if (keys_pressed[j] == KEY_DELETE) {

                            if (text_count[i] > 0) {
                                if (ui.selecting) {
                                    ui.selecting = false;

                                    shift_left_resize(text_buf[i], text_count + i, ui.selection_start, ui.selection_end - ui.selection_start);
                                    ui.cursor_pos = ui.selection_start;
                                } else {
                                    if (ui.cursor_pos < text_count[i]) {
                                        shift_left_resize(text_buf[i], text_count + i, ui.cursor_pos, 1);
                                    }
                                }
                            }

                        } else if (keys_pressed[j] == KEY_A) {
                            if (text_count[i] > 0 && ctrl_key_down) {
                                ui.selecting = true;
                                ui.cursor_pos = 0;
                                ui.selection_anchor = text_count[i];
                            }
                        } else if (keys_pressed[j] == KEY_X) {
                            if (ui.selecting && ctrl_key_down) {
                                ui.selecting = false;
                                char tmp_buf[96];
                                snprintf(tmp_buf, sizeof(tmp_buf), "%.*s", (int)(ui.selection_end - ui.selection_start), ui.selection_start + text_buf[i]);
                                SetClipboardText(tmp_buf);
                                shift_left_resize(text_buf[i], text_count + i, ui.selection_start, ui.selection_end - ui.selection_start);
                            }
                        } else if (keys_pressed[j] == KEY_C) {
                            if (ui.selecting && ctrl_key_down) {
                                char tmp_buf[96];
                                snprintf(tmp_buf, sizeof(tmp_buf), "%.*s", (int)(ui.selection_end - ui.selection_start), ui.selection_start + text_buf[i]);
                                SetClipboardText(tmp_buf);
                            }
                        } else if (keys_pressed[j] == KEY_V) {
                            if (ctrl_key_down) {
                                const char *text = GetClipboardText();
                                u64 len = strlen(text);

                                if (len + text_count[i] - (ui.selection_end - ui.selection_start) > ARRAY_SIZE(text_buf[i])) {
                                    // pasting clipboard would overflow
                                    todo();
                                } else {
                                    if (ui.selecting) {
                                        ui.selecting = false;
                                        shift_left_resize(text_buf[i], text_count + i, ui.selection_start, ui.selection_end - ui.selection_start);
                                        ui.cursor_pos = ui.selection_start;
                                    }
                                    shift_right_resize(text_buf[i], text_count + i, ui.cursor_pos, len);
                                    memcpy(text_buf[i] + ui.cursor_pos, text, len);
                                    ui.cursor_pos += len;
                                }
                                
                            }
                        } else if (keys_pressed[j] == KEY_HOME) {
                            if (text_count[i] > 0) {
                                if (!ui.selecting && shift_key_down) {
                                    ui.selecting = true;
                                    ui.selection_anchor = prev_cursor_pos;
                                }
                                if (ui.selecting && !shift_key_down) {
                                    ui.selecting = false;
                                }
                                ui.cursor_pos = 0;
                            }
                        } else if (keys_pressed[j] == KEY_END) {
                            if (text_count[i] > 0) {
                                if (!ui.selecting && shift_key_down) {
                                    ui.selecting = true;
                                    ui.selection_anchor = prev_cursor_pos;
                                }
                                if (ui.selecting && !shift_key_down) {
                                    ui.selecting = false;
                                }
                                ui.cursor_pos = text_count[i];
                            }
                        } else if (keys_pressed[j] == KEY_LEFT) {
                            if (text_count[i] > 0 && ui.cursor_pos > 0) {

                                if (!ui.selecting && shift_key_down) {
                                    ui.selecting = true;
                                    ui.selection_anchor = prev_cursor_pos;
                                }
                                if (ui.selecting && !shift_key_down) {
                                    ui.selecting = false;
                                }

                                if (ctrl_key_down) {
                                    if (is_whitespace(text_buf[i][ui.cursor_pos - 1])) {
                                        while (ui.cursor_pos > 0 && is_whitespace(text_buf[i][ui.cursor_pos - 1])) {
                                            ui.cursor_pos -= 1;
                                        }
                                        while (ui.cursor_pos > 0 && !is_whitespace(text_buf[i][ui.cursor_pos - 1])) {
                                            ui.cursor_pos -= 1;
                                        }
                                    } else {
                                        while (ui.cursor_pos > 0 && !is_whitespace(text_buf[i][ui.cursor_pos - 1])) {
                                            ui.cursor_pos -= 1;
                                        }
                                    }
                                } else {
                                    ui.cursor_pos -= 1;
                                }
                            }
                                        
                        } else if (keys_pressed[j] == KEY_RIGHT) {
                            if (text_count[i] > 0 && ui.cursor_pos < text_count[i]) {

                                if (!ui.selecting && shift_key_down) {
                                    ui.selecting = true;
                                    ui.selection_anchor = prev_cursor_pos;
                                }
                                if (ui.selecting && !shift_key_down) {
                                    ui.selecting = false;
                                }

                                if (ctrl_key_down) {
                                    if (is_whitespace(text_buf[i][ui.cursor_pos])) {
                                        while (ui.cursor_pos < text_count[i] && is_whitespace(text_buf[i][ui.cursor_pos])) {
                                            ui.cursor_pos += 1;
                                        }
                                        while (ui.cursor_pos < text_count[i] && !is_whitespace(text_buf[i][ui.cursor_pos])) {
                                            ui.cursor_pos += 1;
                                        }
                                    } else {
                                        while (ui.cursor_pos < text_count[i] && !is_whitespace(text_buf[i][ui.cursor_pos])) {
                                            ui.cursor_pos += 1;
                                        }
                                    }
                                } else {
                                    ui.cursor_pos += 1;
                                }
                            }
                        }
                    }

                    if (ui.selecting) {
                        if (ui.selection_anchor < ui.cursor_pos) {
                            ui.selection_start = ui.selection_anchor;
                            ui.selection_end = ui.cursor_pos;
                        } else {
                            ui.selection_start = ui.cursor_pos;
                            ui.selection_end = ui.selection_anchor;
                        }
                    }

                    if (ui.selecting && ui.cursor_pos == ui.selection_anchor) {
                        ui.selecting = false;
                    }



                    // text cursor input
                    for (u64 j = 0; j < char_count && text_count[i] < ARRAY_SIZE(text_buf[i]) - 1; ++j) {

                        if (ui.selecting) {
                            ui.selecting = false;

                            shift_left_resize(text_buf[i], text_count + i, ui.selection_start, ui.selection_end - ui.selection_start);
                            ui.cursor_pos = ui.selection_start;
                        }
                        
                        shift_right_resize(text_buf[i], text_count + i, ui.cursor_pos, 1);
                        text_buf[i][ui.cursor_pos] = (char)chars_pressed[j];
                        ui.cursor_pos += 1;
                    }
                }

                Color color;
                if (i % 2 == 0) color = DARKGREEN;
                else color = LIME;
                
                DrawRectangleV(Vector2 {p1.x, h_offset}, Vector2 {p1.w, TEXT_INPUT_HEIGHT}, color);
                {
                    char tmp_buf[96];
                    snprintf(tmp_buf, sizeof(tmp_buf), "%.*s", (int)text_count[i], text_buf[i]);
                    DrawText(tmp_buf, (s32)p1.x + TEXT_INPUT_MARGIN, (s32)(h_offset) + TEXT_INPUT_FONT_SIZE / 2, TEXT_INPUT_FONT_SIZE, LIGHTGRAY);
                }

                if (ui.active && ui.active_id == i && ui.text_cursor) {
                    char tmp_buf[96];
                    snprintf(tmp_buf, sizeof(tmp_buf), "%.*s", (int)ui.cursor_pos, text_buf[i]);
                    int sz = MeasureText(tmp_buf, TEXT_INPUT_FONT_SIZE);

                    DrawRectangleV(Vector2 {p1.x + TEXT_INPUT_MARGIN + (f32)sz, h_offset + TEXT_INPUT_FONT_SIZE / 2}, Vector2 {1, TEXT_INPUT_FONT_SIZE}, TEXT_INPUT_CURSOR_COLOR);

                    if (ui.selecting) {

                        snprintf(tmp_buf, sizeof(tmp_buf), "%.*s", (int)ui.selection_start, text_buf[i]);
                        int sz = MeasureText(tmp_buf, TEXT_INPUT_FONT_SIZE);
                        
                        snprintf(tmp_buf, sizeof(tmp_buf), "%.*s", (int)(ui.selection_end - ui.selection_start), text_buf[i] + ui.selection_start);
                        int selection_sz = MeasureText(tmp_buf, TEXT_INPUT_FONT_SIZE);
                        Color a = TEXT_INPUT_SELECTION_COLOR;
                        a.a = 100;
                        DrawRectangleV(Vector2 {p1.x + TEXT_INPUT_MARGIN + (f32)sz, h_offset + TEXT_INPUT_FONT_SIZE / 2}, Vector2 {(f32)selection_sz, TEXT_INPUT_FONT_SIZE}, a);

                    }
                }

                h_offset += TEXT_INPUT_HEIGHT;
            }
        }
        pane_id += 1;
        h_offset = p2.y;
        {
            if (ui.dragging && ui.drag_id == pane_id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                ui.dragging = false;
            }


            if (ui.dragging && ui.drag_id == pane_id) {
                p2.x = (f32)mx - ui.drag_x_offset;
                p2.y = (f32)my - ui.drag_y_offset;
            }

            {
                if (mouse_collides(p2.x, h_offset, p2.w, DRAG_BAR_HEIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    ui.dragging = true;
                    ui.drag_id = pane_id;
                    ui.drag_x_offset = (f32)mx - p2.x;
                    ui.drag_y_offset = (f32)my - p2.y;
                }
                DrawRectangleV(Vector2 {p2.x, h_offset}, Vector2 {p2.w, DRAG_BAR_HEIGHT}, GREEN);
            }
            h_offset += DRAG_BAR_HEIGHT;
            {
                DrawRectangleV(Vector2 {p2.x, h_offset}, Vector2 {p2.w, p2.w}, BLACK);



            }
            h_offset += p2.w;
        }

        ClearBackground(DARKGRAY);
        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context

    return 0;
}



