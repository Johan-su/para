#pragma once
#include "common.h"
#include "arena.h"

struct String {
    u8 *dat;
    u64 count;
};


#define str_lit(str) String {(u8 *)(str), sizeof(str) - 1}

#if GCC || CLANG
__attribute__((__format__ (__printf__, 2, 3)))
#endif
String string_printf(Arena *arena, const char *fmt, ...);
bool string_equal(String a, String b);