#include <stdarg.h>
#include <stdio.h>
#include "string.h"


String string_printf(Arena *arena, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len_ = vsnprintf(nullptr, 0, fmt, args);   
    assert(len_ >= 0);

    String s = {};
    s.count = (u64) len_;
    
    s.dat = (u8 *)arena_alloc(arena, s.count + 1);
    vsnprintf((char *)s.dat, s.count + 1, fmt, args);   
    va_end(args);

    return s;
}

bool string_equal(String a, String b) {
    if (a.count != b.count) return false;

    for (u64 i = 0; i < a.count; ++i) {
        if (a.dat[i] != b.dat[i]) return false;
    }

    return true;
}