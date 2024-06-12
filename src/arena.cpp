
struct Arena
{
    usize pos;
    usize max_capacity;
    u8 *data;
};



static void init_arena(Arena *arena, usize max_capacity)
{
    arena->pos = 0;
    arena->data = (u8 *)calloc(max_capacity, 1);
    arena->max_capacity = max_capacity;
}

static void clean_arena(Arena *arena)
{
    free(arena->data);
    memset(arena, 0, sizeof(*arena));
}

static usize align_to_8_boundry(usize a)
{
    usize offset = (8 - (a % 8)) % 8;
    return a + offset;
}

static void *alloc_arena(Arena *arena, usize byte_amount)
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

static void clear_arena(Arena *arena)
{
    arena->pos = 0;
}

static usize get_arena_pos(Arena *arena)
{
    assert(arena->pos % 8 == 0);
    return arena->pos;
}

static void set_arena_pos(Arena *arena, usize pos)
{
    assert(pos % 8 == 0);
    arena->pos = pos;
}

