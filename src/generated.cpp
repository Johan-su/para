# 1 "../src/meta.cpp"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 386 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "../src/meta.cpp" 2
# 28 "../src/meta.cpp"
enum TokenType { TOKEN_INVALID, TOKEN_NUMBER, TOKEN_PLUS, TOKEN_END, }; enum NodeType { NODE_INVALID, NODE_NUMBER, NODE_ADD, };
# 40 "../src/meta.cpp"
const char *str_TokenType(TokenType t) { switch (t) { case TOKEN_INVALID: return "TOKEN_INVALID"; case TOKEN_NUMBER: return "TOKEN_NUMBER"; case TOKEN_PLUS: return "TOKEN_PLUS"; case TOKEN_END: return "TOKEN_END"; } return nullptr; } const char *str_NodeType(NodeType t) { switch (t) { case NODE_INVALID: return "NODE_INVALID"; case NODE_NUMBER: return "NODE_NUMBER"; case NODE_ADD: return "NODE_ADD"; } return nullptr; }
