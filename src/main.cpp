#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <string.h>

#include "common.h"
#include "arena.h"
#include "meta.h"
#include "string.h"

template <typename T>
struct DynArray {
    u64 count;
    u64 cap;
    T *dat;
};
template <typename T>
void dynarray_init(DynArray<T> *dynarray, u64 cap) {
    dynarray->count = 0;
    dynarray->cap = cap;
    dynarray->dat = (T *)calloc(dynarray->cap, sizeof(T));
}
template <typename T>
void dynarray_append(DynArray<T> *dynarray, T v) {
    if (dynarray->cap == 0) dynarray_init(dynarray, 1 << 14);
    assert(dynarray->count < dynarray->cap);
    dynarray->dat[dynarray->count++] = v;
}
template <typename T>
T dynarray_pop(DynArray<T> *dynarray) {
    assert(dynarray->count > 0);
    return dynarray->dat[--dynarray->count];
}




struct NodeTableData {
    s64 precedence;
    bool left_associative;
    bool is_expr;
};

NodeTableData node_table_data[] = {
    #define X(type, precedence, is_left_associative, is_expr) {precedence, is_left_associative, is_expr},
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

struct Item {
    ItemType type;
    String name;
    u64 func_args;
    u64 id;
};

struct Scope {
    DynArray<Item> items;
};

struct Node {
    NodeType type;
    u64 token_index;

    Scope scope;

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

union StackData {
    u64 u;
    f64 f;
};


struct Bytecode {
    BytecodeType type;
    StackData imm;
};

struct Error {
    bool has_statement;
    u64 statement_id;

    bool has_char;
    u64 char_id;

    bool has_token;
    u64 token_id;

    String err_string;
};


struct Interpreter {

    String src;
    Lexer lex;
    Parser ctx;
    DynArray<Bytecode> bytecode;

    DynArray<u64> symbol_ids;
    DynArray<String> symbols;

    Arena func_arena;

    u64 program_counter;
    u64 return_address;
    u64 base_stackframe_index;

    DynArray<StackData> stack;
    DynArray<Error> errors;
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

String string_from_token(Interpreter *inter, u64 token_index) {
    String s = {};
    Token *t = inter->lex.tokens + token_index;
    s.count = t->end - t->start;
    s.dat = inter->src.dat + t->start;

    return s;
};

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

Node *make_number(u64 token_index) {
    Node *n = (Node *)calloc(1, sizeof(*n));

    n->type = NODE_NUMBER;
    n->token_index = token_index;
    return n;
}

bool is_binop(Interpreter *inter, s64 offset) {
    if (is_token(inter, TOKEN_PLUS, offset)) return true;
    if (is_token(inter, TOKEN_MINUS, offset)) return true;
    if (is_token(inter, TOKEN_STAR, offset)) return true;
    if (is_token(inter, TOKEN_SLASH, offset)) return true;


    return false;
}


void pop_reverse_to_subnodes(Parser *ctx, Node *top) {
    for (u64 i = top->node_count; i-- > 0;) {
        top->nodes[i] = pop_node(ctx);
    }
}
Node *make_node_from_stacks(Interpreter *inter) {
    Node *top = pop_op(&inter->ctx);


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
        case NODE_ADD: {
            if (inter->ctx.stack_count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for + operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);
        } break;
        case NODE_SUB: {
            if (inter->ctx.stack_count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for - operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_MUL: {
            if (inter->ctx.stack_count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for * operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_DIV: {
            if (inter->ctx.stack_count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for / operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_UNARYADD: {
            if (inter->ctx.stack_count < 1) {
                Error err = {};
                err.err_string = str_lit("Expected 1 operands for unary + operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_UNARYSUB: {
            if (inter->ctx.stack_count < 1) {
                Error err = {};
                err.err_string = str_lit("Expected 1 operand for unary - operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
    }

    return top;
}

void make_all_nodes_from_operator_ctx(Interpreter *inter) {
    while (inter->ctx.op_count > 0 && inter->ctx.op_stack[inter->ctx.op_count - 1]->type != NODE_OPENPAREN) {
        Node *n = make_node_from_stacks(inter);
        push_node(&inter->ctx, n);
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
                    make_all_nodes_from_operator_ctx(inter);
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
                    Error err = {};
                    err.err_string = str_lit("Expected closing parenthesis");
                    dynarray_append(&inter->errors, err);
                    return;
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
            Node *n = make_number(token_index);

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
                Node *n = make_node_from_stacks(inter);
                push_node(&inter->ctx, n);
            }

        } else {
            Error err = {};
            err.token_id = inter->ctx.iter;
            err.has_token = true;
            err.err_string = str_lit("When parsing expression unexpected token");
            dynarray_append(&inter->errors, err);
            return;
        }
    }
    make_all_nodes_from_operator_ctx(inter);
}

void parse_definition(Interpreter *inter) {

    if (!is_token(inter, TOKEN_IDENTIFIER, 0)) {
        todo();
    }
    u64 id = consume(inter);

    Node *def = (Node *)calloc(1, sizeof(*def));
    def->token_index = id;


    if (is_token(inter, TOKEN_OPENPAREN, 0)) {
        consume(inter);
        def->type = NODE_FUNCTIONDEF;
        u64 stack_start = inter->ctx.stack_count;

        while (is_token(inter, TOKEN_IDENTIFIER, 0)) {
            u64 arg_id = consume(inter);

            Node *iden = (Node *)calloc(1, sizeof(*iden));
            iden->type = NODE_VARIABLE;
            iden->token_index = arg_id;
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

        def->node_count = nodes_to_pop_count;
        def->nodes = (Node **)calloc(def->node_count, sizeof(*def->nodes));

        for (u64 i = def->node_count; i-- > 0;) {
            def->nodes[i] = pop_node(&inter->ctx);
        }
    } else {
        def->type = NODE_VARIABLEDEF;
        if (!is_token(inter, TOKEN_COLON, 0)) {
            todo();
        }
        consume(inter);
        if (!is_token(inter, TOKEN_EQUAL, 0)) {
            todo();
        }
        consume(inter);
        parse_expr(inter, TOKEN_SEMICOLON);

        def->node_count = 1;
        def->nodes = (Node **)calloc(1, sizeof(*def->nodes));

        if (inter->ctx.stack_count == 0) {
            Error err = {};
            err.err_string = str_lit("Cannot have empty expression in variable definition");
            dynarray_append(&inter->errors, err);
            return;
        }
        def->nodes[0] = pop_node(&inter->ctx);

    }

    push_node(&inter->ctx, def);
}


void parse_statement(Interpreter *inter) {

    Node *stmt = (Node *)calloc(1, sizeof(*stmt));
    stmt->type = NODE_STATEMENT;
    stmt->token_index = inter->ctx.iter;
    stmt->node_count = 1;
    stmt->nodes = (Node **)calloc(stmt->node_count, sizeof(*stmt->nodes));

    bool is_definition = false;

    for (u64 i = 0; i < inter->lex.token_count - inter->ctx.iter; ++i) {
        s64 j = (s64)i;
        if (is_token(inter, TOKEN_SEMICOLON, j)) break;

        if (is_token(inter, TOKEN_COLON, j) && is_token(inter, TOKEN_EQUAL, j + 1)) {
            is_definition = true;
            break;
        }
    }

    if (is_definition) {
        parse_definition(inter);
    } else {
        parse_expr(inter, TOKEN_SEMICOLON);
    }

    if (inter->errors.count > 0) {
        inter->errors.dat[inter->errors.count - 1].statement_id = stmt->token_index;
        inter->errors.dat[inter->errors.count - 1].has_statement = true;
        return;
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
        if (inter->errors.count > 0) return;

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
    fprintf(f, "digraph G {\n");


    DynArray<Node *> stack = {};

    dynarray_append(&stack, inter->ctx.root);

    while (stack.count != 0) {
        Node *top = dynarray_pop(&stack);
        Token t = inter->lex.tokens[top->token_index];
        u64 len = t.end - t.start;
        fprintf(f, "n%llu [label=\"%s: %.*s\"]\n", (u64)top, str_NodeType[top->type].dat, (int)len, inter->src.dat + t.start);

        for (u64 i = 0; i < top->node_count; ++i) {
            fprintf(f, "n%llu -> n%llu\n", (u64)top, (u64)top->nodes[i]);
            dynarray_append(&stack, top->nodes[i]);
        }

    }

    fprintf(f, "}");
    fclose(f);
}


Item *find_item_in_scope(String name, Scope *s) {
    for (u64 i = 0; i < s->items.count; ++i) {
        Item *active = s->items.dat + i;
        if (string_equal(name, active->name)) {
            return active;
        }
    }
    return nullptr;
}

void typecheck_tree2(Interpreter *inter, Node *n, Scope *prev_scope) {
    if (prev_scope) {
        for (u64 i = 0; i < prev_scope->items.count; ++i) {
            Item *item = prev_scope->items.dat + i;
            dynarray_append(&n->scope.items, *item);
        }
    }
    switch (n->type) {
        case NODE_INVALID: assert(false && "unreachable"); break;
        case NODE_PROGRAM: {
            for (u64 i = 0; i < n->node_count; ++i) {
                Node *node = n->nodes[i];
                if (node->type == NODE_STATEMENT) {
                    assert(node->node_count == 1);
                    Node *inner = node->nodes[0];
                    Item item = {};
                    if (inner->type == NODE_FUNCTIONDEF) {
                        item.type = ITEM_FUNCTION;
                        item.name = string_from_token(inter, inner->token_index);
                        item.func_args = inner->node_count - 1;
                    } else if (inner->type == NODE_VARIABLEDEF) {
                        item.type = ITEM_GLOBALVARIABLE;
                        item.name = string_from_token(inter, inner->token_index);
                    }
                    dynarray_append(&n->scope.items, item);
                }
            }
            for (u64 i = 0; i < n->node_count; ++i) {
                typecheck_tree2(inter, n->nodes[i], &n->scope);
            }
        } break;
        case NODE_STATEMENT: {
            assert(n->node_count == 1);
            typecheck_tree2(inter, n->nodes[0], &n->scope);
        } break;
        case NODE_NUMBER: {
            // do nothing
        } break;
        case NODE_FUNCTION: {
            String name = string_from_token(inter, n->token_index);

            Item *item = find_item_in_scope(name, &n->scope);
            if (!item) {
                Error err = {};
                err.err_string = str_lit("Undeclared symbol f");
                dynarray_append(&inter->errors, err);
                return;
            }
            if (string_equal(name, item->name)) {
                if (item->type != ITEM_FUNCTION) {
                    // wrong type
                    todo();
                }
                if (n->node_count > item->func_args) {
                    // too many args
                    todo();
                }
                if (n->node_count < item->func_args) {
                    // too few args
                    todo();
                }
            }
            for (u64 i = 0; i < n->node_count; ++i) {
                typecheck_tree2(inter, n->nodes[i], &n->scope);
            }
        } break;
        case NODE_FUNCTIONDEF: {
            for (u64 i = 0; i < n->node_count - 1; ++i) {
                Node *var = n->nodes[i];
                Item item = {};
                item.type = ITEM_VARIABLE;
                item.name = string_from_token(inter, var->token_index);
                item.id = i;
                // shadow declaration if it already exists
                Item *old = find_item_in_scope(item.name, &n->scope);
                if (old) {
                    *old = item;
                } else {
                    dynarray_append(&n->scope.items, item);
                }
            }
            typecheck_tree2(inter, n->nodes[n->node_count - 1], &n->scope);
        } break;
        case NODE_VARIABLE: {
            String name = string_from_token(inter, n->token_index);
            Item *item = find_item_in_scope(name, &n->scope);
            if (!item) {
                Error err = {};
                err.err_string = str_lit("Undeclared variable");
                err.token_id = n->token_index;
                err.has_token = true;
                dynarray_append(&inter->errors, err);
                return;
            }
            if (string_equal(name, item->name)) {
                if (item->type != ITEM_VARIABLE && item->type != ITEM_GLOBALVARIABLE) {
                    // wrong type
                    todo();
                }
            }
        } break;
        case NODE_VARIABLEDEF: {
            typecheck_tree2(inter, n->nodes[0], &n->scope);
        } break;
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MUL:
        case NODE_DIV: {
            assert(n->node_count == 2);
            typecheck_tree2(inter, n->nodes[0], &n->scope);
            typecheck_tree2(inter, n->nodes[1], &n->scope);
        } break;
        case NODE_UNARYADD:
        case NODE_UNARYSUB: {
            assert(n->node_count == 1);
            typecheck_tree2(inter, n->nodes[0], &n->scope);
        } break;
        case NODE_OPENPAREN: assert(false && "unreachable"); break;
        case NodeType_COUNT: assert(false && "unreachable"); break;
    }
}

void typecheck_tree(Interpreter *inter) {
    typecheck_tree2(inter, inter->ctx.root, nullptr);
}

bool get_func_id_from_name(Interpreter *inter, String func, u64 *func_id_out) {
    for (u64 i = 0; i < inter->symbols.count; ++i) {
        if (string_equal(inter->symbols.dat[i], func)) {
            *func_id_out = inter->symbol_ids.dat[i];
            return true;
        }
    }
    return false;
}

void bytecode_from_tree2(Interpreter *inter, Node *n) {
    switch (n->type) {
        case NODE_INVALID: assert(false && "unreachable"); break;
        case NODE_PROGRAM: {
            for (u64 i = 0; i < n->node_count; ++i) {
                bytecode_from_tree2(inter, n->nodes[i]);
            }

        } break;
        case NODE_STATEMENT: {
            assert(n->node_count == 1);
            bool def = n->nodes[0]->type == NODE_FUNCTIONDEF || n->nodes[0]->type == NODE_VARIABLEDEF;
            if (!def) {
                String s = string_printf(&inter->func_arena, "_s%llu", inter->symbols.count);
                dynarray_append(&inter->symbols, s);
                dynarray_append(&inter->symbol_ids, inter->bytecode.count);
            }
            bytecode_from_tree2(inter, n->nodes[0]);
            if (!def) {
                StackData sd = {};
                sd.u = 0;
                dynarray_append(&inter->bytecode, {BYTECODE_RETURN, sd});
            }
        } break;
        case NODE_NUMBER: {
            String num = string_from_token(inter, n->token_index);
            char buf[32] = {};
            snprintf(buf, sizeof(buf), "%.*s", (s32)num.count, num.dat);
            StackData sd = {};
            sd.f = atof(buf);

            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_PUSH, sd});
        } break;
        case NODE_FUNCTION: {
            for (u64 i = n->node_count; i-- > 0;) {
                bytecode_from_tree2(inter, n->nodes[i]);
            }
            String name = string_from_token(inter, n->token_index);
            StackData func_id = {};
            assert(get_func_id_from_name(inter, name, &func_id.u));

            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_CALL, func_id});
        } break;
        case NODE_FUNCTIONDEF: {
            String name = string_from_token(inter, n->token_index);
            dynarray_append(&inter->symbols, name);
            dynarray_append(&inter->symbol_ids, inter->bytecode.count);

            bytecode_from_tree2(inter, n->nodes[n->node_count - 1]);
            StackData num_args = {};
            num_args.u = n->node_count - 1;
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_RETURN, num_args});
        } break;
        case NODE_VARIABLE: {
            String var_name = string_from_token(inter, n->token_index);
            Item *item = find_item_in_scope(var_name, &n->scope);
            assert(item);

            if (item->type == ITEM_GLOBALVARIABLE) {
                String name = string_from_token(inter, n->token_index);
                StackData func_id = {};
                assert(get_func_id_from_name(inter, name, &func_id.u));
                dynarray_append(&inter->bytecode, Bytecode {BYTECODE_CALL, func_id});

            } else if (item->type == ITEM_VARIABLE) {
                StackData sd = {};
                sd.u = item->id;
                dynarray_append(&inter->bytecode, Bytecode {BYTECODE_PUSH_ARG, sd});
            } else {
                assert(false && "unreachable");
            }

        } break;
        case NODE_VARIABLEDEF: {
            String name = string_from_token(inter, n->token_index);
            dynarray_append(&inter->symbols, name);
            dynarray_append(&inter->symbol_ids, inter->bytecode.count);

            assert(n->node_count == 1);
            bytecode_from_tree2(inter, n->nodes[0]);
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_RETURN, {}});
        } break;
        case NODE_ADD: {
            bytecode_from_tree2(inter, n->nodes[1]);
            bytecode_from_tree2(inter, n->nodes[0]);
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_ADD, {}});
        } break;
        case NODE_SUB: {
            bytecode_from_tree2(inter, n->nodes[1]);
            bytecode_from_tree2(inter, n->nodes[0]);
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_SUB, {}});
        } break;
        case NODE_MUL: {
            bytecode_from_tree2(inter, n->nodes[1]);
            bytecode_from_tree2(inter, n->nodes[0]);
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_MUL, {}});
        } break;
        case NODE_DIV: {
            bytecode_from_tree2(inter, n->nodes[1]);
            bytecode_from_tree2(inter, n->nodes[0]);
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_DIV, {}});
        } break;
        case NODE_UNARYADD: {
            bytecode_from_tree2(inter, n->nodes[0]);
        } break;
        case NODE_UNARYSUB: {
            bytecode_from_tree2(inter, n->nodes[0]);
            dynarray_append(&inter->bytecode, Bytecode {BYTECODE_NEG, {}});
        } break;
        case NODE_OPENPAREN: assert(false && "unreachable"); break;
        case NodeType_COUNT: assert(false && "unreachable"); break;
    }
}

