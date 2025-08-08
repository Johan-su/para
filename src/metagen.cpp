#include "common.h"
#include "arena.h"
#include "string.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool write_string_to_file(String file_path, String output) {
    bool result = false;
    HANDLE file_handle = CreateFile((char *)file_path.dat, GENERIC_ALL, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle != INVALID_HANDLE_VALUE) {
        if (WriteFile(file_handle, output.dat, (u32)output.count, nullptr, nullptr)) {
            result = true;
        } else {
            u32 err = GetLastError();
            LOG_ERROR("failed to write to file %u\n", err);
        }
        CloseHandle(file_handle);

    } else {
        u32 err = GetLastError();
        LOG_ERROR("Failed to open %.*s %u\n", (s32)file_path.count, file_path.dat, err);
    }
    return result;
}


u8 b[1000000];
Arena temp = {0, sizeof(b), b};
Arena *scratch = &temp;

u8 header_buf[1000000] = {};
u64 header_buf_count = 0;

u8 src_buf[1000000] = {};
u64 src_buf_count = 0;



struct Table {
    u64 column_count;
    String *columns;

    u64 value_count;
    String *values; // row_count = value_count / column_count
};


void append_to_header_buf(String s) {
    memcpy(header_buf + header_buf_count, s.dat, s.count);
    header_buf_count += s.count;
}

void append_to_src_buf(String s) {
    memcpy(src_buf + src_buf_count, s.dat, s.count);
    src_buf_count += s.count;
}

u64 get_column_id_from_str(Table *t, String column_name) {
    for (u64 i = 0; i < t->column_count; ++i) {
        if (string_equal(column_name, t->columns[i])) {
            return i;
        }
    }
    assert(false);
    return 0;
}

void gen_enum(Table *t, String enum_name, String column_name) {
    u64 row_count = t->value_count / t->column_count;

    u64 x = get_column_id_from_str(t, column_name);
    {
        String start = string_printf(scratch, "enum %.*s {\n", (s32)enum_name.count, enum_name.dat);
        append_to_header_buf(start);
        for (u64 y = 0; y < row_count; ++y) {
            String *value = t->values + x + y * t->column_count;
            String v = string_printf(scratch, "    %.*s,\n", (s32)value->count, value->dat);
            append_to_header_buf(v);
        }
        append_to_header_buf(string_printf(scratch, "};\n"));
        append_to_header_buf(string_printf(scratch, "#define %.*s_COUNT %llu\n\n", (s32)enum_name.count, enum_name.dat, row_count));
        arena_clear(scratch);
    }

    {
        String start = string_printf(scratch, "String str_%.*s[] = {\n", (s32)enum_name.count, enum_name.dat);
        append_to_src_buf(start);
        for (u64 y = 0; y < row_count; ++y) {
            String *value = t->values + x + y * t->column_count;
            String v = string_printf(scratch, "    S(\"%.*s\"),\n", (s32)value->count, value->dat);
            append_to_src_buf(v);
        }

        String v = string_printf(scratch, "};\n\n");
        append_to_src_buf(v);
        arena_clear(scratch);
    }
}

void gen_enum_flags(Table *t, String enum_name, String column_name) {

    u64 row_count = t->value_count / t->column_count;

    u64 x = get_column_id_from_str(t, column_name);
    {
        String start = string_printf(scratch, "enum %.*s {\n", (s32)enum_name.count, enum_name.dat);
        append_to_header_buf(start);
        for (u64 y = 0; y < row_count; ++y) {
            String *value = t->values + x + y * t->column_count;
            String v = string_printf(scratch, "    %.*s = 1 << %llu,\n", (s32)value->count, value->dat, y);
            append_to_header_buf(v);
        }

        String v = string_printf(scratch, "};\n\n");
        append_to_header_buf(v);
        arena_clear(scratch);
    }
}

void gen_table(Table *t, String struct_name, String table_name) {
    u64 row_count = t->value_count / t->column_count;

    append_to_header_buf(string_printf(scratch, "extern %.*s %.*s[%llu];\n", (s32)struct_name.count, struct_name.dat, (s32)table_name.count, table_name.dat, row_count));
    append_to_src_buf(string_printf(scratch, "%.*s %.*s[%llu] {\n", (s32)struct_name.count, struct_name.dat, (s32)table_name.count, table_name.dat, row_count));
    for (u64 y = 0; y < row_count; ++y) {

        append_to_src_buf(string_printf(scratch, "    {"));
        for (u64 x = 0; x < t->column_count; ++x) {
            String *value = t->values + x + y * t->column_count;
            append_to_src_buf(string_printf(scratch, "%.*s, ", (s32)value->count, value->dat));
        }
        append_to_src_buf(string_printf(scratch, "},\n"));
    }
    append_to_src_buf(string_printf(scratch, "};\n\n"));
    arena_clear(scratch);
}

