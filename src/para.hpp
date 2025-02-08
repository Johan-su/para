
struct String {
    u8 *dat;
    u64 count;
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
    Token *tokens;
    u64 token_count;

    u64 iter;
};


struct Node {
    NodeType type;
    union {
        f64 num;
    };

    u64 token_index;

    u64 node_count;
    Node **nodes;
};

struct Parser {
    Node *node_stack[128];
    u64 stack_count;

    Node *op_stack[128];
    u64 op_count;

    u64 iter;

    Node *root;
};


struct Item {
    ItemType type;
    union {
        f64 val;
        Node *func;
    };
};


struct Item_Env {
    String ids[256];
    Item items[256];
    u64 item_count;
};

struct Error {
    u64 statement_id;

    u64 char_id;
    u64 token_id;
};


struct Interpreter {
    Item_Env envs[256];
    u64 env_count;

    String src;
    Lexer lex;
    Parser ctx;


    Stack_Nodep node_stack;
    Stack_f64 val_stack;

    Stack_Error error_stack;
};