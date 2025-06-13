#pragma once
#include "common.h"
#include "string.h"

#define TokenTypeTable(X) \
X(TOKEN_INVALID) \
X(TOKEN_NUMBER) \
X(TOKEN_IDENTIFIER) \
X(TOKEN_EQUAL) \
X(TOKEN_SEMICOLON) \
X(TOKEN_COLON) \
X(TOKEN_COMMA) \
X(TOKEN_PLUS) \
X(TOKEN_MINUS) \
X(TOKEN_STAR) \
X(TOKEN_SLASH) \
X(TOKEN_OPENPAREN) \
X(TOKEN_CLOSEPAREN) \

#define BytecodeTypeTable(X) \
X(BYTECODE_INVALID) \
X(BYTECODE_CALL) \
X(BYTECODE_RETURN) \
X(BYTECODE_PUSH_ARG) \
X(BYTECODE_PUSH) \
X(BYTECODE_NEG) \
X(BYTECODE_ADD) \
X(BYTECODE_SUB) \
X(BYTECODE_MUL) \
X(BYTECODE_DIV) \


// type, precedence, is_left_associative, is_expr,
#define NodeDataTable(X) \
X(NODE_INVALID    , -1 , false, false) \
X(NODE_PROGRAM    , -1 , false, false) \
X(NODE_STATEMENT  , -1 , false, false) \
X(NODE_NUMBER     , -1 , false, true) \
X(NODE_FUNCTION   , -1 , false, true) \
X(NODE_FUNCTIONDEF, -1 , false, false) \
X(NODE_VARIABLE   , -1 , false, true) \
X(NODE_VARIABLEDEF, -1 , false, false) \
X(NODE_ADD        , 1  , true, true) \
X(NODE_SUB        , 1  , true, true) \
X(NODE_MUL        , 2  , true, true) \
X(NODE_DIV        , 2  , true, true) \
X(NODE_UNARYADD   , 100, false, true) \
X(NODE_UNARYSUB   , 101, false, true) \
X(NODE_OPENPAREN  , 0  , true, false) \


#define ItemTypeTable(X) \
X(ITEM_INVALID) \
X(ITEM_VARIABLE) \
X(ITEM_GLOBALVARIABLE) \
X(ITEM_FUNCTION) \


#define PaneFlagsTable(X) \
X(PANE_DRAGGABLE) \
X(PANE_RESIZEABLE) \
X(PANE_TEXT_INPUT) \
X(PANE_TEXT_DISPLAY) \
X(PANE_BACKGROUND_COLOR) \

#define MouseActionsTable(X) \
X(MOUSE_ACTION_NONE) \
X(MOUSE_ACTION_DRAGGING) \
X(MOUSE_ACTION_RESIZING) \


#define GenEnumX(type, ...) type,
#define GenEnum(name, table) \
enum name { \
    table(GenEnumX) \
    name##_COUNT \
}; \
extern String str_##name[];



#define GenEnumSrcX(X, ...) String {(u8 *)#X, (sizeof(#X) - 1)},
#define GenEnumSrc(name, table) \
String str_##name[] { \
    table(GenEnumSrcX) \
}; \



#define GenEnumFlag1(X) BIT##X,
#define GenEnumFlag2(X) X = (1 << BIT##X), 
#define GenEnumFlag(name, table) \
enum BIT##name { \
    table(GenEnumFlag1) \
}; \
enum name { \
    table(GenEnumFlag2) \
}; \







GenEnum(NodeType, NodeDataTable)
GenEnum(BytecodeType, BytecodeTypeTable)
GenEnum(ItemType, ItemTypeTable)
GenEnum(MouseAction, MouseActionsTable)

GenEnumFlag(TokenType, TokenTypeTable)
GenEnumFlag(PaneFlags, PaneFlagsTable)
