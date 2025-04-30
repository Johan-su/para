#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <string.h>

#include "common.h"
#include "arena.h"
#include "meta.h"
#include "string.h"

template <typename T>
struct Stack { 
    u64 count; 
    u64 cap; 
    T *dat; 
}; 
template <typename T>
void stack_init(Stack<T> *stack, u64 cap) { 
    stack->count = 0; 
    stack->cap = cap; 
    stack->dat = (T *)calloc(stack->cap, sizeof(T)); 
} 
template <typename T>
void stack_push(Stack<T> *stack, T v) { 
    if (stack->cap == 0) stack_init(stack, 1 << 14); 
    assert(stack->count < stack->cap); 
    stack->dat[stack->count++] = v; 
} 
template <typename T>
T stack_pop(Stack<T> *stack) { 
    assert(stack->count > 0); 
    return stack->dat[--stack->count]; 
} 




struct NodeTableData {
    s64 precedence;
    bool left_associative;
};

NodeTableData node_table_data[] = {
    #define X(type, precedence, is_left_associative) {precedence, is_left_associative},
    NodeDataTable(X)
    #undef X
};





struct String_Builder {
    u64 count;
    u64 max_capacity;
    u8 *data;
};

struct Token {
    TokenType type;
    u64 start;
    // end exclusive
    u64 end;
};


struct Lexer {
    Token *tokens;
    u64 token_count;

    u64 iter;
};


struct Node {
    NodeType type;
    union {
        f64 num;
    };

    u64 token_index;

    u64 node_count;
    Node **nodes;
};

struct Parser {
    Node *node_stack[128];
    u64 stack_count;

    Node *op_stack[128];
    u64 op_count;

    u64 iter;

    Node *root;
};


struct Item {
    ItemType type;
    String name;
    union {
        f64 val;
        u64 id;
    };
};




struct Bytecode {
    BytecodeType type;
};

struct Bytecode_Generator {
    Bytecode code[1 << 14];
    u64 code_count;
};

struct Error {
    u64 statement_id;

    u64 char_id;
    u64 token_id;
};

struct Scope {
    Stack<Item> items;
};

struct Interpreter {

    String src;
    Lexer lex;
    Parser ctx;
    Bytecode_Generator gen;


    Stack<Scope> scopes;




    Stack<f64> val_stack;
    Stack<Error> error_stack;
};


/*

TODO:
add ability to actually run functions and expressions
separate work thread from ui thread for more heavy tasks for example, running functions in the graph view. 

make ui scale correctly according to window size
chained equality (checking for equality at every step in some list of expressions)

make panes resizable
add snapping for panes
add ability to snap with keyboard hotkeys instead of only mouse
add more math operators/functions like integrals, derivatives, sum.

add undo system
tooltips to evaluate expressions partially (maybe)

use arenas for memory allocation in the lexer/parser/compiler
improve color theme for the ui
add graph viewer similar to desmos


*/

// String

#include <stdarg.h>
#if GCC || CLANG
__attribute__((__format__ (__printf__, 2, 3)))
#endif
String string_printf(Arena *arena, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len_ = vsnprintf(nullptr, 0, fmt, args);   
    assert(len_ >= 0);

    String s = {};
    s.count = (u64) len_;
    
    s.dat = (u8 *)arena_alloc(arena, s.count + 1);
    vsnprintf((char *)s.dat, s.count + 1, fmt, args);   
    va_end(args);

    return s;
}

bool string_equal(String a, String b) {
    if (a.count != b.count) return false;

    for (u64 i = 0; i < a.count; ++i) {
        if (a.dat[i] != b.dat[i]) return false;
    }

    return true;
}


void string_builder_init(String_Builder *sb, u64 cap) {
    sb->count = 0;
    sb->max_capacity = cap;
    sb->data = (u8 *)calloc(1, sb->max_capacity);
}

void string_builder_append(String_Builder *sb, u8 b) {
    assert(sb->count < sb->max_capacity);
    sb->data[sb->count++] = b;
}

void string_builder_concat(String_Builder *sb, String s) {
    assert(sb->count + s.count < sb->max_capacity);
    for (u64 i = 0; i < s.count; ++i) {
        sb->data[sb->count++] = s.dat[i];
    }
}

String string_builder_to_string(String_Builder *sb) {
    return String {sb->data, sb->count};
}


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

