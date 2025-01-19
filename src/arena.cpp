struct Arena
{
    u64 pos;
    u64 max_capacity;
    u8 *data;
};


void init_arena(Arena *arena, u64 max_capacity)
{
    arena->pos = 0;
    arena->data = (u8 *)calloc(max_capacity, 1);
    arena->max_capacity = max_capacity;
}

void clean_arena(Arena *arena)
{
    free(arena->data);
    memset(arena, 0, sizeof(*arena));
}

u64 align_to_8_boundry(u64 a)
{
    usize offset = (8 - (a % 8)) % 8;
    return a + offset;
}

void *alloc_arena(Arena *arena, u64 byte_amount)
{
    assert(arena->pos % 8 == 0);
    if (arena->pos + byte_amount > arena->max_capacity)
    {
        fprintf(stderr, "ERROR: arena out of memory\n");
        return nullptr;
    }


    u8 *p = arena->data + arena->pos;

    memset(p, 0, byte_amount);

    arena->pos += byte_amount;
    arena->pos = align_to_8_boundry(arena->pos);

    return (void *)p;
}

void clear_arena(Arena *arena)
{
    arena->pos = 0;
}

u64 get_arena_pos(Arena *arena)
{
    assert(arena->pos % 8 == 0);
    return arena->pos;
}

void set_arena_pos(Arena *arena, u64 pos)
{
    assert(pos % 8 == 0);
    arena->pos = pos;
}