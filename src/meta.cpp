

#define enumTable \
Y(TokenType) \
Y(NodeType) \


#define TokenTypeTable \
X(TOKEN_INVALID) \
X(TOKEN_NUMBER) \
X(TOKEN_PLUS) \
X(TOKEN_MINUS) \
X(TOKEN_STAR) \
X(TOKEN_SLASH) \
X(TOKEN_OPENPAREN) \
X(TOKEN_CLOSEPAREN) \

#define NodeTypeTable \
X(NODE_INVALID) \
X(NODE_NUMBER) \
X(NODE_ADD) \
X(NODE_SUB) \
X(NODE_MUL) \
X(NODE_DIV) \
X(NODE_UNARYADD) \
X(NODE_UNARYSUB) \



#define X(field) field,
#define Y(name) \
enum name { \
    name##Table \
};

enumTable


#define X(field) #field,
#define Y(name) \
const char *str_##name[] = { \
    name##Table \
}; \


enumTable
