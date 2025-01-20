# 1 "../src/meta.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 386 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "../src/meta.cpp" 2
# 34 "../src/meta.cpp"
enum TokenType { TOKEN_INVALID, TOKEN_NUMBER, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, }; enum NodeType { NODE_INVALID, NODE_NUMBER, NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV, NODE_UNARYADD, NODE_UNARYSUB, };
# 44 "../src/meta.cpp"
const char *str_TokenType[] = { "TOKEN_INVALID", "TOKEN_NUMBER", "TOKEN_PLUS", "TOKEN_MINUS", "TOKEN_STAR", "TOKEN_SLASH", }; const char *str_NodeType[] = { "NODE_INVALID", "NODE_NUMBER", "NODE_ADD", "NODE_SUB", "NODE_MUL", "NODE_DIV", "NODE_UNARYADD", "NODE_UNARYSUB", };
