#include <stdarg.h>
#include <stdio.h>
#include "common.h"



void assert_function(const char *cond, const char *file, s32 line) {
    fprintf(stderr, "Assertion failed '%s' %s:%d\n", cond, file, line);
#ifdef x86_64 
    asm("int3");
#else
    #error "assert function only works for x86_64 for now"
#endif
}