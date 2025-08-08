#pragma once
#include "../common.h"
enum TokenType {
    TOKEN_INVALID = 1 << 0,
    TOKEN_NUMBER = 1 << 1,
    TOKEN_IDENTIFIER = 1 << 2,
    TOKEN_EQUAL = 1 << 3,
    TOKEN_SEMICOLON = 1 << 4,
    TOKEN_COLON = 1 << 5,
    TOKEN_COMMA = 1 << 6,
    TOKEN_PLUS = 1 << 7,
    TOKEN_MINUS = 1 << 8,
    TOKEN_STAR = 1 << 9,
    TOKEN_SLASH = 1 << 10,
    TOKEN_OPENPAREN = 1 << 11,
    TOKEN_CLOSEPAREN = 1 << 12,
};

enum BytecodeType {
    BYTECODE_INVALID,
    BYTECODE_CALL,
    BYTECODE_RETURN,
    BYTECODE_PUSH_ARG,
    BYTECODE_PUSH,
    BYTECODE_NEG,
    BYTECODE_ADD,
    BYTECODE_SUB,
    BYTECODE_MUL,
    BYTECODE_DIV,
};
#define BytecodeType_COUNT 10

enum NodeType {
    NODE_INVALID,
    NODE_PROGRAM,
    NODE_STATEMENT,
    NODE_NUMBER,
    NODE_FUNCTION,
    NODE_FUNCTIONDEF,
    NODE_VARIABLE,
    NODE_VARIABLEDEF,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_UNARYADD,
    NODE_UNARYSUB,
    NODE_OPENPAREN,
};
#define NodeType_COUNT 15

enum ItemType {
    ITEM_INVALID,
    ITEM_VARIABLE,
    ITEM_GLOBALVARIABLE,
    ITEM_FUNCTION,
};
#define ItemType_COUNT 4

enum PaneFlags {
    PANE_DRAGGABLE = 1 << 0,
    PANE_RESIZEABLE = 1 << 1,
    PANE_SCROLL = 1 << 2,
    PANE_TEXT_INPUT = 1 << 3,
    PANE_TEXT_DISPLAY = 1 << 4,
    PANE_BACKGROUND_COLOR = 1 << 5,
};

enum MouseAction {
    MOUSE_ACTION_NONE,
    MOUSE_ACTION_DRAGGING,
    MOUSE_ACTION_RESIZING,
};
#define MouseAction_COUNT 3

struct NodeTableData {
    NodeType node_type;
    s64 precedence;
    bool left_associative;
    bool is_expr;
};
extern NodeTableData node_data_table[15];
