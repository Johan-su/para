

#define enumFlagTable \
Y(TokenType) \



#define enumTable \
Y(NodeType) \
Y(BytecodeType) \
Y(ItemType) \


#define StackTable \
Y(f64, f64) \
Y(Nodep, Node *) \
Y(Error, Error) \
Y(Item, Item) \


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

#define BytecodeTypeTable \
X(BYTECODE_INVALID) \



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


#define ItemTypeTable \
X(ITEM_INVALID) \
X(ITEM_VARIABLE) \
X(ITEM_FUNCTION) \

#ifdef HEADER
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

#endif


#ifdef SOURCE
#define Y(type_simple, type) \
struct Stack_##type_simple { \
    u64 count; \
    u64 cap; \
    type *dat; \
}; \
void stack_init(Stack_##type_simple *stack, u64 cap) { \
    stack->count = 0; \
    stack->cap = cap; \
    stack->dat = (type *)calloc(stack->cap, sizeof(type)); \
} \
void stack_push(Stack_##type_simple *stack, type v) { \
    if (stack->cap == 0) stack_init(stack, 1 << 14); \
    assert(stack->count < stack->cap); \
    stack->dat[stack->count++] = v; \
} \
type stack_pop(Stack_##type_simple *stack) { \
    assert(stack->count > 0); \
    return stack->dat[--stack->count]; \
} \


StackTable


#endif