void tokenize(Interpreter *inter, String src) {
    inter->src = src;
    Lexer *lex = &inter->lex;
    lex->tokens = (Token *)calloc(4096, sizeof(*lex->tokens));

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

        } else if (src.dat[lex->iter] == ';') {
            insert_token(lex, Token {TOKEN_SEMICOLON, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 
        } else if (src.dat[lex->iter] == ':') {
            insert_token(lex, Token {TOKEN_COLON, lex->iter, lex->iter + 1}); 
            lex->iter += 1; 

        } else if (src.dat[lex->iter] == '=') {
            insert_token(lex, Token {TOKEN_EQUAL, lex->iter, lex->iter + 1}); 
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




bool is_token(Interpreter *inter, TokenType t, s64 offset) {
    if ((s64)inter->ctx.iter + offset < 0) return false;
    if ((s64)inter->ctx.iter + offset > (s64)inter->lex.token_count) return false;

    return inter->lex.tokens[(s64)inter->ctx.iter + offset].type == t; 
}

u64 consume(Interpreter *inter) {
    assert(inter->ctx.iter < inter->lex.token_count);
    u64 token_index = inter->ctx.iter;
    inter->ctx.iter += 1;
    return token_index;
}

void push_node(Parser *ctx, Node *n) {
    assert(ctx->stack_count < ARRAY_SIZE(ctx->node_stack));
    ctx->node_stack[ctx->stack_count++] = n;
}

Node *pop_node(Parser *ctx) {
    assert(ctx->stack_count > 0);
    return ctx->node_stack[--ctx->stack_count];
}

void push_op(Parser *ctx, Node *n) {
    assert(ctx->op_count < ARRAY_SIZE(ctx->op_stack));
    ctx->op_stack[ctx->op_count++] = n;
}

Node *pop_op(Parser *ctx) {
    assert(ctx->op_count > 0);
    return ctx->op_stack[--ctx->op_count];
}

Node *make_number(Interpreter *inter, u64 token_index) {
    Node *n = (Node *)calloc(1, sizeof(*n));

    Token t = inter->lex.tokens[token_index];

    n->type = NODE_NUMBER;
    n->token_index = token_index;
    n->num = atof((const char *)inter->src.dat + t.start);
    return n;
}

bool is_binop(Interpreter *inter, s64 offset) {
    if (is_token(inter, TOKEN_PLUS, offset)) return true;
    if (is_token(inter, TOKEN_MINUS, offset)) return true;
    if (is_token(inter, TOKEN_STAR, offset)) return true;
    if (is_token(inter, TOKEN_SLASH, offset)) return true;


    return false;
}


Node *make_node_from_stacks(Parser *ctx) {
    Node *top = pop_op(ctx);


    switch (top->type) {
        case NODE_INVALID:
        case NodeType_COUNT:
        case NODE_PROGRAM:
        case NODE_STATEMENT:
        case NODE_NUMBER: 
        case NODE_OPENPAREN:
        case NODE_FUNCTION:
        case NODE_FUNCTIONDEF:
        case NODE_VARIABLE:
        case NODE_VARIABLEDEF: {
            assert(false);
        } break;
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

void make_all_nodes_from_operator_ctx(Parser *ctx) {
    while (ctx->op_count > 0 && ctx->op_stack[ctx->op_count - 1]->type != NODE_OPENPAREN) {
        Node *n = make_node_from_stacks(ctx);
        push_node(ctx, n);
    }
}

Node *make_function_call(Parser *ctx, u64 token_index, u64 arg_count) {
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

void parse_expr(Interpreter *inter, u64 stop_token_types) {

    while (inter->ctx.iter < inter->lex.token_count) {

        if (inter->lex.tokens[inter->ctx.iter].type & stop_token_types) {
            break;
        }


        if (inter->ctx.op_count >= 2 && inter->ctx.op_stack[inter->ctx.op_count - 1]->type != NODE_OPENPAREN) {
            s64 p_top = node_table_data[inter->ctx.op_stack[inter->ctx.op_count - 1]->type].precedence;
            bool left_associative_top = node_table_data[inter->ctx.op_stack[inter->ctx.op_count - 1]->type].left_associative;

            s64 p_prev = node_table_data[inter->ctx.op_stack[inter->ctx.op_count - 2]->type].precedence;

            if (p_top < p_prev || (p_top == p_prev && left_associative_top)) {

                // ignore top
                {
                    Node *top = pop_op(&inter->ctx);
                    make_all_nodes_from_operator_ctx(&inter->ctx);
                    push_op(&inter->ctx, top);
                }
            }

        }

        if (is_token(inter, TOKEN_IDENTIFIER, 0)) {

            u64 id_index = consume(inter);

            if (is_token(inter, TOKEN_OPENPAREN, 0)) {

                consume(inter);

                u64 func_close_index = 0;
                {
                    u64 unclosed_paren_count = 1;
                    for (u64 i = inter->ctx.iter; i < inter->lex.token_count; ++i) {

                        if (inter->lex.tokens[i].type == TOKEN_OPENPAREN) unclosed_paren_count += 1;

                        if (inter->lex.tokens[i].type == TOKEN_CLOSEPAREN) {
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
                    u64 before_count = inter->lex.token_count;
                    while (!is_token(inter, TOKEN_CLOSEPAREN, 0)) {
                        
                        inter->lex.token_count = func_close_index;
                        parse_expr(inter, TOKEN_COMMA);

                        arg_count += 1;
                        if (is_token(inter, TOKEN_CLOSEPAREN, 0)) {
                            break;
                        } else if (is_token(inter, TOKEN_COMMA, 0)) {
                            consume(inter);
                        } else {
                            // report error 
                            todo();
                        }
                    }
                    inter->lex.token_count = before_count;
                    consume(inter);
                }



                Node *function_call = make_function_call(&inter->ctx, id_index, arg_count); 
                push_node(&inter->ctx, function_call);
            } else {

                Node *var = (Node *)calloc(1, sizeof(*var));
                var->type = NODE_VARIABLE;
                var->token_index = id_index;
                push_node(&inter->ctx, var);
            }
        } else if (is_token(inter, TOKEN_NUMBER, 0)) {
            u64 token_index = consume(inter);
            Node *n = make_number(inter, token_index);
            
            push_node(&inter->ctx, n);
        } else if (is_token(inter, TOKEN_PLUS, 0)) {

            if (is_binop(inter, -1)) {
                u64 plus_index = consume(inter);
                // unary add

                Node *unary_add = (Node *)calloc(1, sizeof(*unary_add));
                unary_add->token_index = plus_index;
                unary_add->type = NODE_UNARYADD;
                unary_add->node_count = 1;
                unary_add->nodes = (Node **)calloc(unary_add->node_count, sizeof(Node *));

                push_op(&inter->ctx, unary_add);
            } else {
                u64 plus_index = consume(inter);
                
                Node *add = (Node *)calloc(1, sizeof(*add));
                add->token_index = plus_index;
                add->type = NODE_ADD;
                add->node_count = 2;
                add->nodes = (Node **)calloc(add->node_count, sizeof(Node *));

                push_op(&inter->ctx, add);
            }
        } else if (is_token(inter, TOKEN_MINUS, 0)) {
            if (is_binop(inter, -1)) {
                u64 plus_index = consume(inter);
                // unary sub

                Node *unary_sub = (Node *)calloc(1, sizeof(*unary_sub));
                unary_sub->token_index = plus_index;
                unary_sub->type = NODE_UNARYSUB;
                unary_sub->node_count = 1;
                unary_sub->nodes = (Node **)calloc(unary_sub->node_count, sizeof(Node *));

                push_op(&inter->ctx, unary_sub);
            } else {
                u64 minus_index = consume(inter);
                
                Node *sub = (Node *)calloc(1, sizeof(*sub));
                sub->token_index = minus_index;
                sub->type = NODE_SUB;
                sub->node_count = 2;
                sub->nodes = (Node **)calloc(sub->node_count, sizeof(Node *));

                push_op(&inter->ctx, sub);
            }
        } else if (is_token(inter, TOKEN_STAR, 0)) {
            u64 star_index = consume(inter);
            
            Node *mul = (Node *)calloc(1, sizeof(*mul));
            mul->token_index = star_index;
            mul->type = NODE_MUL;
            mul->node_count = 2;
            mul->nodes = (Node **)calloc(mul->node_count, sizeof(Node *));

            push_op(&inter->ctx, mul);
        } else if (is_token(inter, TOKEN_SLASH, 0)) {
            u64 slash_index = consume(inter);
            
            Node *div = (Node *)calloc(1, sizeof(*div));
            div->token_index = slash_index;
            div->type = NODE_DIV;
            div->node_count = 2;
            div->nodes = (Node **)calloc(div->node_count, sizeof(Node *));

            push_op(&inter->ctx, div);
        } else if (is_token(inter, TOKEN_OPENPAREN, 0)) {
            u64 open_paren_index = consume(inter);

            Node *open_paren = (Node *)calloc(1, sizeof(*open_paren));

            open_paren->type = NODE_OPENPAREN;
            open_paren->token_index = open_paren_index;

            push_op(&inter->ctx, open_paren);
        } else if (is_token(inter, TOKEN_CLOSEPAREN, 0)) {
            consume(inter);

            while (true) {
                if (inter->ctx.op_count == 0) {
                    // report mismatched parenthesis error here
                    todo();
                }
                if (inter->ctx.op_stack[inter->ctx.op_count - 1]->type == NODE_OPENPAREN) {
                    pop_op(&inter->ctx);
                    break;
                }
                Node *n = make_node_from_stacks(&inter->ctx); 
                push_node(&inter->ctx, n);
            }

        } else {
            assert(false);
        }
    }
    make_all_nodes_from_operator_ctx(&inter->ctx);
}


void parse_function_definition(Interpreter *inter) {

    if (!is_token(inter, TOKEN_IDENTIFIER, 0)) {
        todo();
    }
    u64 func_id = consume(inter);

    Node *func = (Node * )calloc(1, sizeof(*func));
    func->type = NODE_FUNCTIONDEF;
    func->token_index = func_id;


    if (!is_token(inter, TOKEN_OPENPAREN, 0)) {
        todo();
    }
    consume(inter);


    u64 stack_start = inter->ctx.stack_count;

    while (is_token(inter, TOKEN_IDENTIFIER, 0)) {
        u64 id = consume(inter);

        Node *iden = (Node *)calloc(1, sizeof(*iden));
        iden->type = NODE_VARIABLE;
        iden->token_index = id;
        push_node(&inter->ctx, iden);

        if (is_token(inter, TOKEN_COMMA, 0)) {
            consume(inter);
        } else if (is_token(inter, TOKEN_CLOSEPAREN, 0)) {
            break;
        } else {
            todo();
        }
    }
    consume(inter);


    if (!is_token(inter, TOKEN_COLON, 0)) {
        todo();
    }
    consume(inter);
    if (!is_token(inter, TOKEN_EQUAL, 0)) {
        todo();
    }
    consume(inter);

    parse_expr(inter, TOKEN_SEMICOLON);

    u64 nodes_to_pop_count = inter->ctx.stack_count - stack_start;

    func->node_count = nodes_to_pop_count;
    func->nodes = (Node **)calloc(func->node_count, sizeof(*func->nodes));

    for (u64 i = func->node_count; i-- > 0;) {
        func->nodes[i] = pop_node(&inter->ctx);
    }
    push_node(&inter->ctx, func);
}

void parse_statement(Interpreter *inter) {

    Node *stmt = (Node *)calloc(1, sizeof(*stmt));
    stmt->type = NODE_STATEMENT;
    stmt->token_index = inter->ctx.iter;
    stmt->node_count = 1;
    stmt->nodes = (Node **)calloc(stmt->node_count, sizeof(*stmt->nodes));

    bool is_definition = false;

    for (u64 i = inter->ctx.iter; i < inter->lex.token_count; ++i) {
        s64 j = (s64)i;
        if (is_token(inter, TOKEN_SEMICOLON, j)) break;

        if (is_token(inter, TOKEN_COLON, j) && is_token(inter, TOKEN_EQUAL, j + 1)) {
            is_definition = true;
            break;
        }
    }

    if (is_definition) {
        parse_function_definition(inter);
    } else {
        parse_expr(inter, TOKEN_SEMICOLON);
    }


    stmt->nodes[0] = pop_node(&inter->ctx);
    push_node(&inter->ctx, stmt);
}


void parse(Interpreter *inter) {
    Node *prog = (Node *)calloc(1, sizeof(*prog));
    prog->type = NODE_PROGRAM;
    prog->token_index = inter->ctx.iter;

    while (inter->ctx.iter < inter->lex.token_count) {
        parse_statement(inter);
        if (!is_token(inter, TOKEN_SEMICOLON, 0)) {
            todo();
        }
        consume(inter);
    }

    prog->node_count = inter->ctx.stack_count;
    prog->nodes = (Node **)calloc(prog->node_count, sizeof(*prog->nodes));

    for (u64 i = prog->node_count; i-- > 0;) {
        prog->nodes[i] = pop_node(&inter->ctx);
    }

    inter->ctx.root = prog; 
}


void graphviz_out(Interpreter *inter) {
    FILE *f = fopen("./input.dot", "wb");
    fprintf(f, "graph G {\n");


    Stack<Node *> stack = {};

    stack_push(&stack, inter->ctx.root);

    while (stack.count != 0) {
        Node *top = stack_pop(&stack);
        Token t = inter->lex.tokens[top->token_index];
        u64 len = t.end - t.start;
        fprintf(f, "n%llu [label=\"%s: %.*s\"]\n", (u64)top, str_NodeType[top->type], (int)len, inter->src.dat + t.start);

        for (u64 i = 0; i < top->node_count; ++i) {
            fprintf(f, "n%llu -- n%llu\n", (u64)top, (u64)top->nodes[i]);
            stack_push(&stack, top->nodes[i]);
        }

    }

    fprintf(f, "}");
    fclose(f);
}

void bytecode_from_tree2(Interpreter *inter, Node *n) {

    switch (n->type) {
        case NODE_INVALID: todo(); break;
        case NODE_PROGRAM: {
            stack_push(&inter->scopes, Scope {});
            for (u64 i = 0; i < n->node_count; ++i) {
                bytecode_from_tree2(inter, n->nodes[i]);
            }

        } break;
        case NODE_STATEMENT: {
            assert(n->node_count == 1);
            bytecode_from_tree2(inter, n->nodes[0]);
            
        } break;
        case NODE_NUMBER: todo(); break;
        case NODE_FUNCTION: todo(); break;
        case NODE_FUNCTIONDEF: {
           todo();
        } break;
        case NODE_VARIABLE: todo(); break;
        case NODE_VARIABLEDEF: todo(); break;
        case NODE_ADD: todo(); break;
        case NODE_SUB: {
            todo();
        } break;
        case NODE_MUL: todo(); break;
        case NODE_DIV: todo(); break;
        case NODE_UNARYADD: todo(); break;
        case NODE_UNARYSUB: todo(); break;
        case NODE_OPENPAREN: todo(); break;
        case NodeType_COUNT: todo(); break;
    }
}

void bytecode_from_tree(Interpreter *inter) {
    bytecode_from_tree2(inter, inter->ctx.root);
}



void execute(Interpreter *inter, String func) {
    todo();
}


// UI System

#define TEXT_INPUT_FONT_SIZE 20
#define TEXT_INPUT_HEIGHT 35
#define TEXT_INPUT_MARGIN 5
#define TEXT_INPUT_CURSOR_COLOR BLACK
#define TEXT_INPUT_SELECTION_COLOR BLUE
#define DISPLAY_STRING_HEIGHT 25
#define DISPLAY_STRING_FONT_SIZE 15
#define DRAG_BAR_HEIGHT 15


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



void shift_right_resize(u8 *buf, u64 *count, u64 start, u64 amount) {

    for (u64 k = 0; k < amount; ++k) {
        *count += 1;
        // end - 1 to skip last element remember i-- also decrements at first iteration
        for (u64 i = *count - 1; i-- > start;) {
            buf[i + 1] = buf[i];
        } 
    }
}

void shift_left_resize(u8 *buf, u64 *count, u64 start, u64 amount) {
    assert(*count >= amount);
    for (u64 k = 0; k < amount; ++k) {
        for (u64 i = start; i < *count - 1; ++i) {
            buf[i] = buf[i + 1];
        } 
        *count -= 1;

    }
    // end - 1 to not go out of bounds
}


Arena *scratch = nullptr;


u64 get_text_cursor_pos_from_mouse(u8 *text_buf, u64 *text_count, Pane *p, f32 mx) {

    for (u64 j = 1; j <= *text_count; ++j) {

        u64 tmp = arena_get_pos(scratch);

        String s = string_printf(scratch, "%.*s", (int)j, text_buf);
        f32 sz = (f32)MeasureText((char *)s.dat, TEXT_INPUT_FONT_SIZE);

        arena_set_pos(scratch, tmp);

        if (mx - (p->x + TEXT_INPUT_MARGIN + sz) < TEXT_INPUT_FONT_SIZE / 2) {
            return j;
        }
    }
    if (*text_count > 0) return *text_count - 1;
    else return 0; 
}


int main(void) {

    Arena t; arena_init(&t, 1000000);
    scratch = &t;

    String_Builder sb; string_builder_init(&sb, 65000);

    // Interpreter *e = (Interpreter *)arena_alloc(scratch, sizeof(*e));
    Interpreter *inter = (Interpreter *)calloc(1, sizeof(*inter));

    String src = str_from_cstr("f(x):=x*x;f(15);");

    memset(inter, 0, sizeof(*inter));
    tokenize(inter, src);
    parse(inter);
    graphviz_out(inter);
    bytecode_from_tree(inter);
    execute(inter, str_from_cstr("e2"));



    return 0;
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


    u8 text_buf[5][64] = {};
    u64 text_count[5] = {};

    u8 display_text_buf[5][64] = {};
    u64 display_text_count[5] = {};

    UI_State ui = {};


    while (!WindowShouldClose()) {
        u64 tmp_pos = arena_get_pos(scratch);

        f32 mx = (f32)GetMouseX();
        f32 my = (f32)GetMouseY();


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
            if (tmp == KEY_LEFT_CONTROL) continue;
            if (tmp == KEY_RIGHT_CONTROL) continue;
            if (tmp == KEY_LEFT_SHIFT) continue;
            if (tmp == KEY_RIGHT_SHIFT) continue;

            keys_pressed[key_count++] = tmp;
        }

        u64 pane_count = 0;
        f32 h_offset;

        BeginDrawing();


        u64 pane_id = pane_count++;
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


            // text input + display string
            for (u64 i = 0; i < 5; ++i) {

                if (mouse_collides(p1.x, h_offset, p1.w, TEXT_INPUT_HEIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (ui.active && ui.active_id == i && ui.text_cursor) {

                        ui.selecting = false;
                        if (mx < p1.x) {
                            ui.cursor_pos = 0;
                        } else if (mx > p1.x + p1.w) {
                            ui.cursor_pos = text_count[i] - 1;
                        } else {
                            ui.cursor_pos = get_text_cursor_pos_from_mouse((u8 *)text_buf[i], text_count + i, &p1, (f32)mx);
                        }

                    } else {
                        ui.selecting = false;
                        ui.text_cursor = true;
                        ui.cursor_pos = get_text_cursor_pos_from_mouse((u8 *)text_buf[i], text_count + i, &p1, (f32)mx);
                        ui.active_id = i;
                        ui.active = true;
                    }
                }
                bool text_input_changed = false;
                if (ui.active && ui.active_id == i && ui.text_cursor) {

                    text_input_changed = key_count > 0 || char_count > 0;

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

                                            shift_left_resize(text_buf[i], text_count + i, ui.cursor_pos, prev_cursor_pos - ui.cursor_pos);
                                        } else {
                                            shift_left_resize(text_buf[i], text_count + i, ui.cursor_pos - 1, 1);
                                            ui.cursor_pos -= 1;
                                        }
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
                                String s = string_printf(scratch, "%.*s", (int)(ui.selection_end - ui.selection_start), ui.selection_start + text_buf[i]);
                                SetClipboardText((char *)s.dat);
                                shift_left_resize(text_buf[i], text_count + i, ui.selection_start, ui.selection_end - ui.selection_start);
                                ui.cursor_pos = ui.selection_start;
                            }
                        } else if (keys_pressed[j] == KEY_C) {
                            if (ui.selecting && ctrl_key_down) {
                                String s = string_printf(scratch, "%.*s", (int)(ui.selection_end - ui.selection_start), ui.selection_start + text_buf[i]);
                                SetClipboardText((char *)s.dat);
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
                        text_buf[i][ui.cursor_pos] = (u8)chars_pressed[j];
                        ui.cursor_pos += 1;
                    }
                }

                Color color = DARKGREEN;
                
                DrawRectangleV(Vector2 {p1.x, h_offset}, Vector2 {p1.w, TEXT_INPUT_HEIGHT}, color);
                {
                    String s = string_printf(scratch, "%.*s", (int)text_count[i], text_buf[i]);
                    DrawText((char *)s.dat, (s32)p1.x + TEXT_INPUT_MARGIN, (s32)(h_offset) + TEXT_INPUT_FONT_SIZE / 2, TEXT_INPUT_FONT_SIZE, LIGHTGRAY);
                }

                if (ui.active && ui.active_id == i && ui.text_cursor) {
                    String s = string_printf(scratch, "%.*s", (int)ui.cursor_pos, text_buf[i]);
                    int sz = MeasureText((char *)s.dat, TEXT_INPUT_FONT_SIZE);

                    DrawRectangleV(Vector2 {p1.x + TEXT_INPUT_MARGIN + (f32)sz, h_offset + TEXT_INPUT_FONT_SIZE / 2}, Vector2 {1, TEXT_INPUT_FONT_SIZE}, TEXT_INPUT_CURSOR_COLOR);

                    if (ui.selecting) {

                        String s1 = string_printf(scratch, "%.*s", (int)ui.selection_start, text_buf[i]);
                        int sz2 = MeasureText((char *)s1.dat, TEXT_INPUT_FONT_SIZE);
                        
                        String s2 = string_printf(scratch, "%.*s", (int)(ui.selection_end - ui.selection_start), text_buf[i] + ui.selection_start);
                        int selection_sz = MeasureText((char *)s2.dat, TEXT_INPUT_FONT_SIZE);
                        Color a = TEXT_INPUT_SELECTION_COLOR;
                        a.a = 100;
                        DrawRectangleV(Vector2 {p1.x + TEXT_INPUT_MARGIN + (f32)sz2, h_offset + TEXT_INPUT_FONT_SIZE / 2}, Vector2 {(f32)selection_sz, TEXT_INPUT_FONT_SIZE}, a);

                    }
                }

                h_offset += TEXT_INPUT_HEIGHT;

                if (text_input_changed) {

                    sb.count = 0;

                    for (u64 j = 0; j < ARRAY_SIZE(text_buf); ++j) {
                        if (text_count[j] > 0) {
                            string_builder_concat(&sb, String {(u8 *)text_buf[j], text_count[j]});
                            string_builder_append(&sb, ';');
                        }
                    }
                    String src_ = string_builder_to_string(&sb);

                    for (u64 j = 0; j < ARRAY_SIZE(display_text_buf); ++j) {
                        display_text_count[j] = 0;
                    }

                    memset(inter, 0, sizeof(*inter));
                    tokenize(inter, src_);
                    parse(inter);
                    graphviz_out(inter);
                    bytecode_from_tree(inter);
                    todo();
                    // execute(inter);


                    
                    u64 non_empty_id = 0;
                    for (u64 j = 0; j < ARRAY_SIZE(text_count); ++j) {
                        if (text_count[j] == 0) continue;

                        bool skip = false;
                        while (inter->error_stack.count > 0) {
                            Error *e = inter->error_stack.dat + inter->error_stack.count - 1;
                            todo();
                        }
                        if (!skip) {
                            Node *prog = inter->ctx.root;
                            Node *stmt = prog->nodes[non_empty_id];

                            bool is_definition = false;
                            assert(stmt->node_count == 1);
                            if (stmt->nodes[0]->type == NODE_FUNCTIONDEF || stmt->nodes[0]->type == NODE_VARIABLEDEF) {
                                is_definition = true;
                            }


                            if (is_definition) {
                                todo();
                            } else {
                                String s = string_printf(scratch, "%lg", stack_pop(&inter->val_stack));
                                memcpy(display_text_buf[j], s.dat, s.count + 1);
                                display_text_count[j] = s.count;
                            }
                        }
                        non_empty_id += 1;
                    }
                    
                }
                DrawRectangleV(Vector2 {p1.x, h_offset}, Vector2 {p1.w, DISPLAY_STRING_HEIGHT}, ColorBrightness(color, 0.2f));
                DrawText((char *)display_text_buf[i], (s32)p1.x + TEXT_INPUT_MARGIN, (s32)(h_offset) + DISPLAY_STRING_FONT_SIZE / 2, DISPLAY_STRING_FONT_SIZE, LIGHTGRAY);
                




                h_offset += DISPLAY_STRING_HEIGHT;
            }
        }
        pane_id = pane_count++;
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
        arena_set_pos(scratch, tmp_pos);
    }
    arena_clean(&t);
    scratch = nullptr;


    CloseWindow();        // Close window and OpenGL context

    return 0;
}