void bytecode_from_tree(Interpreter *inter) {
    bytecode_from_tree2(inter, inter->ctx.root);
}

void print_bytecode(DynArray<Bytecode> *dynarray) {
    for (u64 i = 0; i < dynarray->count; ++i) {
        Bytecode *curr = dynarray->dat + i;
        printf("%.*s, %f, %llu\n", (s32)str_BytecodeType[curr->type].count, str_BytecodeType[curr->type].dat, curr->imm.f, curr->imm.u);
    }
}


bool execute(Interpreter *inter, String func, f64 *args, u64 func_args_count) {
    if (inter->errors.count > 0) return false;
    u64 func_id = 0;
    if (!get_func_id_from_name(inter, func, &func_id)) {
        return false;
    }

    Item *item = find_item_in_scope(func, &inter->ctx.root->scope);
    if (item) {
        if (item->func_args != func_args_count) return false;
    } else {
        if (func_args_count != 0) return false;
    }

    for (u64 j = func_args_count; j-- > 0;) {
        StackData sd = {};
        sd.f = args[j];
        dynarray_append(&inter->stack, sd);
    }


    inter->program_counter = func_id;
    inter->base_stackframe_index = inter->stack.count;
    // arg n - 1
    // arg 1
    // arg 0
    // return address
    // base pointer
    // stuff
    while (true) {

        Bytecode *curr = inter->bytecode.dat + inter->program_counter;

        switch (curr->type) {

            case BYTECODE_INVALID: assert(false && "unreachable"); break;
            case BYTECODE_CALL: {
                inter->return_address = inter->program_counter + 1;
                inter->program_counter = curr->imm.u;

                StackData return_addr = {};
                return_addr.u = inter->return_address;
                dynarray_append(&inter->stack, return_addr);

                StackData base = {};
                base.u = inter->base_stackframe_index;
                inter->base_stackframe_index = inter->stack.count;
                dynarray_append(&inter->stack, base);
            } break;
            case BYTECODE_RETURN: {
                if (inter->base_stackframe_index == 0) {
                    return true;
                }
                // save result then cleanup
                StackData result = dynarray_pop(&inter->stack);

                inter->stack.count = inter->base_stackframe_index + 1;
                inter->base_stackframe_index = dynarray_pop(&inter->stack).u;
                inter->return_address = dynarray_pop(&inter->stack).u;
                inter->program_counter = inter->return_address;

                for (u64 i = 0; i < curr->imm.u; ++i) {
                    dynarray_pop(&inter->stack);
                }
                dynarray_append(&inter->stack, result);
            } break;
            case BYTECODE_PUSH_ARG: {
                StackData sd = inter->stack.dat[inter->base_stackframe_index - 2 - curr->imm.u];
                dynarray_append(&inter->stack, sd);
                inter->program_counter += 1;
            } break;
            case BYTECODE_PUSH: {
                dynarray_append(&inter->stack, curr->imm);
                inter->program_counter += 1;
            } break;
            case BYTECODE_NEG: {
                StackData sd = dynarray_pop(&inter->stack);
                sd.f = -sd.f;
                dynarray_append(&inter->stack, sd);
                inter->program_counter += 1;
            } break;
            case BYTECODE_ADD: {
                StackData sd1 = dynarray_pop(&inter->stack);
                StackData sd2 = dynarray_pop(&inter->stack);
                StackData sd3 = {};

                sd3.f = sd1.f + sd2.f;
                dynarray_append(&inter->stack, sd3);
                inter->program_counter += 1;
            } break;
            case BYTECODE_SUB: {
                StackData sd1 = dynarray_pop(&inter->stack);
                StackData sd2 = dynarray_pop(&inter->stack);
                StackData sd3 = {};

                sd3.f = sd1.f - sd2.f;
                dynarray_append(&inter->stack, sd3);
                inter->program_counter += 1;
            } break;
            case BYTECODE_MUL: {
                StackData sd1 = dynarray_pop(&inter->stack);
                StackData sd2 = dynarray_pop(&inter->stack);
                StackData sd3 = {};

                sd3.f = sd1.f * sd2.f;
                dynarray_append(&inter->stack, sd3);
                inter->program_counter += 1;

            } break;
            case BYTECODE_DIV: {
                StackData sd1 = dynarray_pop(&inter->stack);
                StackData sd2 = dynarray_pop(&inter->stack);
                StackData sd3 = {};

                sd3.f = sd1.f / sd2.f;
                dynarray_append(&inter->stack, sd3);
                inter->program_counter += 1;

            } break;
            case BytecodeType_COUNT: assert(false && "unreachable"); break;
        }
    }

    return false;
}

