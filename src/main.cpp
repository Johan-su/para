#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include "common.h"
#include "arena.h"
#include "meta.h"
#include "string.h"

#include "window.h"
#include "glad/glad.h"
#include <stb/stb_truetype.h>


Window g_window = {};

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
    DynArray<Token> tokens;

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
    Arena node_arena;
    DynArray<Node *> node_stack;
    DynArray<Node *> op_stack;

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

add snapping for panes
add ability to snap with keyboard hotkeys instead of only mouse
add more math operators/functions like integrals, derivatives, sum.

add undo system
tooltips to evaluate expressions partially (maybe)

use arenas for memory allocation in the lexer/parser/compiler
improve color theme for the ui
add graph viewer similar to desmos


*/

void string_builder_append(DynArray<u8> *sb, u8 b) {
    assert(sb->count < sb->cap);
    sb->dat[sb->count++] = b;
}

void string_builder_concat(DynArray<u8> *sb, String s) {
    assert(sb->count + s.count < sb->cap);
    for (u64 i = 0; i < s.count; ++i) {
        sb->dat[sb->count++] = s.dat[i];
    }
}

String string_builder_to_string(DynArray<u8> *sb) {
    return String {sb->dat, sb->count};
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
                
                Error err = {};
                err.err_string = str_lit("Non digits in number");
                err.char_id = lex->iter;
                err.has_char = true;
                dynarray_append(&inter->errors, err);
                return;
            }
            u64 index_end = lex->iter;

            Token t = Token {TOKEN_NUMBER, index_start, index_end};
            dynarray_append(&lex->tokens, t);
        } else if (src.dat[lex->iter] == '+') { // 1 character tokens
            dynarray_append(&lex->tokens, Token {TOKEN_PLUS, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == '-') {
            dynarray_append(&lex->tokens, Token {TOKEN_MINUS, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == '*') {
            dynarray_append(&lex->tokens, Token {TOKEN_STAR, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == '/') {
            dynarray_append(&lex->tokens, Token {TOKEN_SLASH, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == '(') {
            dynarray_append(&lex->tokens, Token {TOKEN_OPENPAREN, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == ')') {
            dynarray_append(&lex->tokens, Token {TOKEN_CLOSEPAREN, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == ',') {
            dynarray_append(&lex->tokens, Token {TOKEN_COMMA, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == ';') {
            dynarray_append(&lex->tokens, Token {TOKEN_SEMICOLON, lex->iter, lex->iter + 1});
            lex->iter += 1;
        } else if (src.dat[lex->iter] == ':') {
            dynarray_append(&lex->tokens, Token {TOKEN_COLON, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (src.dat[lex->iter] == '=') {
            dynarray_append(&lex->tokens, Token {TOKEN_EQUAL, lex->iter, lex->iter + 1});
            lex->iter += 1;

        } else if (is_alpha(src.dat[lex->iter])) {

            u64 index_start = lex->iter;
            lex->iter += 1;
            while (is_alpha(src.dat[lex->iter]) || is_digit(src.dat[lex->iter])) {
                lex->iter += 1;
            }
            u64 index_end = lex->iter;
            Token t = Token {TOKEN_IDENTIFIER, index_start, index_end};

            dynarray_append(&lex->tokens, t);
        } else {
            Error err = {};
            err.has_char = true;
            err.char_id = lex->iter;
            err.err_string = str_lit("Unexpected character");
            dynarray_append(&inter->errors, err);
            return;
        }
    }
}




bool is_token(Interpreter *inter, TokenType t, s64 offset) {
    if ((s64)inter->ctx.iter + offset < 0) return false;
    if ((s64)inter->ctx.iter + offset > (s64)inter->lex.tokens.count) return false;

    return inter->lex.tokens.dat[(s64)inter->ctx.iter + offset].type == t;
}

String string_from_token(Interpreter *inter, u64 token_index) {
    String s = {};
    Token *t = inter->lex.tokens.dat + token_index;
    s.count = t->end - t->start;
    s.dat = inter->src.dat + t->start;

    return s;
};

u64 consume(Interpreter *inter) {
    assert(inter->ctx.iter < inter->lex.tokens.count);
    u64 token_index = inter->ctx.iter;
    inter->ctx.iter += 1;
    return token_index;
}

Node *make_number(Interpreter *inter, u64 token_index) {
    Node *n = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*n));

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
        top->nodes[i] = dynarray_pop(&ctx->node_stack);
    }
}
Node *make_node_from_stacks(Interpreter *inter) {
    Node *top = dynarray_pop(&inter->ctx.op_stack);


    switch (top->type) {
        case NODE_OPENPAREN: {
            Error err = {};
            err.token_id = top->token_index;
            err.has_token = true;
            err.err_string = str_lit("Mismatched opening parenthesis with no closing parenthesis");
            dynarray_append(&inter->errors, err);
            return nullptr;
        } break;
        case NODE_INVALID:
        case NodeType_COUNT:
        case NODE_PROGRAM:
        case NODE_STATEMENT:
        case NODE_NUMBER:
        case NODE_FUNCTION:
        case NODE_FUNCTIONDEF:
        case NODE_VARIABLE:
        case NODE_VARIABLEDEF: {
            assert(false);
        } break;
        case NODE_ADD: {
            if (inter->ctx.node_stack.count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for + operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);
        } break;
        case NODE_SUB: {
            if (inter->ctx.node_stack.count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for - operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_MUL: {
            if (inter->ctx.node_stack.count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for * operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_DIV: {
            if (inter->ctx.node_stack.count < 2) {
                Error err = {};
                err.err_string = str_lit("Expected 2 operands for / operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_UNARYADD: {
            if (inter->ctx.node_stack.count < 1) {
                Error err = {};
                err.err_string = str_lit("Expected 1 operands for unary + operator");
                dynarray_append(&inter->errors, err);
                return nullptr;
            }
            pop_reverse_to_subnodes(&inter->ctx, top);

        } break;
        case NODE_UNARYSUB: {
            if (inter->ctx.node_stack.count < 1) {
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
#define has_flags(data, flags) (((data) & (flags)) == (flags))

void make_all_nodes_from_operator_ctx(Interpreter *inter, NodeType stop_node) {
    while (inter->ctx.op_stack.count > 0 && inter->ctx.op_stack.dat[inter->ctx.op_stack.count - 1]->type != stop_node) {
        Node *n = make_node_from_stacks(inter);
        dynarray_append(&inter->ctx.node_stack, n);
    }
}

Node *make_function_call(Parser *ctx, u64 token_index, u64 arg_count) {
    Node *func = (Node *)arena_alloc(&ctx->node_arena, sizeof(*func));

    func->type = NODE_FUNCTION;
    func->token_index = token_index;
    func->node_count = arg_count;

    func->nodes = (Node **)arena_alloc(&ctx->node_arena, func->node_count *sizeof(Node *));

    for (u64 i = func->node_count; i-- > 0;) {
        func->nodes[i] = dynarray_pop(&ctx->node_stack);
    }


    return func;
}

void parse_expr(Interpreter *inter, u64 stop_token_types) {

    while (inter->ctx.iter < inter->lex.tokens.count) {

        if (inter->lex.tokens.dat[inter->ctx.iter].type & stop_token_types) {
            break;
        }

        DynArray<Node *> *ops = &inter->ctx.op_stack;

        if (ops->count >= 2 && ops->dat[ops->count - 1]->type != NODE_OPENPAREN) {
            s64 p_top = node_table_data[ops->dat[ops->count - 1]->type].precedence;
            bool left_associative_top = node_table_data[ops->dat[ops->count - 1]->type].left_associative;

            s64 p_prev = node_table_data[ops->dat[ops->count - 2]->type].precedence;

            if (p_top < p_prev || (p_top == p_prev && left_associative_top)) {

                // ignore top
                {
                    Node *top = dynarray_pop(ops);
                    make_all_nodes_from_operator_ctx(inter, NODE_OPENPAREN);
                    dynarray_append(ops, top);
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
                    for (u64 i = inter->ctx.iter; i < inter->lex.tokens.count; ++i) {

                        if (inter->lex.tokens.dat[i].type == TOKEN_OPENPAREN) unclosed_paren_count += 1;

                        if (inter->lex.tokens.dat[i].type == TOKEN_CLOSEPAREN) {
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
                    u64 before_count = inter->lex.tokens.count;
                    while (!is_token(inter, TOKEN_CLOSEPAREN, 0)) {

                        inter->lex.tokens.count = func_close_index;
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
                    inter->lex.tokens.count = before_count;
                    consume(inter);
                }



                Node *function_call = make_function_call(&inter->ctx, id_index, arg_count);
                dynarray_append(&inter->ctx.node_stack, function_call);
            } else {

                Node *var = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*var));
                var->type = NODE_VARIABLE;
                var->token_index = id_index;
                dynarray_append(&inter->ctx.node_stack, var);
            }
        } else if (is_token(inter, TOKEN_NUMBER, 0)) {
            u64 token_index = consume(inter);
            Node *n = make_number(inter, token_index);

            dynarray_append(&inter->ctx.node_stack, n);
        } else if (is_token(inter, TOKEN_PLUS, 0)) {

            if (is_binop(inter, -1)) {
                u64 plus_index = consume(inter);
                // unary add

                Node *unary_add = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*unary_add));
                unary_add->token_index = plus_index;
                unary_add->type = NODE_UNARYADD;
                unary_add->node_count = 1;
                unary_add->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, unary_add->node_count *sizeof(Node *));

                dynarray_append(&inter->ctx.op_stack, unary_add);
            } else {
                u64 plus_index = consume(inter);

                Node *add = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*add));
                add->token_index = plus_index;
                add->type = NODE_ADD;
                add->node_count = 2;
                add->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, add->node_count * sizeof(Node *));

                dynarray_append(&inter->ctx.op_stack, add);
            }
        } else if (is_token(inter, TOKEN_MINUS, 0)) {
            if (is_binop(inter, -1)) {
                u64 plus_index = consume(inter);
                // unary sub

                Node *unary_sub = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*unary_sub));
                unary_sub->token_index = plus_index;
                unary_sub->type = NODE_UNARYSUB;
                unary_sub->node_count = 1;
                unary_sub->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, unary_sub->node_count * sizeof(Node *));

                dynarray_append(&inter->ctx.op_stack, unary_sub);
            } else {
                u64 minus_index = consume(inter);

                Node *sub = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*sub));
                sub->token_index = minus_index;
                sub->type = NODE_SUB;
                sub->node_count = 2;
                sub->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, sub->node_count * sizeof(Node *));

                dynarray_append(&inter->ctx.op_stack, sub);
            }
        } else if (is_token(inter, TOKEN_STAR, 0)) {
            u64 star_index = consume(inter);

            Node *mul = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*mul));
            mul->token_index = star_index;
            mul->type = NODE_MUL;
            mul->node_count = 2;
            mul->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, mul->node_count * sizeof(Node *));

            dynarray_append(&inter->ctx.op_stack, mul);
        } else if (is_token(inter, TOKEN_SLASH, 0)) {
            u64 slash_index = consume(inter);

            Node *div = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*div));
            div->token_index = slash_index;
            div->type = NODE_DIV;
            div->node_count = 2;
            div->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, div->node_count * sizeof(Node *));

            dynarray_append(&inter->ctx.op_stack, div);
        } else if (is_token(inter, TOKEN_OPENPAREN, 0)) {
            u64 open_paren_index = consume(inter);

            Node *open_paren = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*open_paren));

            open_paren->type = NODE_OPENPAREN;
            open_paren->token_index = open_paren_index;

            dynarray_append(&inter->ctx.op_stack, open_paren);
        } else if (is_token(inter, TOKEN_CLOSEPAREN, 0)) {
            u64 token_id = consume(inter);
            if (inter->ctx.node_stack.count == 0) {
                Error err = {};
                err.has_token = true;
                err.token_id = token_id;
                err.err_string = str_lit("Empty parenthesis pair");
                dynarray_append(&inter->errors, err);
                return;
            }
            while (true) {
                if (inter->ctx.op_stack.count == 0) {
                    // report mismatched parenthesis error here
                    Error err = {};
                    err.has_token = true;
                    err.token_id = token_id;
                    err.err_string = str_lit("Mismatched parenthesis");
                    dynarray_append(&inter->errors, err);
                    return;
                }
                if (inter->ctx.op_stack.dat[inter->ctx.op_stack.count - 1]->type == NODE_OPENPAREN) {
                    dynarray_pop(&inter->ctx.op_stack);
                    break;
                }
                Node *n = make_node_from_stacks(inter);
                dynarray_append(&inter->ctx.node_stack, n);
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
    make_all_nodes_from_operator_ctx(inter, NODE_INVALID);
}

void parse_definition(Interpreter *inter) {

    if (!is_token(inter, TOKEN_IDENTIFIER, 0)) {
        todo();
    }
    u64 id = consume(inter);

    Node *def = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*def));
    def->token_index = id;


    if (is_token(inter, TOKEN_OPENPAREN, 0)) {
        consume(inter);
        def->type = NODE_FUNCTIONDEF;
        u64 stack_start = inter->ctx.node_stack.count;

        while (is_token(inter, TOKEN_IDENTIFIER, 0)) {
            u64 arg_id = consume(inter);

            Node *iden = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*iden));
            iden->type = NODE_VARIABLE;
            iden->token_index = arg_id;
            dynarray_append(&inter->ctx.node_stack, iden);

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
        u64 nodes_to_pop_count = inter->ctx.node_stack.count - stack_start;

        def->node_count = nodes_to_pop_count + 1;
        def->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, def->node_count * sizeof(*def->nodes));

        for (u64 i = def->node_count - 1; i-- > 0;) {
            def->nodes[i] = dynarray_pop(&inter->ctx.node_stack);
        }
        u64 before = inter->ctx.node_stack.count;
        parse_expr(inter, TOKEN_SEMICOLON);
        u64 count = inter->ctx.node_stack.count - before;
        if (count == 0) {
            // no expr
            Error err = {};
            err.err_string = str_lit("No expression after definition");
            err.token_id = def->token_index;
            err.has_token = true;
            dynarray_append(&inter->errors, err);
            return;
        }
        def->nodes[def->node_count - 1] = dynarray_pop(&inter->ctx.node_stack);

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
        def->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, sizeof(*def->nodes));

        if (inter->ctx.node_stack.count == 0) {
            Error err = {};
            err.err_string = str_lit("Cannot have empty expression in variable definition");
            dynarray_append(&inter->errors, err);
            return;
        }
        def->nodes[0] = dynarray_pop(&inter->ctx.node_stack);

    }
    dynarray_append(&inter->ctx.node_stack, def);
}


void parse_statement(Interpreter *inter) {

    Node *stmt = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*stmt));
    stmt->type = NODE_STATEMENT;
    stmt->token_index = inter->ctx.iter;
    stmt->node_count = 1;
    stmt->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, stmt->node_count * sizeof(*stmt->nodes));

    bool is_definition = false;

    for (u64 i = 0; i < inter->lex.tokens.count - inter->ctx.iter; ++i) {
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

    stmt->nodes[0] = dynarray_pop(&inter->ctx.node_stack);
    dynarray_append(&inter->ctx.node_stack, stmt);
}


