#include "gen.h"
#include "../string.h"
String str_BytecodeType[] = {
    S("BYTECODE_INVALID"),
    S("BYTECODE_CALL"),
    S("BYTECODE_RETURN"),
    S("BYTECODE_PUSH_ARG"),
    S("BYTECODE_PUSH"),
    S("BYTECODE_NEG"),
    S("BYTECODE_ADD"),
    S("BYTECODE_SUB"),
    S("BYTECODE_MUL"),
    S("BYTECODE_DIV"),
};

String str_NodeType[] = {
    S("NODE_INVALID"),
    S("NODE_PROGRAM"),
    S("NODE_STATEMENT"),
    S("NODE_NUMBER"),
    S("NODE_FUNCTION"),
    S("NODE_FUNCTIONDEF"),
    S("NODE_VARIABLE"),
    S("NODE_VARIABLEDEF"),
    S("NODE_ADD"),
    S("NODE_SUB"),
    S("NODE_MUL"),
    S("NODE_DIV"),
    S("NODE_UNARYADD"),
    S("NODE_UNARYSUB"),
    S("NODE_OPENPAREN"),
};

String str_ItemType[] = {
    S("ITEM_INVALID"),
    S("ITEM_VARIABLE"),
    S("ITEM_GLOBALVARIABLE"),
    S("ITEM_FUNCTION"),
};

String str_MouseAction[] = {
    S("MOUSE_ACTION_NONE"),
    S("MOUSE_ACTION_DRAGGING"),
    S("MOUSE_ACTION_RESIZING"),
};

NodeTableData node_data_table[15] {
    {NODE_INVALID, -1, false, false, },
    {NODE_PROGRAM, -1, false, false, },
    {NODE_STATEMENT, -1, false, false, },
    {NODE_NUMBER, -1, false, true, },
    {NODE_FUNCTION, -1, false, true, },
    {NODE_FUNCTIONDEF, -1, false, false, },
    {NODE_VARIABLE, -1, false, true, },
    {NODE_VARIABLEDEF, -1, false, false, },
    {NODE_ADD, 1, true, true, },
    {NODE_SUB, 1, true, true, },
    {NODE_MUL, 2, true, true, },
    {NODE_DIV, 2, true, true, },
    {NODE_UNARYADD, 100, false, true, },
    {NODE_UNARYSUB, 101, false, true, },
    {NODE_OPENPAREN, 0, true, false, },
};