void clear_interpreter(Interpreter *inter) {
    inter->src = {};
    inter->lex = {};
    inter->ctx = {};
    inter->bytecode.count = 0;
    inter->symbol_ids.count = 0;
    inter->symbols.count = 0;
    arena_clear(&inter->func_arena);
    inter->program_counter = 0;
    inter->return_address = 0;
    inter->base_stackframe_index = 0;
    inter->stack.count = 0;
    inter->errors.count = 0;
}


void compile(Interpreter *inter, String src) {
    tokenize(inter, src);
    if (inter->errors.count == 0) parse(inter);
    if (inter->errors.count == 0) graphviz_out(inter);
    if (inter->errors.count == 0) typecheck_tree(inter);
    if (inter->errors.count == 0) bytecode_from_tree(inter);
}


// UI System

#define TEXT_INPUT_FONT_SIZE 20
#define TEXT_INPUT_HEIGHT 35
#define TEXT_INPUT_MARGIN 5
#define TEXT_INPUT_CURSOR_COLOR BLACK
#define TEXT_INPUT_SELECTION_COLOR BLUE
#define TEXT_INPUT_BACKGROUND_COLOR DARKGREEN

#define DISPLAY_STRING_HEIGHT 25
#define DISPLAY_STRING_FONT_SIZE 15

