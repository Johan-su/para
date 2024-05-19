#include <raylib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>



// TODO: fix bug with error text output

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


typedef float f32;
typedef double f64;

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

#ifndef __GNUG__
#define __attribute__(x)
#define __format__(x)
#endif

#if __has_builtin(__builtin_debugtrap)
#else
#define __builtin_debugtrap(x)
#endif

#define assert(condition)                                                                    \
do                                                                                                  \
{                                                                                                   \
    if (!(condition))                                                                               \
    {                                                                                               \
        fprintf(stderr, "ERROR: assertion failed [%s] at %s:%d\n", #condition, __FILE__, __LINE__); \
        __builtin_debugtrap();                                                                        \
        exit(1);                                                                                    \
    }                                                                                               \
} while (0)

#define todo() assert(false && "TODO")


#include "arena.cpp"

#define alloc(arena, type, amount) (type*)(alloc_arena(arena, sizeof(type)*(amount)))

static bool is_whitespace(char c)
{
    switch (c)
    {
        case ' ': return true;
        case '\n': return true;
        case '\t': return true;
        case '\r': return true;
    }
    return false;
}

struct String
{
    char *data;
    usize len;
};

static bool String_equal(String a, String b)
{
    if (a.len != b.len) return false;

    usize len = a.len;
    for (usize i = 0; i < len; ++i)
    {
        if (a.data[i] != b.data[i]) return false;
    }
    return true;
}

static String string_from_cstr(Arena *arena, const char *cstr)
{
    String s = {};

    s.len = strlen(cstr);
    s.data = alloc(arena, char, s.len);

    memcpy(s.data, cstr, s.len);

    return s;
}

__attribute__((__format__ (__printf__, 2, 3)))
static char *tprintf(Arena *arena, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    int len_ = vsnprintf(nullptr, 0, format, args);   
    assert(len_ > 0);
    usize len = (usize) len_;
    

    char *d = alloc(arena, char, len + 1);
    vsnprintf(d, len + 1, format, args);   
    va_end(args);
    return d;
}

__attribute__((__format__ (__printf__, 2, 3)))
static String tprintf_string(Arena *arena, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    int len_ = vsnprintf(nullptr, 0, format, args);   
    assert(len_ > 0);
    usize len = (usize) len_;
    

    char *d = alloc(arena, char, len + 1);
    vsnprintf(d, len + 1, format, args);   
    va_end(args);


    String s = {};
    s.data = d;
    s.len = len;
    return s;
}

#include "parse.cpp"



// static s32 min(s32 a, s32 b)
// {
//     if (a < b) return a;
//     return b;
// }

#define BLINKING_TIME_SEC 3


static f32 dt = 0.0f;

static s32 mx = 0;
static s32 my = 0;


static s32 screen_width = 0;
static s32 screen_height = 0;


static bool mouse_left = false;
static bool mouse_left_pressed = false;
static bool mouse_left_released = false;

static bool key_down[512] = {};
static bool key_pressed[512] = {};
static bool key_released[512] = {};

static s32 unicode_chars[16] = {};
static s32 char_count = 0;


struct UI_Result
{
    bool pressed;
    bool down;
    bool released;

    bool active;


    bool finished;
    u32 len;
};


enum UI_Flags : u64
{
    UI_Flags_clickable                       = (1 << 0),
    UI_Flags_clickable_toggle_active         = (1 << 1),
    UI_Flags_clickable_click_active          = (1 << 2),
    UI_Flags_clickable_click_release_active  = (1 << 3),
    UI_Flags_text_input                      = (1 << 4),
    UI_Flags_draw_text                       = (1 << 5),
    UI_Flags_draw_border                     = (1 << 6),
    UI_Flags_draw_background                 = (1 << 7),
    UI_Flags_brighten_background_when_hot    = (1 << 8),
    UI_Flags_brighten_background_when_active = (1 << 9),
    UI_Flags_horizontal_scroll               = (1 << 10),
    UI_Flags_draw_samples                    = (1 << 11),
};

struct UI_Element
{
    u32 flags;

    u64 id;
    s32 index;
    s32 parent_index;


    f32 *data;
    s32 data_cap;

    s32 text_cursor_index;
    char *text;
    s32 text_capacity;
    
    // used for selection while holding LSHIFT
    bool text_mark;
    bool text_select;
    s32 text_mark_start_index;

    bool active;
    Color background_color;

    UI_Result result;

    s32 x, y, w, h;
};


enum UI_Flow
{
    RIGHT,
    LEFT,
    UP,
    DOWN,
};

struct UI_Layout
{
    s32 x, y, w, h;


    UI_Flow flow;
};


static UI_Element hot = {};
static UI_Element parent = {};


static UI_Element last_stack[512] = {};
static s32 last_count = 0;

static UI_Element element_list[512] = {};
static s32 element_count = 0;


static UI_Layout layout_list[512] = {};
static s32 layout_count = 0;


static void push_sub_layout(s32 w, s32 h, UI_Flow flow)
{
    assert(layout_count > 0);
    UI_Layout *l = layout_list + layout_count - 1;
    UI_Layout layout = {l->x, l->y, w, h, flow};


    switch (l->flow)
    {
        case RIGHT:
        {
            l->x += w;
        } break;
        case LEFT:
        {
            todo();
        } break;
        case UP:
        {
            todo();
        } break;
        case DOWN:
        {
            l->y += h;
        } break;
    }


    assert(layout_count < (s32) sizeof(layout_list));
    layout_list[layout_count++] = layout;
}

static void push_layout(UI_Layout layout)
{
    assert(layout_count < (s32) sizeof(layout_list));
    layout_list[layout_count++] = layout;
}


static void pop_layout()
{
    assert(layout_count > 0);
    layout_count -= 1;
}

static void append_element(UI_Element element)
{
    assert(element_count < (s32) sizeof(element_list));
    element_list[element_count++] = element;
}


static bool is_active(u64 id, s32 index)
{
    UI_Element *last_e = last_stack + index;

    if (id != last_e->id) return false;

    return last_e->active;
}


static bool is_hot(u64 id)
{
    return id == hot.id;
}

static void set_hot(UI_Element e)
{
    hot = e;
}


static bool hover(s32 x, s32 y, s32 w, s32 h)
{
    if (mx < x) return false;
    if (mx > x + w) return false;
    if (my < y) return false;
    if (my > y + h) return false;

    return true;
}

static void begin_children()
{
    assert(element_count > 0);
    parent = element_list[element_count - 1];
}

static void end_children()
{
    parent = {};
}

// static bool bounding_box(u64 id, s32 x, s32 y, s32 w, s32 h)
// {
//     UI_Element e = {};

//     e.flags = UI_Flags_clickable|UI_Flags_draw_background|UI_Flags_draw_border|UI_Flags_resizeable|UI_Flags_change_cursor_at_border;
//     e.id = id;
//     e.index = element_count;
//     e.parent_index = parent.index;

//     e.active = last_stack[e.index].active;
//     e.background_color = ColorFromHSV(236.0f, 0.45f, 0.45f);

//     e.x = x;
//     e.y = y;
//     e.w = w;
//     e.h = h;

//     bool is_hover = hover(x + 1, y + 1, w - 1, h - 1);


//     bool result = false;

//     if (is_active(id, e.index))
//     {
//         if (mouse_left_pressed && !is_hover)
//         {
//             e.active = false;
//         }
//     }
//     else if (is_hot(id))
//     {
//         if (mouse_left_pressed && is_hover)
//         {
//             e.text_cursor_index = 0;
//             e.active = true;
//         }
//     }
//     else if (is_hover)
//     {
//         set_hot(e);
//     }





//     append_element(e);
//     return result;
// }



static UI_Element create_push_element(
    u32 flags, 
    u64 id,
    f32 *data,
    s32 data_cap,
    char *text,
    s32 text_capacity, 
    Color background_color,
    s32 w, 
    s32 h
)
{
    UI_Element e = {};
    e.flags = flags;

    e.id = id;
    e.index = element_count;
    e.parent_index = parent.index;

    e.data = data;
    e.data_cap = data_cap;

    const UI_Element *le = last_stack + element_count;

    e.text_cursor_index = le->text_cursor_index;
    e.text_mark = le->text_mark;
    e.text_select = le->text_select;
    e.text_mark_start_index = le->text_mark_start_index;

    e.text = text;
    e.text_capacity = text_capacity;

    e.active = le->active;
    e.background_color = background_color;

    e.result = le->result;

    assert(layout_count > 0);
    UI_Layout *l = layout_list + layout_count - 1;

    e.x = l->x;
    e.y = l->y;
    e.w = w;
    e.h = h;

    switch (l->flow)
    {
        case RIGHT:
        {
            l->x += e.w;
        } break;
        case LEFT:
        {
            todo();
        } break;
        case UP:
        {
            todo();
        } break;
        case DOWN:
        {
            l->y += e.h;
        } break;
    }


    append_element(e);
    return e;
}


static UI_Result button(char *buf, s32 buf_size, s32 w, s32 h)
{
    UI_Element e = create_push_element( 
        UI_Flags_clickable|
        UI_Flags_clickable_click_active|
        UI_Flags_draw_background|
        UI_Flags_brighten_background_when_hot|
        UI_Flags_draw_border|
        UI_Flags_draw_text,
        (u64) buf, 
        nullptr, 0,
        buf, buf_size, 
        ColorFromHSV(236.0f, 0.45f, 0.45f), 
        w, h
    );

    return e.result;
}

static UI_Result dropdown_menu(char *buf, s32 buf_size, s32 w, s32 h)
{
    UI_Element e = create_push_element(
        UI_Flags_clickable|
        UI_Flags_clickable_toggle_active|
        UI_Flags_draw_background|
        UI_Flags_brighten_background_when_hot|
        UI_Flags_brighten_background_when_active|
        UI_Flags_draw_border|
        UI_Flags_draw_text,
        (u64) buf, 
        nullptr, 0,
        buf, buf_size, 
        ColorFromHSV(236.0f, 0.45f, 0.45f), 
        w, h
    );

    return e.result;
}

// static bool slider(f32 *data, f32 min, f32 max, s32 w, s32 h)
// {
//     return false; 
// }


static UI_Result text_output(char *buf, s32 buf_size, s32 w, s32 h)
{
    UI_Element e = create_push_element(
        UI_Flags_draw_text|UI_Flags_draw_background|UI_Flags_draw_border,
        (u64) buf,
        nullptr, 0,
        buf,
        buf_size,
        ColorFromHSV(236.0f, 0.45f, 0.45f),
        w,
        h
    );

    return e.result;
}

// uses buf pointer as unique id
static UI_Result text_input(char *buf, s32 buf_size, s32 w, s32 h)
{
    UI_Element e = create_push_element(
        UI_Flags_clickable|
        UI_Flags_text_input|
        UI_Flags_draw_text|
        UI_Flags_draw_background|
        UI_Flags_draw_border|
        UI_Flags_brighten_background_when_hot,
        (u64) buf,
        nullptr, 0,
        buf,
        buf_size,
        ColorFromHSV(236.0f, 0.47f, 0.45f),
        w,
        h
    );

    return e.result;
}
// uses buf pointer as unique id
static UI_Result plot_lines(f32 *buf, s32 buf_size, s32 w, s32 h)
{
    UI_Element e = create_push_element(
        UI_Flags_draw_background|
        UI_Flags_draw_border|
        UI_Flags_draw_samples|
        UI_Flags_brighten_background_when_hot,
        (u64) buf,
        buf, buf_size,
        nullptr,
        0,
        ColorFromHSV(236.0f, 0.47f, 0.45f),
        w,
        h
    );

    return e.result;
}

static void begin_ui()
{
    UI_Element e = {};

    e.flags = 0;
    e.id = 0;
    e.index = element_count;
    e.parent_index = 0;
    e.x = 0;
    e.y = 0;
    e.w = screen_width;
    e.h = screen_height;

    // if (hover(e.x + 1, e.y + 1, e.w - 1, e.h - 1))
    // {
    //     set_hot(e);
   // }


    append_element(e);
}

s32 font_size = 18;

s32 glyph_width = (s32)((f32)font_size*0.65f);

static void draw_text(const char *text, s32 text_len, int pos_x, int pos_y, Color color)
{
    for (s32 i = 0; i < text_len; ++i)
    {
        char buf[2];
        buf[0] = text[i];
        buf[1] = '\0';
        
        s32 width = glyph_width;

        DrawText(buf, pos_x + width * i, pos_y, font_size, color);
    }
}

static void remove_text_region_and_set_cursor(char *text, s32 *text_len, s32 *text_cursor_index, s32 start, s32 end)
{
    s32 region_len = end - start + 1;
    s32 right_len = *text_len - end - 1;

    memmove(text + start, text + end + 1, (usize)right_len);
    memset(text + start + right_len, 0, (usize)region_len);

    *text_len -= region_len;

    s32 cursor_change = start - *text_cursor_index;
    if (cursor_change > 0) cursor_change = 0;
    if (cursor_change < -region_len) cursor_change = -region_len; 

    *text_cursor_index += cursor_change;
}

static void remove_marked_region_and_set_cursor(UI_Element *e, s32 *text_len)
{
    s32 start_index = 0;
    s32 end_index = 0;
    if (e->text_cursor_index < e->text_mark_start_index)
    {
        start_index = e->text_cursor_index;
        end_index = e->text_mark_start_index;
    }
    else
    {
        start_index = e->text_mark_start_index;
        end_index = e->text_cursor_index;
    }

    remove_text_region_and_set_cursor(e->text, text_len, &e->text_cursor_index, start_index, end_index - 1);
}



// static void remove_text_region(char *text, s32 *text_len, s32 start, s32 end)
// {
//     s32 region_len = end - start;
//     s32 right_len = *text_len - end;

//     memmove(text + start, text + end, right_len);
//     memset(text + start + right_len, 0, region_len);

//     *text_len -= region_len;
// }

static usize bounded_strlen(char *data, usize max_size)
{
    usize len = 0;

    while (len < max_size && data[len] != '\0')
    {
        len += 1;
    }

    return len;
}


static bool has_active_children(s32 index)
{
    for (s32 i = 0; i < element_count; ++i)
    {
        UI_Element *e = element_list + i;

        if (e->parent_index == index && (e->active || hover(e->x + 1, e->y + 1, e->w - 1, e->h - 1)))
        {
            return true;   
        }
    }
    return false;
}

static void end_ui()
{
    bool was_set_hot = false;
    // iterating over all elements
    for (UI_Element *e = element_list; e < element_list + element_count; ++e)
    {
        UI_Result result = {};

        // if (e->active)
        // {
        //     printf("e->index = %d active\n", e->index);
        // }

        bool is_hover = hover(e->x + 1, e->y + 1, e->w - 1, e->h - 1);
        if (e->flags & UI_Flags_clickable)
        {
            if (is_hover)
            {
                result.pressed = mouse_left_pressed;
                result.released = mouse_left_released;
                result.down = mouse_left;

                if ((e->flags & UI_Flags_clickable_toggle_active) && mouse_left_pressed)
                {
                    e->active = !e->active;
                }
            }

            if (is_active(e->id, e->index))
            {
                if (!is_hover && !has_active_children(e->index))
                {
                    if (mouse_left_pressed)
                    {
                        e->active = false;
                    }
                }
            }
            else
            {
                if (is_hot(e->id))
                {
                    if (mouse_left_pressed)
                    {
                        e->active = true;
                    }
                }
            }

            if (is_hover)
            {
                was_set_hot = true;
                set_hot(*e);
            }


        }


        


        if (e->flags & UI_Flags_draw_background)
        {
            Color color = e->background_color;
            if (e->flags & UI_Flags_brighten_background_when_hot)
            {
                if (is_hot(e->id))
                {
                    color = ColorBrightness(e->background_color, 0.4f);
                }
            }
            if (e->flags & UI_Flags_brighten_background_when_active)
            {
                if (is_active(e->id, e->index))
                {
                    color = ColorBrightness(e->background_color, 0.4f);
                }
            }
            DrawRectangle(e->x, e->y, e->w, e->h, color);                
        }

        if ((e->flags & UI_Flags_clickable) && (e->flags & UI_Flags_text_input))
        {
            if (is_active(e->id, e->index))
            {
                if (mouse_left_pressed)
                {
                    if (!is_hover)
                    {
                        e->active = false;
                        result.finished = true;
                    }
                    else
                    {
                        // TODO(Johan) maybe set according to mouse position
                        e->text_cursor_index = 0;
                    }
                }

                s32 len = 0;
                {
                    assert(e->text_capacity >= 0);
                    usize len_ = bounded_strlen(e->text, (usize)e->text_capacity);
                    assert(len_ < INT32_MAX);
                    len = (s32) len_;
                }


                if (key_pressed[KEY_LEFT_SHIFT] && !e->text_select)
                {
                    e->text_mark = true;
                    e->text_select = true;
                    e->text_mark_start_index = e->text_cursor_index;
                }
                else if (key_released[KEY_LEFT_SHIFT] && e->text_select)
                {
                    e->text_select = false;
                }

                if (key_pressed[KEY_RIGHT] && e->text_cursor_index < len)
                {
                    if (!e->text_select)
                        e->text_mark = false;

                    if (key_down[KEY_LEFT_CONTROL])
                    {
                        s32 new_index = e->text_cursor_index;

                        while (new_index < len && is_whitespace(e->text[new_index])) 
                        {
                            new_index += 1;
                        }
                        while (new_index < len && !is_whitespace(e->text[new_index])) 
                        {
                            new_index += 1;
                        }
                        e->text_cursor_index = new_index;
                    }
                    else
                    {
                        e->text_cursor_index += 1;   
                    }
                }
                else if (key_pressed[KEY_LEFT] && e->text_cursor_index != 0)
                {
                    if (!e->text_select)
                        e->text_mark = false;

                    if (key_down[KEY_LEFT_CONTROL])
                    {  
                        s32 new_index = e->text_cursor_index - 1;

                        while (new_index >= 0 && is_whitespace(e->text[new_index])) 
                        {
                            new_index -= 1;
                        }
                        while (new_index >= 0 && !is_whitespace(e->text[new_index])) 
                        {
                            new_index -= 1;
                        }
                        e->text_cursor_index = new_index + 1;
                    }
                    else
                    {
                        e->text_cursor_index -= 1;   
                    }
                }
                else if (key_pressed[KEY_BACKSPACE])
                {
                    if (e->text_mark)
                    {
                        e->text_mark = false;

                        remove_marked_region_and_set_cursor(e, &len);
                    }
                    else if (e->text_cursor_index != 0)
                    {
                        if (key_down[KEY_LEFT_CONTROL])
                        {

                            s32 start_index = e->text_cursor_index - 1;

                            // move left while whitespace
                            while (start_index > 0 && is_whitespace(e->text[start_index])) 
                            {
                                start_index -= 1;
                            }
                            // move left until no whitespace
                            while (start_index > 0 && !is_whitespace(e->text[start_index])) 
                            {
                                start_index -= 1;
                            }

                            s32 end_index = e->text_cursor_index - 1; 
                            remove_text_region_and_set_cursor(e->text, &len, &e->text_cursor_index, start_index, end_index);

                        }
                        else
                        {
                            for (s32 i = e->text_cursor_index; i < len; ++i)
                            {
                                e->text[i - 1] = e->text[i];
                            }
                            e->text[len - 1] = '\0';
                            e->text_cursor_index -= 1;
                        }
                    }
                }
                else if (key_pressed[KEY_DELETE])
                {
                    if (e->text_mark)
                    {
                        e->text_mark = false;

                        remove_marked_region_and_set_cursor(e, &len);
                    }
                    else if (e->text_cursor_index != len)
                    {
                        for (s32 i = e->text_cursor_index + 1; i < len; ++i)
                        {
                            e->text[i - 1] = e->text[i];
                        }
                        e->text[len - 1] = '\0';
                    }
                }
                else if (key_pressed[KEY_HOME])
                {
                    e->text_cursor_index = 0;
                }
                else if (key_pressed[KEY_END])
                {
                    e->text_cursor_index = len;
                }
                else if (key_pressed[KEY_ENTER] || key_pressed[KEY_KP_ENTER])
                {
                    e->active = false;
                    result.finished = true;
                }
                else if (key_pressed[KEY_C] && key_down[KEY_LEFT_CONTROL])
                {
                    s32 start_index = 0;
                    s32 end_index = 0;
                    if (e->text_cursor_index < e->text_mark_start_index)
                    {
                        start_index = e->text_cursor_index;
                        end_index = e->text_mark_start_index;
                    }
                    else
                    {
                        start_index = e->text_mark_start_index;
                        end_index = e->text_cursor_index;
                    }

                    if (e->text_mark)
                    {
                        char buf[128] = {};
                        snprintf(buf, sizeof(buf), "%.*s", (int)(end_index - start_index), e->text + start_index);
                        SetClipboardText(buf);
                    }
                }
                else if (key_pressed[KEY_V] && key_down[KEY_LEFT_CONTROL])
                {
                    todo();
                    GetClipboardText();
                }
                else
                {

                    for (s32 i = 0; i < char_count; ++i)
                    {
                        s32 key = unicode_chars[i];
                        // TODO(Johan) maybe change so, shift still can mark after typing
                        if (e->text_select)
                        {
                            e->text_select = false;
                            e->text_mark = false;
                        }
                        
                        if (e->text_mark)
                        {
                            e->text_mark = false;
                            remove_marked_region_and_set_cursor(e, &len);
                        }

                        if (len + 1 < e->text_capacity)
                        {
                            for (s32 j = len; j-- > e->text_cursor_index; )
                            {
                                e->text[j + 1] = e->text[j];
                            }
                            //TODO(Johan) handle unicode characters correctly
                            e->text[e->text_cursor_index] = (char) key;
                            len += 1;
                            e->text_cursor_index += 1;
                        }
                    }
                }

                s32 center_y = e->y + e->h / 2;
                // s32 text_width = glyph_width * len;

                s32 text_start_y = center_y - font_size / 2;


                DrawRectangle(e->x + 7 + glyph_width * e->text_cursor_index, text_start_y, 1, font_size, RED);

                // draw selected area


                if (e->text_mark)
                {
                    s32 start_index = 0;
                    s32 end_index = 0;
                    if (e->text_cursor_index < e->text_mark_start_index)
                    {
                        start_index = e->text_cursor_index;
                        end_index = e->text_mark_start_index;
                    }
                    else
                    {
                        start_index = e->text_mark_start_index;
                        end_index = e->text_cursor_index;
                    }
                    s32 mark_len = end_index - start_index;

                    DrawRectangle(e->x + 7 + glyph_width * start_index, text_start_y, glyph_width * mark_len, font_size, BLUE);
                }
                result.len = (u32)len;
            }
        }

        if (e->flags & UI_Flags_draw_samples)
        {
            f32 max_y = -INFINITY;
            f32 min_y = INFINITY;


            for (s32 i = 0; i < e->data_cap; ++i)
            {
                f32 sample = e->data[i];

                if (sample > max_y)
                    max_y = sample;
                if (sample < min_y)
                    min_y = sample;
            }




            f32 coord_to_screen_x = (f32) e->w / (f32) e->data_cap;
            f32 coord_to_screen_y = ((f32) e->h) / (max_y - min_y);




            for (s32 i = 0; i < e->data_cap - 1; ++i)
            {
                f32 sample = e->data[i];
                f32 sample_2 = e->data[i + 1];

                Vector2 start = {(e->x + 1) + i * coord_to_screen_x, (e->y + e->h) - (sample - min_y) * coord_to_screen_y};

                Vector2 end = {(e->x + 1) + (i + 1) * coord_to_screen_x, (e->y + e->h) - (sample_2 - min_y) * coord_to_screen_y};

                DrawLineEx(start, end, 1.0f, BLACK);
            }
        }

        if (e->flags & UI_Flags_draw_text)
        {
            s32 center_y = e->y + e->h / 2;

            s32 len = 0;
            {
                usize len_ = bounded_strlen(e->text, (usize)e->text_capacity);
                assert(len_ < INT32_MAX);
                len = (s32) len_;
            }


            draw_text(e->text, len, e->x + 8, center_y - font_size / 2, WHITE);
        }


        if (e->flags & UI_Flags_draw_border)
        {
            DrawRectangleLines(e->x, e->y, e->w, e->h, YELLOW);
        }

        result.active = e->active;
        e->result = result;
    }

    if (!was_set_hot)
    {
        set_hot({});
    }

    memcpy(last_stack, element_list, sizeof(element_list));
    last_count = element_count;
    element_count = 0;
    // TODO(Johan) remove this memset. it is only for debugging.
    memset(element_list, 0, sizeof(element_list));

}




// static String concat_and_add_semicolon_at_the_end_of_every_substr(Arena *arena, char **strs, u32 *lens, u32 string_count)
// {
//     String str = {};

//     for (u32 i = 0; i < string_count; ++i)
//     {
//         if (lens[i] > 0)
//         {
//             // + 1 memory for semicolon
//             str.len += lens[i] + 1;
//         }
//     } 
//     str.data = alloc(arena, char, str.len);


//     u32 offset = 0;
//     for (u32 i = 0; i < string_count; ++i)
//     {
//         memcpy(str.data + offset, strs[i], lens[i]);
//         if (lens[i] > 0)
//         {
//             str.data[lens[i] + offset] = ';';
//             offset += 1;
//         }
//         offset += lens[i];
//     } 


//     return str;
// }


static String concat_and_add_semicolon_at_the_end_of_every_substr(Arena *arena, char **strs, u32 *lens, u32 string_count)
{
    String str = {};

    for (u32 i = 0; i < string_count; ++i)
    {
        if (lens[i] > 0)
        {
            // + 1 memory for semicolon
            str.len += lens[i] + 1;
        }
    } 
    str.data = alloc(arena, char, str.len);


    u32 offset = 0;
    for (u32 i = 0; i < string_count; ++i)
    {
        memcpy(str.data + offset, strs[i], lens[i]);
        if (lens[i] > 0)
        {
            str.data[lens[i] + offset] = ';';
            offset += 1;
        }
        offset += lens[i];
    } 


    return str;
}

int main()
{
    // test();
    // return 0;
    int fps = 60;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1366, 768, "test");
    SetTargetFPS(fps);


    Arena temp_arena; init_arena(&temp_arena, 1000000);
    Arena compile_arena; init_arena(&compile_arena, 1000000);
    Arena execute_arena; init_arena(&execute_arena, 1000000);



    //TODO: horizontal scrolling
    char *input_buf[8] = {};
    char *output_buf[ARRAY_SIZE(input_buf)] = {};
    f32 *samples[ARRAY_SIZE(input_buf)] = {};

    bool draw_samples[ARRAY_SIZE(input_buf)] = {};
    bool draw_text[ARRAY_SIZE(input_buf)] = {};

    s32 text_input_size = 32;
    s32 sample_size = 128;

    for (u32 i = 0; i < ARRAY_SIZE(input_buf); ++i)
    {
        usize a = (usize)text_input_size;

        input_buf[i] = alloc(&temp_arena, char, a);
        output_buf[i] = alloc(&temp_arena, char, a);

        usize b = (usize)sample_size;
        samples[i] = alloc(&temp_arena, f32, b);
    }

    u32 lens[ARRAY_SIZE(input_buf)] = {};


    char menu_buf[] = "menu1";
    char button_bufs[4][16] = {"button1", "button2", "button3", "button4"};


    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        char_count = 0;
        while (true)
        {
            s32 unicode_char = GetCharPressed();
            if (unicode_char == 0 || char_count == ARRAY_SIZE(unicode_chars))
                break;

            unicode_chars[char_count++] = unicode_char;
        }

        {
            bool data = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            mouse_left_pressed = data && !mouse_left;
            mouse_left_released = !data && mouse_left;
            mouse_left = data;
        }
        for (int i = 0; i <= 348; ++i)
        {
            bool data = IsKeyDown(i);
            key_pressed[i] = data && !key_down[i];
            key_released[i] = !data && key_down[i];
            key_down[i] = data;
        }

        mx = GetMouseX();
        my = GetMouseY();

        screen_width = GetScreenWidth();
        screen_height = GetScreenHeight();




        BeginDrawing();
        //
        ClearBackground(RAYWHITE);


        begin_ui();
        push_layout({0, 0, screen_width, screen_height, RIGHT});


        s32 text_w = 128 * 3;        
        s32 text_h = 48;


        push_sub_layout(2 * text_w, screen_height, DOWN);
        if (dropdown_menu(menu_buf, sizeof(menu_buf), 128, 32).active)
        {
            begin_children();
            for (usize i = 0; i < ARRAY_SIZE(input_buf); ++i)
            {
                push_sub_layout(text_w + 20, text_h, RIGHT);
                UI_Result result = text_input(input_buf[i], text_input_size, text_w, text_h);
                if (result.finished)
                {
                    memset(draw_text, 0, sizeof(draw_text));
                    memset(draw_samples, 0, sizeof(draw_samples));

                    lens[i] = result.len;
                    clear_arena(&compile_arena);
                    String s = concat_and_add_semicolon_at_the_end_of_every_substr(&compile_arena, (char **)input_buf, lens, 8);
                    // String s = string_from_cstr(&compile_arena, "h(x)=x;g(x)=h(x);f(x)=g(x);");
                    printf("%.*s\n", (int)s.len, s.data);
                    Program program;
                    int err = compile(&compile_arena, s.data, (u32)s.len, &program);
                    if (err)
                    {
                        Error e = get_error();

                        u32 expr_index = 0;
                        for (u32 j = 0; j < s.len && j < e.errs[0].error_index; ++j)
                        {
                            if (s.data[j] == ';')   
                            {
                                expr_index += 1;
                            }
                        }
                        String err_str = e.errs[0].msg;

                        assert(expr_index < 8);
                        draw_text[expr_index] = true;
                        memset(output_buf[expr_index], 0, 32);
                        memcpy(output_buf[expr_index], err_str.data, err_str.len < 31 ? err_str.len : 31);
                    }
                    else
                    {
                        u32 sym_count = 0;
                        for (usize j = 0; j < ARRAY_SIZE(input_buf); ++j)
                        {
                            if (lens[j] == 0)
                                continue;
                            Result r = {};
                            clear_arena(&execute_arena);
                            Symbol *sym = program.syms + sym_count + program.predefined_end;
                            sym_count += 1;

                            switch (sym->type)
                            {
                                case Symbol_Type::INVALID: assert(false);
                                case Symbol_Type::FUNCTION:
                                {
                                    if (sym->arg_count == 1)
                                    {
                                        draw_samples[j] = true;

                                        f32 *sample_list = samples[j];
                                        for (u32 k = 0; k < sample_size; ++k)
                                        {
                                            clear_arena(&execute_arena);
                                            f64 inputs[1] = {k * (10.0 / (f64) sample_size)};
                                            Result res = execute_ops(&execute_arena, program, sym->index, inputs, 1);
                                            assert(res.result_count == 1);
                                            sample_list[k] = (f32) res.result[0];
                                        }
                                    }
                                } break;
                                case Symbol_Type::VARIABLE:
                                {
                                    if (sym->scope == 0)
                                    {
                                        draw_text[j] = true;
                                        r = execute_ops(&execute_arena, program, sym->index, nullptr, 0);
                                    }
                                    else
                                    {
                                        j -= 1;
                                    }
                                } break;
                                case Symbol_Type::EXPR:
                                {
                                    draw_text[j] = true;
                                    r = execute_ops(&execute_arena, program, sym->index, nullptr, 0);
                                } break;
                                case Symbol_Type::BUILTIN_FUNC:
                                case Symbol_Type::BUILTIN_VAR:
                                {
                                    // do nothing
                                } break;
                            }
                            if (r.result_count > 1)
                            {
                                todo();
                            }
                            if (r.result_count == 1)
                            {
                                String val_str = tprintf_string(&compile_arena, "%g", r.result[0]);

                                memset(output_buf[j], 0, 32);
                                memcpy(output_buf[j], val_str.data, val_str.len);
                            }
                        }
                    }
                    // printf("input_buf%zu: %.*s\n", i, (int)text_input_size, input_buf[i]);
                }
                if (draw_samples[i])
                {
                    plot_lines(samples[i], sample_size, 48, 48);
                }
                if (draw_text[i])
                {
                    usize size = bounded_strlen(output_buf[i], text_input_size);
                    text_output(output_buf[i], text_input_size, 10 + glyph_width * (s32)size, 20);
                }
                pop_layout();
            }
            end_children();
        }
        pop_layout();

        pop_layout();
        end_ui();


        char buf1[32] = {};
        snprintf(buf1, sizeof(buf1), "mouse {%d, %d}", mx, my);

        DrawText(buf1, 0, screen_height - 24, 12, BLACK);


        char buf2[32] = {};
        snprintf(buf2, sizeof(buf2), "screen {%d, %d}", screen_width, screen_height);
        DrawText(buf2, 0, screen_height - 12, 12, BLACK);

        

        //
        EndDrawing();
    }

    clean_arena(&temp_arena);
    clean_arena(&compile_arena);
    clean_arena(&execute_arena);
    CloseWindow();
}