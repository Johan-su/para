

#define enumFlagTable \
Y(TokenType) \



#define enumTable \
Y(NodeType) \


#define TokenTypeTable \
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




#define NodeTypeTable \
X(NODE_INVALID) \
X(NODE_PROGRAM) \
X(NODE_STATEMENT) \
X(NODE_NUMBER) \
X(NODE_FUNCTION) \
X(NODE_FUNCTIONDEF) \
X(NODE_VARIABLE) \
X(NODE_VARIABLEDEF) \
X(NODE_ADD) \
X(NODE_SUB) \
X(NODE_MUL) \
X(NODE_DIV) \
X(NODE_UNARYADD) \
X(NODE_UNARYSUB) \
X(NODE_OPENPAREN) \



#define X(field) field,
#define Y(name) \
enum name { \
    name##Table \
}; \

enumTable


#define X(field) #field,
#define Y(name) \
const char *str_##name[] = { \
    name##Table \
}; \


enumTable


#define X(field) field##_BIT_POS,
#define Y(name) \
enum name##_BIT_POS { \
    name##Table \
}; \

enumFlagTable


#define X(field) field = 1 << field##_BIT_POS,
#define Y(name) \
enum name { \
    name##Table \
}; \

enumFlagTable