void parse(Interpreter *inter) {
    Node *prog = (Node *)arena_alloc(&inter->ctx.node_arena, sizeof(*prog));
    prog->type = NODE_PROGRAM;
    prog->token_index = inter->ctx.iter;

    while (inter->ctx.iter < inter->lex.tokens.count) {
        parse_statement(inter);
        if (inter->errors.count > 0) return;

        if (!is_token(inter, TOKEN_SEMICOLON, 0)) {
            todo();
        }
        consume(inter);
    }

    prog->node_count = inter->ctx.node_stack.count;
    prog->nodes = (Node **)arena_alloc(&inter->ctx.node_arena, prog->node_count * sizeof(*prog->nodes));

    for (u64 i = prog->node_count; i-- > 0;) {
        prog->nodes[i] = dynarray_pop(&inter->ctx.node_stack);
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
        Token t = inter->lex.tokens.dat[top->token_index];
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
                    Error err = {};
                    err.err_string = str_lit("Too many arguments in function ");
                    err.token_id = n->token_index;
                    err.has_token = true;
                    dynarray_append(&inter->errors, err);
                    return;
                }
                if (n->node_count < item->func_args) {
                    // too few args
                    Error err = {};
                    err.err_string = str_lit("Too few arguments in function ");
                    err.token_id = n->token_index;
                    err.has_token = true;
                    dynarray_append(&inter->errors, err);
                    return;
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
                    Error err = {};
                    err.token_id = n->token_index;
                    err.has_token = true;
                    err.err_string = str_lit("Tried to use non variable as a variable");
                    dynarray_append(&inter->errors, err);
                    return;
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

void reset_interpreter(Interpreter *inter) {
    inter->src = {};

    inter->lex.tokens.count = 0;
    inter->lex.iter = 0;

    inter->ctx.node_stack.count = 0;
    inter->ctx.op_stack.count = 0;
    inter->ctx.iter = 0;
    inter->ctx.root = nullptr;


    inter->bytecode.count = 0;
    inter->symbol_ids.count = 0;
    inter->symbols.count = 0;
    arena_clear(&inter->func_arena);
    arena_clear(&inter->ctx.node_arena);

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



union V2f32 {
    f32 v[2];
    struct {
        f32 x, y;
    };

};

#define make_V2f32(x, y) V2f32 {{x, y}}

union V4u8 {
    u8 v[4];
    struct {
        u8 x, y, z, w;
    };
};

union V4f32 {
    f32 v[4];
    struct {
        f32 x, y, z, w;
    }; 
};

#define make_V4f32(x, y, z, w) V4f32 {{x, y, z, w}}


// UI System

#define TEXT_INPUT_FONT_SIZE 30
#define TEXT_INPUT_MARGIN 5
#define TEXT_INPUT_CURSOR_COLOR make_V4f32(0, 0, 0.0f, 1.0f)
#define TEXT_INPUT_SELECTION_COLOR make_V4f32(0, 0, 1.0f, 1.0f)
#define TEXT_INPUT_BACKGROUND_COLOR make_V4f32(0, 0.39f, 0, 1.0f)

#define DISPLAY_STRING_FONT_SIZE 15

#define PANE_MARGIN 2

#define DRAG_BAR_HEIGHT 15
#define DRAG_BAR_COLOR make_V4f32(0, 1.0f, 0, 1.0f)


const u64 nil_id = 0;

struct Ui_Event {
    bool text_input_changed;
};

struct UI_Pane {
    u64 hash;


    u64 flags;

    bool is_parent;

    V4f32 background_color;


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

    Input *input;

    bool active;
    u64 active_id;

    MouseAction mouse_action;
    MouseCursor display_mouse;

    u64 drag_id;
    f32 drag_x_offset;
    f32 drag_y_offset;

    u64 resize_id;
    f32 resize_x;
    f32 resize_y;
    f32 resize_w;
    f32 resize_h;
    V2f32 resize_pos;



    bool text_cursor;
    u64 cursor_pos;

    bool selecting;
    u64 selection_anchor;
    u64 selection_start;
    u64 selection_end;





    DynArray<UI_Pane> ui_panes[2];
    u64 active_panes_id;


    DynArray<u64> parent_stack;


    u8 parent_depth;
};

u32 quad_shader = 0;
u32 text_shader = 0;

bool mouse_collides(Input *input, f32 x, f32 y, f32 w, f32 h) {
    f32 mx = (f32)input->mx;
    f32 my = (f32)input->my;

    bool x_intercept = mx >= x && mx <= x + w;
    bool y_intercept = my >= y && my <= y + h;

    return x_intercept && y_intercept;
}


void push_parent(UI_State *ui) {
    DynArray<UI_Pane> *panes = ui->ui_panes + ui->active_panes_id; 
    assert(panes->count > 0);
    u64 id = panes->count - 1;
    panes->dat[id].is_parent = true;
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
stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
u32 ftex;

f32 font_pixel_size;

u32 screen_w = 1366;
u32 screen_h = 768;

V2f32 screen_space_to_gl(V2f32 v) {

    V2f32 new_v = {};
    new_v.x = 2 * v.x / (f32)screen_w - 1;
    new_v.y = -(2 * v.y / (f32)screen_h - 1);

    return new_v;
}

V2f32 gl_to_screen_space(V2f32 v) {

    V2f32 new_v = {};
    new_v.x = ((f32)screen_w * (v.x + 1)) / 2;
    new_v.y = ((f32)screen_h * (v.y + 1)) / 2;

    return new_v;
}

void get_baked_quad(const stbtt_bakedchar *chardata, u32 pw, u32 ph, s32 char_index, f32 *xpos, f32 *ypos, f32 scale, stbtt_aligned_quad *q) {

   const stbtt_bakedchar *b = chardata + char_index;
   f32 round_x = floorf((*xpos + b->xoff * scale) + 0.5f);
   f32 round_y = floorf((*ypos + b->yoff * scale) + 0.5f);

   q->x0 = round_x;
   q->y0 = round_y;
   q->x1 = round_x + (b->x1 - b->x0) * scale;
   q->y1 = round_y + (b->y1 - b->y0) * scale;



   f32 ipw = 1.0f / (f32)pw;
   f32 iph = 1.0f / (f32)ph;
   q->s0 = b->x0 * ipw;
   q->t0 = b->y0 * iph;
   q->s1 = b->x1 * ipw;
   q->t1 = b->y1 * iph;

   *xpos += b->xadvance * scale;
}


f32 measure_text(String text, f32 size) {

    f32 scale = size / font_pixel_size;

    f32 x = 0;
    f32 y = 0;
    for (u64 i = 0; i < text.count; ++i) {
        if (text.dat[i] >= 32 && text.dat[i] < 128) {
            stbtt_aligned_quad q;
            get_baked_quad(cdata, 512, 512, text.dat[i] - 32, &x, &y, scale, &q);
        }
    }
    return x;
}

u64 get_text_cursor_pos_from_mouse(u8 *text_buf, u64 *text_count, UI_Pane *p, f32 mx) {

    for (u64 j = 0; j <= *text_count; ++j) {

        u64 tmp = arena_get_pos(scratch);

        String s = string_printf(scratch, "%.*s", (int)j, text_buf);
        f32 sz = measure_text(s, TEXT_INPUT_FONT_SIZE);

        arena_set_pos(scratch, tmp);

        if (mx - (p->x + TEXT_INPUT_MARGIN + sz) < TEXT_INPUT_FONT_SIZE / 2.0f) {
            return j + 1;
        }
    }
    if (*text_count > 0) return *text_count;
    else return 0;
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

Ui_Event create_pane(UI_State *ui, u64 flags, u64 hash, f32 x, f32 y, f32 w, f32 h, V4f32 background_color, u8 *text_buf, u64 *text_count, u64 text_capacity) {
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
    pane.h_offset = 0;
    pane.background_color = background_color;
    pane.parent_id = get_parent_id(ui);

    dynarray_append(ui->ui_panes + ui->active_panes_id, pane);


    return pane.event;
}

struct QuadData {
    u32 vao;
    u32 vbo_coords;
    u32 vbo_texcoords;
    u32 ibo;
    V2f32 positions[4];
    V2f32 tex_coords[4];
    u32 indicies[6];

};

QuadData quad_data = {
    0, 0, 0, 0,
    {
        make_V2f32(-0.5f, -0.5f),
        make_V2f32(0.5f, -0.5f),
        make_V2f32(0.5f, 0.5f),
        make_V2f32(-0.5f, 0.5f),
    },
    {
        make_V2f32(0.0f, 0.0f),
        make_V2f32(1.0f, 0.0f),
        make_V2f32(1.0f, 1.0f),
        make_V2f32(0.0f, 1.0f),
    },
    {
        0, 1, 2,
        2, 3, 0,
    }
};

void draw_quad(V2f32 pos0, V2f32 pos1, V2f32 uv0, V2f32 uv1, u32 shader, V4f32 color) {

    V2f32 positions[] = {
        screen_space_to_gl(pos0),
        screen_space_to_gl(make_V2f32(pos1.x, pos0.y)),
        screen_space_to_gl(pos1),
        screen_space_to_gl(make_V2f32(pos0.x, pos1.y)),
    };

    V2f32 tex_coords[] = {
        uv0,
        make_V2f32(uv1.x, uv0.y),
        uv1,
        make_V2f32(uv0.x, uv1.y),
    };

    glBindBuffer(GL_ARRAY_BUFFER, quad_data.vbo_coords);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

    glBindBuffer(GL_ARRAY_BUFFER, quad_data.vbo_texcoords);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tex_coords), tex_coords);


    glBindVertexArray(quad_data.vao);
    glUseProgram(shader);
    glUniform4f(glGetUniformLocation(shader, "u_color"), color.x, color.y, color.z, color.w);
    glDrawElements(GL_TRIANGLES, ARRAY_SIZE(quad_data.indicies), GL_UNSIGNED_INT, nullptr);
}

void draw_rectangle(f32 x, f32 y, f32 w, f32 h, V4f32 color) {
    draw_quad(make_V2f32(x, y), make_V2f32(x + w, y + h), make_V2f32(0, 0), make_V2f32(1, 1), quad_shader, color);
}


void init_font_texture(String path, f32 font_pixel_height) {
    
    u64 tmp = arena_get_pos(scratch);
    u8 *tmp_bitmap = (u8 *)arena_alloc(scratch, 512*512);
    u8 *ttf_buf = (u8 *)arena_alloc(scratch, 1<<20);

    fread(ttf_buf, 1, 1<<20, fopen((char *)path.dat, "rb"));
    font_pixel_size = font_pixel_height;
    stbtt_BakeFontBitmap(ttf_buf, 0, font_pixel_size, tmp_bitmap, 512, 512, 32, 96, cdata); // no guarantee this fits!

    u32 tex = 0;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, tmp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       

    arena_set_pos(scratch, tmp);
    ftex = tex;
}


void draw_text(String text, f32 x, f32 y, f32 size, V4f32 color) {

    f32 scale = size / font_pixel_size;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (u64 i = 0; i < text.count; ++i) {
        if (text.dat[i] >= 32 && text.dat[i] < 128) {
            stbtt_aligned_quad q;
            get_baked_quad(cdata, 512, 512, text.dat[i] - 32, &x, &y, scale, &q);
            draw_quad(make_V2f32(q.x0, q.y0), make_V2f32(q.x1, q.y1), make_V2f32(q.s0, q.t0), make_V2f32(q.s1, q.t1), text_shader, color);
        }
    }
    glDisable(GL_BLEND);
}

void draw_ui(UI_State *ui) {

    DynArray<UI_Pane> *panes = ui->ui_panes + ui->active_panes_id;

    for (u64 i = 0; i < panes->count; ++i) {

        UI_Pane *pane = panes->dat + i;

        if (pane->is_parent) {
            if (pane->parent_id == nil_id) {
                ui->parent_depth = 0;
            }

            glStencilMask(0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
            f32 x = pane->x + PANE_MARGIN;
            f32 y = pane->y + PANE_MARGIN;
            f32 w = pane->w - PANE_MARGIN;
            f32 h = pane->h - PANE_MARGIN;

            draw_quad(make_V2f32(x, y), make_V2f32(pane->x + w, pane->y + h), make_V2f32(0, 0), make_V2f32(1, 1), quad_shader, make_V4f32(1, 1, 1, 1));
            glStencilMask(0);
            ui->parent_depth += 1;
        }

        if (pane->parent_id != nil_id) {
            glStencilFunc(GL_EQUAL, ui->parent_depth, 0xFF);
        }

        if (has_flags(pane->flags, PANE_BACKGROUND_COLOR)) {
            draw_rectangle(pane->x, pane->y, pane->w, pane->h, pane->background_color);
        }

        if (has_flags(pane->flags, PANE_DRAGGABLE)) {
            draw_rectangle(pane->x, pane->y + PANE_MARGIN, pane->w, DRAG_BAR_HEIGHT, DRAG_BAR_COLOR);
        }

        if (has_flags(pane->flags, PANE_TEXT_INPUT)) {
            f32 h_offset = pane->y;


            if (ui->active && ui->active_id == pane->hash && ui->text_cursor) {
                String s = string_printf(scratch, "%.*s", (int)ui->cursor_pos, pane->text_buf);
                f32 sz = measure_text(s, TEXT_INPUT_FONT_SIZE);

                f32 bar_size = TEXT_INPUT_FONT_SIZE - 8;
                if (bar_size < 8) {
                    bar_size = 8;
                }
                draw_rectangle(pane->x + TEXT_INPUT_MARGIN + (f32)sz, h_offset + bar_size / 2.0f, 1, bar_size, TEXT_INPUT_CURSOR_COLOR);

                if (ui->selecting) {

                    String s1 = string_printf(scratch, "%.*s", (int)ui->selection_start, pane->text_buf);
                    f32 sz2 = measure_text(s1, TEXT_INPUT_FONT_SIZE);

                    String s2 = string_printf(scratch, "%.*s", (int)(ui->selection_end - ui->selection_start), pane->text_buf + ui->selection_start);
                    f32 selection_sz = measure_text(s2, TEXT_INPUT_FONT_SIZE);
                    V4f32 a = TEXT_INPUT_SELECTION_COLOR;
                    a.x *= 1.5f;
                    a.y *= 1.5f;
                    a.z *= 1.5f;
                    draw_rectangle(pane->x + TEXT_INPUT_MARGIN + (f32)sz2, h_offset + bar_size / 2.0f, (f32)selection_sz, bar_size, a);

                }
            }

        }

        if (has_flags(pane->flags, PANE_TEXT_DISPLAY)) {
            String s = string_printf(scratch, "%.*s", (int)*pane->text_count, pane->text_buf);
            draw_text(s, pane->x + TEXT_INPUT_MARGIN, pane->y + TEXT_INPUT_FONT_SIZE, TEXT_INPUT_FONT_SIZE, make_V4f32(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (pane->parent_id != nil_id) {
            glStencilFunc(GL_EQUAL, 0, 0x0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }

    }
}

f32 max(f32 a, f32 b) {
    if (a > b) return a;
    return b;
}

f32 min(f32 a, f32 b) {
    if (a < b) return a;
    return b;
}

void set_resizing(UI_State *ui, UI_Pane *pane, V2f32 pos) {
    ui->mouse_action = MOUSE_ACTION_RESIZING;
    ui->resize_id = pane->hash;
    ui->resize_x = ui->input->mx;
    ui->resize_y = ui->input->my;
    ui->resize_w = pane->w;
    ui->resize_h = pane->h;
    ui->resize_pos = pos;
}



void update_panes(UI_State *ui) {

    if (!button_down(ui->input->buttons + BUTTON_ML)) {
        ui->display_mouse = MOUSE_CURSOR_ARROW;
    }

    DynArray<UI_Pane> *panes = ui->ui_panes + ui->active_panes_id;

    for (u64 i = 0; i < panes->count; ++i) {
        UI_Pane *pane = panes->dat + i;


        if (has_flags(pane->flags, PANE_DRAGGABLE)) {
            if (ui->mouse_action == MOUSE_ACTION_DRAGGING && ui->drag_id == pane->hash && button_released(ui->input->buttons + BUTTON_ML)) {
                ui->mouse_action = MOUSE_ACTION_NONE;
            }

            if (ui->mouse_action == MOUSE_ACTION_DRAGGING && ui->drag_id == pane->hash) {
                pane->x = ui->input->mx - ui->drag_x_offset;
                pane->y = ui->input->my - ui->drag_y_offset;
            }
            if (mouse_collides(ui->input, pane->x, pane->y + PANE_MARGIN, pane->w, DRAG_BAR_HEIGHT)) {
                ui->display_mouse = MOUSE_CURSOR_HAND;
                if (get_button_presses(ui->input->buttons + BUTTON_ML) > 0) {
                    ui->mouse_action = MOUSE_ACTION_DRAGGING;
                    ui->drag_id = pane->hash;
                    ui->drag_x_offset = ui->input->mx - pane->x;
                    ui->drag_y_offset = ui->input->my - pane->y;
                }
            }
            pane->h_offset += DRAG_BAR_HEIGHT + 2 * PANE_MARGIN;
        }

        // if (has_flags(pane->flags, PANE_SCROLL)) {
        //     todo();
        // }


        if (has_flags(pane->flags, PANE_RESIZEABLE)) {

            if (ui->mouse_action == MOUSE_ACTION_RESIZING && ui->resize_id == pane->hash && button_released(ui->input->buttons + BUTTON_ML)) {
                ui->mouse_action = MOUSE_ACTION_NONE;
            }

            if (ui->mouse_action == MOUSE_ACTION_RESIZING && ui->resize_id == pane->hash) {
                
                if (ui->resize_pos.x < 0) {
                    pane->x = ui->input->mx;
                    pane->w = ui->resize_w + ui->resize_x - ui->input->mx;

                } else if (ui->resize_pos.x > 0) {
                    pane->w = ui->resize_w + ui->input->mx - ui->resize_x;
                } 

                if (ui->resize_pos.y < 0) {
                    pane->y = ui->input->my;
                    pane->h = ui->resize_h + ui->resize_y - ui->input->my;
                } else if (ui->resize_pos.y > 0) {
                    pane->h = ui->resize_h + ui->input->my - ui->resize_y;
                }
            }

            if (ui->mouse_action != MOUSE_ACTION_RESIZING) {
                struct Value {f32 x, y, w, h, dx, dy;} table[] = {
                    {pane->x, pane->y, pane->w, PANE_MARGIN, 0, -1},
                    {pane->x, pane->y + pane->h - PANE_MARGIN, pane->w, PANE_MARGIN, 0, 1},
                    {pane->x + pane->w - PANE_MARGIN, pane->y, PANE_MARGIN, pane->h, 1, 0},
                    {pane->x, pane->y, PANE_MARGIN, pane->h, -1, 0},
                };
                f32 dx = 0;
                f32 dy = 0;
                for (u64 j = 0; j < ARRAY_SIZE(table); ++j) {
                    Value *v = table + j;
                    if (mouse_collides(ui->input, v->x, v->y, v->w, v->h)) {
                        dx += v->dx;
                        dy += v->dy;
                    }
                }
                if (dx != 0 || dy != 0) {
                    
                    if (dx == 0 && dy != 0) {
                        ui->display_mouse = MOUSE_CURSOR_RESIZE_NS;
                    } else if (dy == 0 && dx != 0) {
                        ui->display_mouse = MOUSE_CURSOR_RESIZE_WE;
                    } else if (dx != 0 && dx == dy) {
                        ui->display_mouse = MOUSE_CURSOR_RESIZE_NWSE;
                    } else if (dx != 0 && dx == -dy) {
                        ui->display_mouse = MOUSE_CURSOR_RESIZE_NESW;
                    }

                    if (ui->mouse_action != MOUSE_ACTION_RESIZING && get_button_presses(ui->input->buttons + BUTTON_ML) > 0) {
                        set_resizing(ui, pane, make_V2f32(dx, dy));
                    }
                }
            }
        }

        bool inside_parent_active_area = true;
        if (pane->parent_id != nil_id) {
            inside_parent_active_area = false;
            UI_Pane *parent = panes->dat + pane->parent_id;
            if (mouse_collides(ui->input, parent->x + PANE_MARGIN, parent->y + PANE_MARGIN, parent->w - 2 * PANE_MARGIN, parent->h - 2 * PANE_MARGIN)) {
                inside_parent_active_area = true;
            }
            // assuming downwards layout
            pane->x = PANE_MARGIN + parent->x;
            pane->y = parent->y + parent->h_offset;
            pane->w = parent->w - 2 * PANE_MARGIN;
            parent->h_offset += pane->h + PANE_MARGIN;
        }

        if (has_flags(pane->flags, PANE_TEXT_INPUT)) {

            if (inside_parent_active_area && mouse_collides(ui->input, pane->x, pane->y, pane->w, pane->h)) {
                ui->display_mouse = MOUSE_CURSOR_IBEAM;
                if (get_button_presses(ui->input->buttons + BUTTON_ML) > 0) {

                    if (ui->active && ui->active_id == pane->hash && ui->text_cursor) {

                        ui->selecting = false;
                        if (ui->input->mx < pane->x) {
                            ui->cursor_pos = 0;
                        } else if (ui->input->mx > pane->x + pane->w) {
                            ui->cursor_pos = *pane->text_count - 1;
                        } else {
                            ui->cursor_pos = get_text_cursor_pos_from_mouse(pane->text_buf, pane->text_count, pane, ui->input->mx);
                        }

                    } else {
                        ui->selecting = false;
                        ui->text_cursor = true;
                        ui->cursor_pos = get_text_cursor_pos_from_mouse(pane->text_buf, pane->text_count, pane, ui->input->mx);
                        ui->active_id = pane->hash;
                        ui->active = true;
                    }
                }
            }
            pane->event.text_input_changed = false;
            if (ui->active && ui->active_id == pane->hash && ui->text_cursor) {

                pane->event.text_input_changed = ui->input->key_count > 0 || ui->input->char_count > 0;

                for (u64 j = 0; j < ui->input->key_count; ++j) {

                    u64 prev_cursor_pos = ui->cursor_pos;

                    if (ui->input->keys[j] == BUTTON_BACKSPACE) {

                        if (*pane->text_count > 0) {
                            if (ui->selecting) {
                                ui->selecting = false;

                                shift_left_resize(pane->text_buf, pane->text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                                ui->cursor_pos = ui->selection_start;
                            } else {
                                if (ui->cursor_pos > 0) {

                                    if (ui->input->buttons[BUTTON_CTRL].ended_down) {

                                        if (is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                            while (ui->cursor_pos > 0 && is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                                ui->cursor_pos -= 1;
                                            }
                                            while (ui->cursor_pos > 0 && !is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                                ui->cursor_pos -= 1;
                                            }
                                        } else {
                                            while (ui->cursor_pos > 0 && !is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                                ui->cursor_pos -= 1;
                                            }
                                        }

                                        shift_left_resize(pane->text_buf, pane->text_count, ui->cursor_pos, prev_cursor_pos - ui->cursor_pos);
                                    } else {
                                        shift_left_resize(pane->text_buf, pane->text_count, ui->cursor_pos - 1, 1);
                                        ui->cursor_pos -= 1;
                                    }
                                }
                            }
                        }

                    } else if (ui->input->keys[j] == BUTTON_DELETE) {

                        if (*pane->text_count > 0) {
                            if (ui->selecting) {
                                ui->selecting = false;

                                shift_left_resize(pane->text_buf, pane->text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                                ui->cursor_pos = ui->selection_start;
                            } else {
                                if (ui->cursor_pos < *pane->text_count) {
                                    shift_left_resize(pane->text_buf, pane->text_count, ui->cursor_pos, 1);
                                }
                            }
                        }

                    } else if (ui->input->keys[j] == BUTTON_A) {
                        if (*pane->text_count > 0 && ui->input->buttons[BUTTON_CTRL].ended_down) {
                            ui->selecting = true;
                            ui->cursor_pos = 0;
                            ui->selection_anchor = *pane->text_count;
                        }
                    } else if (ui->input->keys[j] == BUTTON_X) {
                        if (ui->selecting && ui->input->buttons[BUTTON_CTRL].ended_down) {
                            ui->selecting = false;
                            String s = string_printf(scratch, "%.*s", (int)(ui->selection_end - ui->selection_start), ui->selection_start + pane->text_buf);
                            set_clipboard_text(s);
                            shift_left_resize(pane->text_buf, pane->text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                            ui->cursor_pos = ui->selection_start;
                        }
                    } else if (ui->input->keys[j] == BUTTON_C) {
                        if (ui->selecting && ui->input->buttons[BUTTON_CTRL].ended_down) {
                            String s = string_printf(scratch, "%.*s", (int)(ui->selection_end - ui->selection_start), ui->selection_start + pane->text_buf);
                            set_clipboard_text(s);
                        }
                    } else if (ui->input->keys[j] == BUTTON_V) {
                        if (ui->input->buttons[BUTTON_CTRL].ended_down) {
                            String text = get_clipboard_text();
                            u64 len = text.count;

                            if (len + *pane->text_count - (ui->selection_end - ui->selection_start) > pane->text_capacity) {
                                // pasting clipboard would overflow
                                todo();
                            } else {
                                if (ui->selecting) {
                                    ui->selecting = false;
                                    shift_left_resize(pane->text_buf, pane->text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                                    ui->cursor_pos = ui->selection_start;
                                }
                                shift_right_resize(pane->text_buf, pane->text_count, ui->cursor_pos, len);
                                memcpy(pane->text_buf + ui->cursor_pos, text.dat, len);
                                ui->cursor_pos += len;
                            }

                        }
                    } else if (ui->input->keys[j] == BUTTON_HOME) {
                        if (*pane->text_count > 0) {
                            if (!ui->selecting && ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = true;
                                ui->selection_anchor = prev_cursor_pos;
                            }
                            if (ui->selecting && !ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = false;
                            }
                            ui->cursor_pos = 0;
                        }
                    } else if (ui->input->keys[j] == BUTTON_END) {
                        if (*pane->text_count > 0) {
                            if (!ui->selecting && ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = true;
                                ui->selection_anchor = prev_cursor_pos;
                            }
                            if (ui->selecting && !ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = false;
                            }
                            ui->cursor_pos = *pane->text_count;
                        }
                    } else if (ui->input->keys[j] == BUTTON_LEFT) {
                        if (*pane->text_count > 0 && ui->cursor_pos > 0) {

                            if (!ui->selecting && ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = true;
                                ui->selection_anchor = prev_cursor_pos;
                            }
                            if (ui->selecting && !ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = false;
                            }

                            if (ui->input->buttons[BUTTON_CTRL].ended_down) {
                                if (is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                    while (ui->cursor_pos > 0 && is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                        ui->cursor_pos -= 1;
                                    }
                                    while (ui->cursor_pos > 0 && !is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                        ui->cursor_pos -= 1;
                                    }
                                } else {
                                    while (ui->cursor_pos > 0 && !is_whitespace(pane->text_buf[ui->cursor_pos - 1])) {
                                        ui->cursor_pos -= 1;
                                    }
                                }
                            } else {
                                ui->cursor_pos -= 1;
                            }

                        }

                    } else if (ui->input->keys[j] == BUTTON_RIGHT) {
                        if (*pane->text_count > 0 && ui->cursor_pos < *pane->text_count) {

                            if (!ui->selecting && ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = true;
                                ui->selection_anchor = prev_cursor_pos;
                            }
                            if (ui->selecting && !ui->input->buttons[BUTTON_SHIFT].ended_down) {
                                ui->selecting = false;
                            }

                            if (ui->input->buttons[BUTTON_CTRL].ended_down) {
                                if (is_whitespace(pane->text_buf[ui->cursor_pos])) {
                                    while (ui->cursor_pos < *pane->text_count && is_whitespace(pane->text_buf[ui->cursor_pos])) {
                                        ui->cursor_pos += 1;
                                    }
                                    while (ui->cursor_pos < *pane->text_count && !is_whitespace(pane->text_buf[ui->cursor_pos])) {
                                        ui->cursor_pos += 1;
                                    }
                                } else {
                                    while (ui->cursor_pos < *pane->text_count && !is_whitespace(pane->text_buf[ui->cursor_pos])) {
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
                for (u64 j = 0; j < ui->input->char_count && *pane->text_count < pane->text_capacity - 1; ++j) {

                    u32 character = ui->input->chars[j];
                    if (character >= 32 && character < 127) {
                        if (ui->selecting) {
                            ui->selecting = false;

                            shift_left_resize(pane->text_buf, pane->text_count, ui->selection_start, ui->selection_end - ui->selection_start);
                            ui->cursor_pos = ui->selection_start;
                        }

                        shift_right_resize(pane->text_buf, pane->text_count, ui->cursor_pos, 1);
                        pane->text_buf[ui->cursor_pos] = (u8)character;
                        ui->cursor_pos += 1;
                    }
                }
            }
        }
    }
}

void begin_ui(UI_State *ui) {
    ui->active_panes_id += 1;
    ui->active_panes_id %= 2;
    ui->ui_panes[ui->active_panes_id].count = 0;
    // add dummy pane at id zero
    dynarray_append(&ui->ui_panes[ui->active_panes_id], {});
}

void end_ui(UI_State *ui) {
    update_panes(ui);
    set_mouse_cursor(ui->display_mouse);
}




void test() {
    static Interpreter test_inter = {};
    // String src = str_lit("f(x, y):=x*y;f(1,2);");
    String src = str_lit("(5+5+5);");

    arena_init(&test_inter.func_arena, 100000);
    arena_init(&test_inter.ctx.node_arena, 100000);
    compile(&test_inter, src);
    for (u64 i = 0; i < test_inter.errors.count; ++i) {
        Error *err = test_inter.errors.dat + i;
        String s = err->err_string;
        printf("ERROR: %.*s ", (s32)s.count, s.dat);

        if (err->has_token) {
            String tok = string_from_token(&test_inter, err->token_id);
            printf("`%.*s`", (s32)tok.count, tok.dat);
        }


        printf("\n");
    }
    print_bytecode(&test_inter.bytecode);
    bool r = execute(&test_inter, str_lit("_s2"), nullptr, 0);
    printf("r = %s\n", r ? "true" : "false");
    if (r) {
        printf("Result = %g\n", test_inter.stack.dat[test_inter.stack.count - 1].f);
    }
}






u32 compile_shader(String src, u32 shader_type) {

    u32 shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char **)&src.dat, nullptr);
    glCompileShader(shader);


    s32 compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        s32 len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        // TODO: actually handle memory
        char *log = (char *)malloc((u64)len);
        glGetShaderInfoLog(shader, len, nullptr, log);
        LOG_ERROR("Failed to compile shader %.*s because of %s\n", (s32)src.count, src.dat, log);
        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

u32 create_glshader(String vsrc, String fsrc) {

    u32 fshader = compile_shader(fsrc, GL_FRAGMENT_SHADER);
    u32 vshader = compile_shader(vsrc, GL_VERTEX_SHADER);


    u32 program = 0;
    if (fshader && vshader) {
        program = glCreateProgram();

        glAttachShader(program, fshader);
        glAttachShader(program, vshader);

        glLinkProgram(program);
        glValidateProgram(program);

        s32 valid = 0;
        glGetProgramiv(program, GL_VALIDATE_STATUS, &valid);
        if (!valid) {
            s32 len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
            // TODO: actually handle memory
            char *log = (char *)malloc((u64)len);
            glGetProgramInfoLog(program, len, nullptr, log);
            LOG_ERROR("Shader failed validation because of %s\n", log);
            glDeleteProgram(program);
            program = 0;
        }
    } else {
        if (fshader) glDeleteShader(fshader);
        if (vshader) glDeleteShader(vshader);
    }
    // TODO: maybe handle all error paths


    // removes source code and stuff from gpu
    #if 0
    glDeleteShader(fshader);
    glDeleteShader(vshader);

    glDetachShader(program, fshader);
    glDetachShader(program, vshader);
    #endif
    return program;
}





String quad_vertex_shader = str_lit(R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

void main() {
    gl_Position = vec4(position, 0, 1);
    v_TexCoord = texCoord;
}
)");

String quad_frag_shader = str_lit(R"(
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_color;
// in vec2 v_TexCoord;

// uniform sampler2D u_Texture;
void main() {
    color = u_color;
    // color = vec4(v_TexCoord, 0, 0);
}
)");

String quadstr_frag_shader = str_lit(R"(
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;
void main() {
    color = u_color * vec4(texture(u_Texture, v_TexCoord).xxxx);
}
)");








UI_State ui = {};
Interpreter inter = {};

int main(void) {

    Arena t; arena_init(&t, 10000000);
    scratch = &t;

    DynArray<u8> sb; dynarray_init(&sb, 65000);



    arena_init(&inter.func_arena, 100000);
    arena_init(&inter.ctx.node_arena, 100000);


    if (!create_window((s32)screen_w, (s32)screen_h, str_lit("Para"), &g_window)) return 1;
    if (!gladLoadGL()) {
        LOG_ERROR("Failed to load newer OpenGl functions\n");
        return 1;
    }

    LOG_INFO("OpenGl version %s\n", glGetString(GL_VERSION));
    LOG_INFO("OpenGl renderer %s\n", glGetString(GL_RENDERER));


    glGenVertexArrays(1, &quad_data.vao);
    glBindVertexArray(quad_data.vao);

    {
        glGenBuffers(1, &quad_data.vbo_coords);
        glBindBuffer(GL_ARRAY_BUFFER, quad_data.vbo_coords);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data.positions), quad_data.positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(V2f32), nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &quad_data.vbo_texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, quad_data.vbo_texcoords);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data.tex_coords), quad_data.tex_coords, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(V2f32), nullptr);
        glEnableVertexAttribArray(1);


        glGenBuffers(1, &quad_data.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_data.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_data.indicies), quad_data.indicies, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    quad_shader = create_glshader(quad_vertex_shader, quad_frag_shader);
    if (!quad_shader) return 1;
    text_shader = create_glshader(quad_vertex_shader, quadstr_frag_shader);
    if (!text_shader) return 1;


    // f32 x = 0;

    // while (true) {
    //     x += 0.01f;
    //     if (x > 1) x = 0;
    //     get_inputs(&g_window);

    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //     glClearColor(0.0f, 0.5f, 0.0f, 1.0f);

    //     draw_rectangle(100, 100, 200, 200, make_V4f32(0.23f, 0.23f, 0.23f, 1));

    //     swap_buffers(&g_window);
    // }

    // return 0;

    u8 text_buf[5][64] = {};
    u64 text_count[5] = {};

    u8 display_text_buf[5][64] = {};
    u64 display_text_count[5] = {};

    V4f32 light_gray = {};
    light_gray.x = 0.5f;
    light_gray.y = 0.5f;
    light_gray.z = 0.5f;
    light_gray.w = 1.0f;

    V4f32 red = {};
    red.x = 1.0f;
    red.y = 0;
    red.z = 0;
    red.w = 1.0f;

    V4f32 black = {};
    black.x = 0;
    black.y = 0;
    black.z = 0;
    black.w = 1.0f;

    V4f32 dark_green = {};
    dark_green.x = 0;
    dark_green.y = 0.4f;
    dark_green.z = 0;
    dark_green.w = 1.0f;


    init_font_texture(str_lit("c:/windows/fonts/times.ttf"), TEXT_INPUT_FONT_SIZE);

    bool running = true;
    while (running) {

        Input input = get_inputs(&g_window);
        for (u64 i = 0; i < input.char_count; ++i) {
            printf("char: %u\n", input.chars[i]);
        }
        for (u64 i = 0; i < input.key_count; ++i) {
            printf("key: %u\n", input.keys[i]);
        }
        ui.input = &input;

        screen_w = input.screen_width;
        screen_h = input.screen_height;
        u64 tmp_pos = arena_get_pos(scratch);

        begin_ui(&ui);
        {
            create_pane(&ui, PANE_DRAGGABLE|PANE_RESIZEABLE|PANE_BACKGROUND_COLOR, 69420'0, 0, 0, 400, 500, light_gray, nullptr, nullptr, 0);

            // text input + display string
            push_parent(&ui);
            for (u64 i = 0; i < 5; ++i) {
                Ui_Event event = create_pane(&ui, PANE_TEXT_INPUT|PANE_TEXT_DISPLAY|PANE_BACKGROUND_COLOR, 79420+i, 0, 0, 400, 35, dark_green, text_buf[i], text_count + i, sizeof(text_buf[i]));
                create_pane(&ui, PANE_TEXT_DISPLAY|PANE_BACKGROUND_COLOR, 80420+i, 0, 0, 400, 35, red, display_text_buf[i], display_text_count + i, sizeof(display_text_buf[i]));
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

                        reset_interpreter(&inter);
                        compile(&inter, src_);


                        u64 node_id = 0;
                        for (u64 j = 0; j < ARRAY_SIZE(text_count); ++j) {
                            if (text_count[j] == 0) continue;

                            bool skip = false;
                            if (inter.errors.count > 0) skip = true;
                            // while (inter->errors.count > 0) {
                            //     Error *e = inter->errors.dat + inter->errors.count - 1;
                            //     todo();
                            // }
                            if (!skip) {
                                Node *prog = inter.ctx.root;
                                Node *stmt = prog->nodes[node_id];

                                bool is_definition = false;
                                assert(stmt->node_count == 1);
                                if (stmt->nodes[0]->type == NODE_FUNCTIONDEF || stmt->nodes[0]->type == NODE_VARIABLEDEF) {
                                    is_definition = true;
                                }


                                if (is_definition) {
                                    // todo();
                                } else {
                                    String stmt_s = string_printf(&t, "_s%llu", node_id);
                                    execute(&inter, stmt_s, nullptr, 0);
                                    String s = string_printf(scratch, "%lg", dynarray_pop(&inter.stack));
                                    memcpy(display_text_buf[j], s.dat, s.count + 1);
                                    display_text_count[j] = s.count;
                                }
                                node_id += 1;
                            }
                        }
                    }
                }
            }
            pop_parent(&ui);
        }
        {
            create_pane(&ui, PANE_DRAGGABLE|PANE_BACKGROUND_COLOR, 1337420, 1366-800, 0, 800, 400, black, nullptr, nullptr, 0);
        }

        end_ui(&ui);


        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xff);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(0.23f, 0.23f, 0.23f, 0.0f);
        draw_ui(&ui);

        swap_buffers(&g_window);
        arena_set_pos(scratch, tmp_pos);
    }
    arena_clean(&t);
    scratch = nullptr;


    return 0;
}


