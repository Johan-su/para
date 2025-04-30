#pragma once
#include "common.h"

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


// type, precedence, is_left_associative,
#define NodeDataTable(X) \
X(NODE_INVALID    , -1 , false) \
X(NODE_PROGRAM    , -1 , false) \
X(NODE_STATEMENT  , -1 , false) \
X(NODE_NUMBER     , -1 , false) \
X(NODE_FUNCTION   , -1 , false) \
X(NODE_FUNCTIONDEF, -1 , false) \
X(NODE_VARIABLE   , -1 , false) \
X(NODE_VARIABLEDEF, -1 , false) \
X(NODE_ADD        , 1  , true) \
X(NODE_SUB        , 1  , true) \
X(NODE_MUL        , 2  , true) \
X(NODE_DIV        , 2  , true) \
X(NODE_UNARYADD   , 100, false) \
X(NODE_UNARYSUB   , 101, false) \
X(NODE_OPENPAREN  , 0  , true) \


#define ItemTypeTable(X) \
X(ITEM_INVALID) \
X(ITEM_VARIABLE) \
X(ITEM_FUNCTION) \




#define GenEnumX(type, ...) type,
#define GenEnum(name, table) \
enum name { \
    table(GenEnumX) \
    name##_COUNT \
}; \
extern const char *str_##name[];



#define GenEnumSrcX(X, ...) #X,
#define GenEnumSrc(name, table) \
const char *str_##name[] { \
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

GenEnumFlag(TokenType, TokenTypeTable)

