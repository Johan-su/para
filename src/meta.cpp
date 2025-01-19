

#define enumTable \
Y(TokenType) \
Y(NodeType) \


#define TokenTypeTable \
X(TOKEN_INVALID) \
X(TOKEN_NUMBER) \
X(TOKEN_PLUS) \
X(TOKEN_END) \

#define NodeTypeTable \
X(NODE_INVALID) \
X(NODE_NUMBER) \
X(NODE_ADD) \




#define X(field) field,
#define Y(name) \
enum name { \
    name##Table \
};

enumTable


#define X(field) case field: return #field;
#define Y(name) \
const char *str_##name(name t) { \
    switch (t) { \
        name##Table \
    } \
    return nullptr; \
}

enumTable
