


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
    usize offset = (8 - (a & 3)) & 3;
    return a + offset;
}

static void *alloc_arena(Arena *arena, usize byte_amount)
{
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