#define DRAG_BAR_HEIGHT 15
#define DRAG_BAR_COLOR GREEN


const u64 nil_id = 0;

struct Ui_Event {
    bool text_input_changed;
};

struct UI_Pane {
    u64 hash;



    u64 flags;
    f32 x, y;
    f32 w, h;

    f32 h_offset;

    u64 parent_id;
    u64 next_id;

    u8 *text_buf;
    u64 *text_count;
    u64 text_capacity;

    Ui_Event event;

};


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

    u64 pane_count;


    DynArray<UI_Pane> ui_panes[2];
    u64 active_panes_id;


    DynArray<u64> parent_stack;


    f32 mx;
    f32 my;


    u64 key_count;
    u64 char_count;


    bool ctrl_key_down;
    bool shift_key_down;

    int keys_pressed[16];
    int chars_pressed[16];
};


bool mouse_collides(f32 x, f32 y, f32 w, f32 h) {
    f32 mx = (f32)GetMouseX();
    f32 my = (f32)GetMouseY();

    bool x_intercept = mx >= x && mx <= x + w;
    bool y_intercept = my >= y && my <= y + h;

    return x_intercept && y_intercept;
}


void push_parent(UI_State *ui) {
    DynArray<UI_Pane> *panes = ui->ui_panes + ui->active_panes_id; 
    assert(panes->count > 0);
    u64 id = panes->count - 1;
    dynarray_append(&ui->parent_stack, id);
}


