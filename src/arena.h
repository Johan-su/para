#pragma once
#include "common.h"

struct Arena {
    u64 pos;
    u64 max_capacity;
    u8 *data;
};


void arena_init(Arena *arena, u64 max_capacity);
void arena_clean(Arena *arena);
u64 align_to_8_boundry(u64 a);
void *arena_alloc(Arena *arena, u64 byte_amount);
void arena_clear(Arena *arena);
u64 arena_get_pos(Arena *arena);
void arena_set_pos(Arena *arena, u64 pos);