#define make_table(columns, values) Table {ARRAY_SIZE(columns), columns, ARRAY_SIZE(values), values}

int main() {



    Table token_table;
    {
        String columns[] = {
            S("enum"),
        };
        String values[] = {
            S("TOKEN_INVALID"),
            S("TOKEN_NUMBER"),
            S("TOKEN_IDENTIFIER"),
            S("TOKEN_EQUAL"),
            S("TOKEN_SEMICOLON"),
            S("TOKEN_COLON"),
            S("TOKEN_COMMA"),
            S("TOKEN_PLUS"),
            S("TOKEN_MINUS"),
            S("TOKEN_STAR"),
            S("TOKEN_SLASH"),
            S("TOKEN_OPENPAREN"),
            S("TOKEN_CLOSEPAREN"),
        };

        token_table = make_table(columns, values);
    }
    Table bytecode_table;
    {
        String columns[] = {
            S("enum"),
        };
        String values[] = {
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

        bytecode_table = make_table(columns, values);
    }
    Table node_table;
    {
        String columns[] = {
            S("enum"), S("precedence"), S("is_left_associative"), S("is_expr"),
        };
        String values[] = {
            S("NODE_INVALID")    , S("-1") , S("false"), S("false"),
            S("NODE_PROGRAM")    , S("-1") , S("false"), S("false"),
            S("NODE_STATEMENT")  , S("-1") , S("false"), S("false"),
            S("NODE_NUMBER")     , S("-1") , S("false"), S("true"),
            S("NODE_FUNCTION")   , S("-1") , S("false"), S("true"),
            S("NODE_FUNCTIONDEF"), S("-1") , S("false"), S("false"),
            S("NODE_VARIABLE")   , S("-1") , S("false"), S("true"),
            S("NODE_VARIABLEDEF"), S("-1") , S("false"), S("false"),
            S("NODE_ADD")        , S("1")  , S("true"), S("true"),
            S("NODE_SUB")        , S("1")  , S("true"), S("true"),
            S("NODE_MUL")        , S("2")  , S("true"), S("true"),
            S("NODE_DIV")        , S("2")  , S("true"), S("true"),
            S("NODE_UNARYADD")   , S("100"), S("false"), S("true"),
            S("NODE_UNARYSUB")   , S("101"), S("false"), S("true"),
            S("NODE_OPENPAREN")  , S("0")  , S("true"), S("false"),
        };

        node_table = make_table(columns, values);
    }


    Table item_table;
    {
        String columns[] = {
            S("enum"),
        };
        String values[] = {
            S("ITEM_INVALID"),
            S("ITEM_VARIABLE"),
            S("ITEM_GLOBALVARIABLE"),
            S("ITEM_FUNCTION"),
        };

        item_table = make_table(columns, values);
    }

    Table pane_flags_table;
    {
        String columns[] = {
            S("enum"),
        };
        String values[] = {
            S("PANE_DRAGGABLE"),
            S("PANE_RESIZEABLE"),
            S("PANE_SCROLL"),
            S("PANE_TEXT_INPUT"),
            S("PANE_TEXT_DISPLAY"),
            S("PANE_BACKGROUND_COLOR"),
        };

        pane_flags_table = make_table(columns, values);
    }

    Table mouse_actions_table;
    {
        String columns[] = {
            S("enum"),
        };
        String values[] = {
            S("MOUSE_ACTION_NONE"),
            S("MOUSE_ACTION_DRAGGING"),
            S("MOUSE_ACTION_RESIZING"),
        };

        mouse_actions_table = make_table(columns, values);
    }

    append_to_src_buf(S("#include \"gen.h\"\n"));
    append_to_src_buf(S("#include \"../string.h\"\n"));
    append_to_header_buf(S("#pragma once\n"));
    append_to_header_buf(S("#include \"../common.h\"\n"));

    gen_enum_flags(&token_table, S("TokenType"), S("enum"));
    gen_enum(&bytecode_table, S("BytecodeType"), S("enum"));
    gen_enum(&node_table, S("NodeType"), S("enum"));
    gen_enum(&item_table, S("ItemType"), S("enum"));
    gen_enum_flags(&pane_flags_table, S("PaneFlags"), S("enum"));
    gen_enum(&mouse_actions_table, S("MouseAction"), S("enum"));


    append_to_header_buf(S(
R"(struct NodeTableData {
    NodeType node_type;
    s64 precedence;
    bool left_associative;
    bool is_expr;
};)""\n"
    ));
    gen_table(&node_table, S("NodeTableData"), S("node_data_table"));

    if (!CreateDirectory("./src/gen", nullptr)) {
        u32 err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) {
            LOG_ERROR("Failed to create gen directory\n");
            return 1;
        }
    }

    if (!write_string_to_file(S("./src/gen/gen.h"), String {header_buf, header_buf_count})) return 1;
    if (!write_string_to_file(S("./src/gen/gen.cpp"), String {src_buf, src_buf_count})) return 1;
}