void pop_parent(UI_State *ui) {
    dynarray_pop(&ui->parent_stack);    
}

u64 get_parent_id(UI_State *ui) {
    if (ui->parent_stack.count == 0) return 0;
    return ui->parent_stack.dat[ui->parent_stack.count - 1];
}


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


u64 get_text_cursor_pos_from_mouse(u8 *text_buf, u64 *text_count, UI_Pane *p, f32 mx) {

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



#define has_flags(data, flags) (((data) & (flags)) == (flags))


void begin_ui(UI_State *ui) {
    ui->ui_panes[ui->active_panes_id].count = 0;
}

void end_ui(UI_State *ui) {
    ui->active_panes_id += 1;
    ui->active_panes_id %= 2;
}

UI_Pane *get_pane_from_hash(UI_State *ui, u64 hash) {
    u64 prev_id = (ui->active_panes_id + 1) % 2;
    for (u64 i = 0; i < ui->ui_panes[prev_id].count; ++i) {
        if (ui->ui_panes[prev_id].dat[i].hash == hash) {
            return ui->ui_panes[prev_id].dat + i;
        } 
    }
    return nullptr;
}

Ui_Event create_pane(UI_State *ui, u64 flags, u64 hash, f32 x, f32 y, f32 w, f32 h, u8 *text_buf, u64 *text_count, u64 text_capacity) {
    UI_Pane pane = {};

    UI_Pane *old_pane = get_pane_from_hash(ui, hash);
    if (old_pane) {
        pane = *old_pane;
    } else {
        pane.x = x;
        pane.y = y;
        pane.w = w;
        pane.h = h;

        pane.text_buf = text_buf;
        pane.text_count = text_count;
        pane.text_capacity = text_capacity;
    }
    
    pane.flags = flags;
    pane.hash = hash;
    
    pane.parent_id = get_parent_id(ui);

    dynarray_append(ui->ui_panes + ui->active_panes_id, pane);


    return pane.event;
}

void draw_ui(UI_State *ui) {

    DynArray<UI_Pane> *panes = ui->ui_panes + ui->active_panes_id;

    for (u64 i = 0; i < panes->count; ++i) {

        UI_Pane *pane = panes->dat + i;

        if (has_flags(pane->flags, PANE_DRAGGABLE)) {
            DrawRectangleV(Vector2 {pane->x, pane->y}, Vector2 {pane->w, DRAG_BAR_HEIGHT}, DRAG_BAR_COLOR);
        }

        if (has_flags(pane->flags, PANE_TEXT_INPUT)) {
            DrawRectangleV(Vector2 {pane->x, pane->y}, Vector2 {pane->w, TEXT_INPUT_HEIGHT}, TEXT_INPUT_BACKGROUND_COLOR);
        }

    }
}

Ui_Event update_pane(UI_State *ui, u64 flags, f32 *h_offset, UI_Pane *p, u8 *text_buf, u64 *text_count, u64 text_capacity) {

    u64 pane_id = ui->pane_count++;

    Ui_Event event = {};


    if (has_flags(flags, PANE_DRAGGABLE)) {
        if (ui->dragging && ui->drag_id == pane_id && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            ui->dragging = false;
        }

        if (ui->dragging && ui->drag_id == pane_id) {
            p->x = ui->mx - ui->drag_x_offset;
            p->y = ui->my - ui->drag_y_offset;
        }

        if (mouse_collides(p->x, *h_offset, p->w, DRAG_BAR_HEIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ui->dragging = true;
            ui->drag_id = pane_id;
            ui->drag_x_offset = ui->mx - p->x;
            ui->drag_y_offset = ui->my - p->y;
        }
        *h_offset += DRAG_BAR_HEIGHT;
    }



    if (has_flags(flags, PANE_TEXT_INPUT)) {

        if (mouse_collides(p->x, *h_offset, p->w, TEXT_INPUT_HEIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (ui->active && ui->active_id == pane_id && ui->text_cursor) {

                ui->selecting = false;
                if (ui->mx < p->x) {
                    ui->cursor_pos = 0;
                } else if (ui->mx > p->x + p->w) {
                    ui->cursor_pos = *text_count - 1;
                } else {
                    ui->cursor_pos = get_text_cursor_pos_from_mouse(text_buf, text_count, p, ui->mx);
                }

            } else {
                ui->selecting = false;
                ui->text_cursor = true;
                ui->cursor_pos = get_text_cursor_pos_from_mouse(text_buf, text_count, p, ui->mx);
                ui->active_id = pane_id;
                ui->active = true;
            }
        }
        event.text_input_changed = false;
        if (ui->active && ui->active_id == pane_id && ui->text_cursor) {

            event.text_input_changed = ui->key_count > 0 || ui->char_count > 0;

            for (u64 j = 0; j < ui->key_count; ++j) {

                u64 prev_cursor_pos = ui->cursor_pos;

                if (ui->keys_pressed[j] == KEY_BACKSPACE) {

                    if (*text_count > 0) {
                        if (ui->selecting) {
                            ui->selecting = false;

                            shift_left_resize(text_buf, text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                            ui->cursor_pos = ui->selection_start;
                        } else {
                            if (ui->cursor_pos > 0) {

                                if (ui->ctrl_key_down) {

                                    if (is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                        while (ui->cursor_pos > 0 && is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                            ui->cursor_pos -= 1;
                                        }
                                        while (ui->cursor_pos > 0 && !is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                            ui->cursor_pos -= 1;
                                        }
                                    } else {
                                        while (ui->cursor_pos > 0 && !is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                            ui->cursor_pos -= 1;
                                        }
                                    }

                                    shift_left_resize(text_buf, text_count, ui->cursor_pos, prev_cursor_pos - ui->cursor_pos);
                                } else {
                                    shift_left_resize(text_buf, text_count, ui->cursor_pos - 1, 1);
                                    ui->cursor_pos -= 1;
                                }
                            }
                        }
                    }

                } else if (ui->keys_pressed[j] == KEY_DELETE) {

                    if (*text_count > 0) {
                        if (ui->selecting) {
                            ui->selecting = false;

                            shift_left_resize(text_buf, text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                            ui->cursor_pos = ui->selection_start;
                        } else {
                            if (ui->cursor_pos < *text_count) {
                                shift_left_resize(text_buf, text_count, ui->cursor_pos, 1);
                            }
                        }
                    }

                } else if (ui->keys_pressed[j] == KEY_A) {
                    if (*text_count > 0 && ui->ctrl_key_down) {
                        ui->selecting = true;
                        ui->cursor_pos = 0;
                        ui->selection_anchor = *text_count;
                    }
                } else if (ui->keys_pressed[j] == KEY_X) {
                    if (ui->selecting && ui->ctrl_key_down) {
                        ui->selecting = false;
                        String s = string_printf(scratch, "%.*s", (int)(ui->selection_end - ui->selection_start), ui->selection_start + text_buf);
                        SetClipboardText((char *)s.dat);
                        shift_left_resize(text_buf, text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                        ui->cursor_pos = ui->selection_start;
                    }
                } else if (ui->keys_pressed[j] == KEY_C) {
                    if (ui->selecting && ui->ctrl_key_down) {
                        String s = string_printf(scratch, "%.*s", (int)(ui->selection_end - ui->selection_start), ui->selection_start + text_buf);
                        SetClipboardText((char *)s.dat);
                    }
                } else if (ui->keys_pressed[j] == KEY_V) {
                    if (ui->ctrl_key_down) {
                        const char *text = GetClipboardText();
                        u64 len = strlen(text);

                        if (len + *text_count - (ui->selection_end - ui->selection_start) > text_capacity) {
                            // pasting clipboard would overflow
                            todo();
                        } else {
                            if (ui->selecting) {
                                ui->selecting = false;
                                shift_left_resize(text_buf, text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                                ui->cursor_pos = ui->selection_start;
                            }
                            shift_right_resize(text_buf, text_count, ui->cursor_pos, len);
                            memcpy(text_buf + ui->cursor_pos, text, len);
                            ui->cursor_pos += len;
                        }

                    }
                } else if (ui->keys_pressed[j] == KEY_HOME) {
                    if (*text_count > 0) {
                        if (!ui->selecting && ui->shift_key_down) {
                            ui->selecting = true;
                            ui->selection_anchor = prev_cursor_pos;
                        }
                        if (ui->selecting && !ui->shift_key_down) {
                            ui->selecting = false;
                        }
                        ui->cursor_pos = 0;
                    }
                } else if (ui->keys_pressed[j] == KEY_END) {
                    if (*text_count > 0) {
                        if (!ui->selecting && ui->shift_key_down) {
                            ui->selecting = true;
                            ui->selection_anchor = prev_cursor_pos;
                        }
                        if (ui->selecting && !ui->shift_key_down) {
                            ui->selecting = false;
                        }
                        ui->cursor_pos = *text_count;
                    }
                } else if (ui->keys_pressed[j] == KEY_LEFT) {
                    if (*text_count > 0 && ui->cursor_pos > 0) {

                        if (!ui->selecting && ui->shift_key_down) {
                            ui->selecting = true;
                            ui->selection_anchor = prev_cursor_pos;
                        }
                        if (ui->selecting && !ui->shift_key_down) {
                            ui->selecting = false;
                        }

                        if (ui->ctrl_key_down) {
                            if (is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                while (ui->cursor_pos > 0 && is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                    ui->cursor_pos -= 1;
                                }
                                while (ui->cursor_pos > 0 && !is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                    ui->cursor_pos -= 1;
                                }
                            } else {
                                while (ui->cursor_pos > 0 && !is_whitespace(text_buf[ui->cursor_pos - 1])) {
                                    ui->cursor_pos -= 1;
                                }
                            }
                        } else {
                            ui->cursor_pos -= 1;
                        }

                    }

                } else if (ui->keys_pressed[j] == KEY_RIGHT) {
                    if (*text_count > 0 && ui->cursor_pos < *text_count) {

                        if (!ui->selecting && ui->shift_key_down) {
                            ui->selecting = true;
                            ui->selection_anchor = prev_cursor_pos;
                        }
                        if (ui->selecting && !ui->shift_key_down) {
                            ui->selecting = false;
                        }

                        if (ui->ctrl_key_down) {
                            if (is_whitespace(text_buf[ui->cursor_pos])) {
                                while (ui->cursor_pos < *text_count && is_whitespace(text_buf[ui->cursor_pos])) {
                                    ui->cursor_pos += 1;
                                }
                                while (ui->cursor_pos < *text_count && !is_whitespace(text_buf[ui->cursor_pos])) {
                                    ui->cursor_pos += 1;
                                }
                            } else {
                                while (ui->cursor_pos < *text_count && !is_whitespace(text_buf[ui->cursor_pos])) {
                                    ui->cursor_pos += 1;
                                }
                            }
                        } else {
                            ui->cursor_pos += 1;
                        }
                    }
                }
            }

            if (ui->selecting) {
                if (ui->selection_anchor < ui->cursor_pos) {
                    ui->selection_start = ui->selection_anchor;
                    ui->selection_end = ui->cursor_pos;
                } else {
                    ui->selection_start = ui->cursor_pos;
                    ui->selection_end = ui->selection_anchor;
                }
            }

            if (ui->selecting && ui->cursor_pos == ui->selection_anchor) {
                ui->selecting = false;
            }



            // text cursor input
            for (u64 j = 0; j < ui->char_count && *text_count < text_capacity - 1; ++j) {

                if (ui->selecting) {
                    ui->selecting = false;

                    shift_left_resize(text_buf, text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                    ui->cursor_pos = ui->selection_start;
                }

                shift_right_resize(text_buf, text_count, ui->cursor_pos, 1);
                text_buf[ui->cursor_pos] = (u8)ui->chars_pressed[j];
                ui->cursor_pos += 1;
            }
        }



        if (ui->active && ui->active_id == pane_id && ui->text_cursor) {
            String s = string_printf(scratch, "%.*s", (int)ui->cursor_pos, text_buf);
            int sz = MeasureText((char *)s.dat, TEXT_INPUT_FONT_SIZE);

            DrawRectangleV(Vector2 {p->x + TEXT_INPUT_MARGIN + (f32)sz, *h_offset + TEXT_INPUT_FONT_SIZE / 2}, Vector2 {1, TEXT_INPUT_FONT_SIZE}, TEXT_INPUT_CURSOR_COLOR);

            if (ui->selecting) {

                String s1 = string_printf(scratch, "%.*s", (int)ui->selection_start, text_buf);
                int sz2 = MeasureText((char *)s1.dat, TEXT_INPUT_FONT_SIZE);

                String s2 = string_printf(scratch, "%.*s", (int)(ui->selection_end - ui->selection_start), text_buf + ui->selection_start);
                int selection_sz = MeasureText((char *)s2.dat, TEXT_INPUT_FONT_SIZE);
                Color a = TEXT_INPUT_SELECTION_COLOR;
                a.a = 100;
                DrawRectangleV(Vector2 {p->x + TEXT_INPUT_MARGIN + (f32)sz2, *h_offset + TEXT_INPUT_FONT_SIZE / 2}, Vector2 {(f32)selection_sz, TEXT_INPUT_FONT_SIZE}, a);

            }
        }


        // DrawRectangleV(Vector2 {p->x, *h_offset}, Vector2 {p->w, DISPLAY_STRING_HEIGHT}, ColorBrightness(color, 0.2f));
        // DrawText((char *)display_text_buf[pane_id], (s32)p->x + TEXT_INPUT_MARGIN, (s32)(*h_offset) + DISPLAY_STRING_FONT_SIZE / 2, DISPLAY_STRING_FONT_SIZE, LIGHTGRAY);

        // *h_offset += DISPLAY_STRING_HEIGHT;
        
    }

    if (has_flags(flags, PANE_TEXT_DISPLAY)) {
        String s = string_printf(scratch, "%.*s", (int)*text_count, text_buf);
        DrawText((char *)s.dat, (s32)p->x + TEXT_INPUT_MARGIN, (s32)(*h_offset) + TEXT_INPUT_FONT_SIZE / 2, TEXT_INPUT_FONT_SIZE, LIGHTGRAY);
    }
    // TODO: use real layouting
    if (has_flags(flags, PANE_TEXT_INPUT)) *h_offset += TEXT_INPUT_HEIGHT;





    return event;
}





UI_State ui = {};

int main(void) {

    Arena t; arena_init(&t, 1000000);
    scratch = &t;

    String_Builder sb; string_builder_init(&sb, 65000);

    // Interpreter *e = (Interpreter *)arena_alloc(scratch, sizeof(*e));
    Interpreter *inter = (Interpreter *)calloc(1, sizeof(*inter));

    // String src = str_lit("f(x, y):=x*y;f(1,2);");
    String src = str_lit("asdf;");

    memset(inter, 0, sizeof(*inter));
    arena_init(&inter->func_arena, 100000);
    compile(inter, src);
    for (u64 i = 0; i < inter->errors.count; ++i) {
        Error *err = inter->errors.dat + i;
        String s = err->err_string;
        printf("ERROR: %.*s ", (s32)s.count, s.dat);

        if (err->has_token) {
            String tok = string_from_token(inter, err->token_id);
            printf("`%.*s`", (s32)tok.count, tok.dat);
        }


        printf("\n");
    }
    print_bytecode(&inter->bytecode);
    bool r = execute(inter, str_lit("_s2"), nullptr, 0);
    printf("r = %s\n", r ? "true" : "false");
    if (r) {
        printf("Result = %g\n", inter->stack.dat[inter->stack.count - 1].f);
    }

    int screen_w = 1366;
    int screen_h = 768;


    InitWindow(screen_w, screen_h, "Para");

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);


    UI_Pane p1 = {
        0, 0,
        400, 50,
    };

    UI_Pane p2 = {
        1366-800, 0,
        800, 400,
    };


    u8 text_buf[5][64] = {};
    u64 text_count[5] = {};

    u8 display_text_buf[5][64] = {};
    u64 display_text_count[5] = {};



    while (!WindowShouldClose()) {
        u64 tmp_pos = arena_get_pos(scratch);

        ui.mx = (f32)GetMouseX();
        ui.my = (f32)GetMouseY();


        ui.char_count = 0;
        while (true) {
            int tmp = GetCharPressed();
            if (tmp == 0) break;

            ui.chars_pressed[ui.char_count++] = tmp;
        }

        ui.ctrl_key_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        ui.shift_key_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        ui.key_count = 0;
        while (true) {
            int tmp = GetKeyPressed();
            if (tmp == 0) break;
            if (tmp == KEY_LEFT_CONTROL) continue;
            if (tmp == KEY_RIGHT_CONTROL) continue;
            if (tmp == KEY_LEFT_SHIFT) continue;
            if (tmp == KEY_RIGHT_SHIFT) continue;

            ui.keys_pressed[ui.key_count++] = tmp;
        }

        f32 h_offset;

        ui.pane_count = 0;

        h_offset = p1.y;
        {
            update_pane(&ui, PANE_DRAGGABLE, &h_offset, &p1, nullptr, nullptr, 0);

            // text input + display string
            push_parent(&ui);
            for (u64 i = 0; i < 5; ++i) {
                Ui_Event event = update_pane(&ui, PANE_TEXT_INPUT|PANE_TEXT_DISPLAY, &h_offset, &p1, text_buf[i], text_count + i, sizeof(text_buf[i]));
                update_pane(&ui, PANE_TEXT_DISPLAY, &h_offset, &p1, display_text_buf[i], display_text_count + i, sizeof(display_text_buf[i]));
                if (event.text_input_changed) {
                    if (event.text_input_changed) {

                        sb.count = 0;

                        for (u64 j = 0; j < ARRAY_SIZE(text_count); ++j) {
                            if (text_count[j] > 0) {
                                string_builder_concat(&sb, String {text_buf[j], text_count[j]});
                                string_builder_append(&sb, ';');
                            }
                        }
                        String src_ = string_builder_to_string(&sb);

                        for (u64 j = 0; j < ARRAY_SIZE(display_text_buf); ++j) {
                            display_text_count[j] = 0;
                        }

                        clear_interpreter(inter);
                        compile(inter, src_);
                        String stmt_s = string_printf(&t, "_s%llu");
                        execute(inter, stmt_s, nullptr, 0);



                        u64 non_empty_id = 0;
                        for (u64 j = 0; j < ARRAY_SIZE(text_count); ++j) {
                            if (text_count[j] == 0) continue;

                            bool skip = false;
                            if (inter->errors.count > 0) skip = true;
                            // while (inter->errors.count > 0) {
                            //     Error *e = inter->errors.dat + inter->errors.count - 1;
                            //     todo();
                            // }
                            if (!skip) {
                                Node *prog = inter->ctx.root;
                                Node *stmt = prog->nodes[non_empty_id];

                                bool is_definition = false;
                                assert(stmt->node_count == 1);
                                if (stmt->nodes[0]->type == NODE_FUNCTIONDEF || stmt->nodes[0]->type == NODE_VARIABLEDEF) {
                                    is_definition = true;
                                }


                                if (is_definition) {
                                    // todo();
                                } else {
                                    String s = string_printf(scratch, "%lg", dynarray_pop(&inter->stack));
                                    memcpy(display_text_buf[j], s.dat, s.count + 1);
                                    display_text_count[j] = s.count;
                                }
                            }
                            non_empty_id += 1;
                        }
                    }
                }
            }
            pop_parent(&ui);
        }
        h_offset = p2.y;
        {
            update_pane(&ui, PANE_DRAGGABLE, &h_offset, &p2, nullptr, nullptr, 0);

            {
                DrawRectangleV(Vector2 {p2.x, h_offset}, Vector2 {p2.w, p2.w}, BLACK);
            }
            h_offset += p2.w;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        draw_ui(&ui);
        EndDrawing();
        arena_set_pos(scratch, tmp_pos);
    }
    arena_clean(&t);
    scratch = nullptr;


    CloseWindow(); // Close window and OpenGL context

    return 0;
}


