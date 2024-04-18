#include <raylib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef signed char        s8;
typedef short              s16;
typedef int                s32;
typedef long long          s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef size_t usize;

typedef float f32;
typedef double f64;




#define assert(condition)                                                                    \
do                                                                                                  \
{                                                                                                   \
    if (!(condition))                                                                               \
    {                                                                                               \
        fprintf(stderr, "ERROR: assertion failed [%s] at %s:%d\n", #condition, __FILE__, __LINE__); \
        exit(1);                                                                                    \
    }                                                                                               \
} while (0)


#define todo() assert(false && "TODO")


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


static s32 min(s32 a, s32 b)
{
    if (a < b) return a;
    return b;
}

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




struct UI_Result
{
    bool pressed;
    bool down;
    bool released;

    bool active;


    bool finished;
};


enum UI_Flags : u64
{
    UI_Flags_clickable = (1 << 0),
    UI_Flags_clickable_toggle_active = (1 << 1),
    UI_Flags_clickable_click_active = (1 << 2),  
    UI_Flags_clickable_click_release_active = (1 << 3), 
    UI_Flags_text_input = (1 << 4),
    UI_Flags_draw_text = (1 << 5),
    UI_Flags_draw_border = (1 << 6),
    UI_Flags_draw_background = (1 << 7),
    UI_Flags_brighten_background_when_hot = (1 << 8),
    UI_Flags_brighten_background_when_active = (1 << 9),
};

struct UI_Element
{
    u32 flags;

    u64 id;
    s32 index;
    s32 parent_index;


    s32 text_index;
    bool text_mark;
    bool text_select;
    s32 text_mark_start_index;
    char *text;
    s32 text_capacity;

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

//TODO fix bug with text when pressing ctrl backspace sometimes

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


static bool is_active(u64 id, u64 index)
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
//             e.text_index = 0;
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

    const UI_Element *le = last_stack + element_count;

    e.text_index = le->text_index;
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
        buf, buf_size, 
        ColorFromHSV(236.0f, 0.45f, 0.45f), 
        w, h
    );

    return e.result;
}

static bool slider(f32 *data, f32 min, f32 max, s32 w, s32 h)
{
    return false; 
}


static UI_Result text_output(char *buf, s32 buf_size, s32 w, s32 h)
{
    UI_Element e = create_push_element(
        UI_Flags_draw_text|UI_Flags_draw_background|UI_Flags_draw_border,
        (u64) buf,
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
        buf,
        buf_size,
        ColorFromHSV(236.0f, 0.47f, 0.45f),
        w,
        h
    );

    return e.result;
}

static void begin_ui()
{
    UI_Layout layout = {};
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

static void remove_text_region_and_set_cursor(char *text, s32 *text_len, s32 *text_index, s32 start, s32 end)
{
    s32 region_len = end - start + 1;
    s32 right_len = *text_len - end - 1;

    memmove(text + start, text + end + 1, right_len);
    memset(text + start + right_len, 0, region_len);

    *text_len -= region_len;

    s32 cursor_change = start - *text_index;
    if (cursor_change > 0) cursor_change = 0;
    if (cursor_change < -region_len) cursor_change = -region_len; 

    *text_index += cursor_change;
}

static void remove_marked_region_and_set_cursor(UI_Element *e, s32 *text_len)
{
    s32 start_index = 0;
    s32 end_index = 0;
    if (e->text_index < e->text_mark_start_index)
    {
        start_index = e->text_index;
        end_index = e->text_mark_start_index;
    }
    else
    {
        start_index = e->text_mark_start_index;
        end_index = e->text_index;
    }

    remove_text_region_and_set_cursor(e->text, text_len, &e->text_index, start_index, end_index - 1);
}



// static void remove_text_region(char *text, s32 *text_len, s32 start, s32 end)
// {
//     s32 region_len = end - start;
//     s32 right_len = *text_len - end;

//     memmove(text + start, text + end, right_len);
//     memset(text + start + right_len, 0, region_len);

//     *text_len -= region_len;
// }

static void end_ui()
{
    // iterating over all elements
    for (UI_Element *e = element_list; e < element_list + element_count; ++e)
    {
        UI_Result result = {};

        if (e->active)
        {
            printf("e->index = %d active\n", e->index);
        }

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
                if (!is_hover)
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
                        e->text_index = 0;
                    }
                }

                s32 len = 0;
                {
                    usize len_ = strlen(e->text);
                    assert(len_ < INT32_MAX);
                    len = (s32) len_;
                }


                if (key_pressed[KEY_LEFT_SHIFT] && !e->text_select)
                {
                    e->text_mark = true;
                    e->text_select = true;
                    e->text_mark_start_index = e->text_index;
                }
                else if (key_released[KEY_LEFT_SHIFT] && e->text_select)
                {
                    e->text_select = false;
                }

                if (key_pressed[KEY_RIGHT] && e->text_index < len)
                {
                    if (!e->text_select)
                        e->text_mark = false;

                    if (key_down[KEY_LEFT_CONTROL])
                    {
                        s32 new_index = e->text_index;

                        while (new_index < len && is_whitespace(e->text[new_index])) 
                        {
                            new_index += 1;
                        }
                        while (new_index < len && !is_whitespace(e->text[new_index])) 
                        {
                            new_index += 1;
                        }
                        e->text_index = new_index;
                    }
                    else
                    {
                        e->text_index += 1;   
                    }
                }
                else if (key_pressed[KEY_LEFT] && e->text_index != 0)
                {
                    if (!e->text_select)
                        e->text_mark = false;

                    if (key_down[KEY_LEFT_CONTROL])
                    {  
                        s32 new_index = e->text_index - 1;

                        while (new_index >= 0 && is_whitespace(e->text[new_index])) 
                        {
                            new_index -= 1;
                        }
                        while (new_index >= 0 && !is_whitespace(e->text[new_index])) 
                        {
                            new_index -= 1;
                        }
                        e->text_index = new_index + 1;
                    }
                    else
                    {
                        e->text_index -= 1;   
                    }
                }
                else if (key_pressed[KEY_BACKSPACE])
                {
                    if (e->text_mark)
                    {
                        e->text_mark = false;

                        remove_marked_region_and_set_cursor(e, &len);
                    }
                    else if (e->text_index != 0)
                    {
                        if (key_down[KEY_LEFT_CONTROL])
                        {

                            s32 start_index = e->text_index - 1;

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

                            s32 end_index = e->text_index - 1; 
                            remove_text_region_and_set_cursor(e->text, &len, &e->text_index, start_index, end_index);

                        }
                        else
                        {
                            for (s32 i = e->text_index; i < len; ++i)
                            {
                                e->text[i - 1] = e->text[i];
                            }
                            e->text[len - 1] = '\0';
                            e->text_index -= 1;
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
                    else if (e->text_index != len)
                    {
                        for (s32 i = e->text_index + 1; i < len; ++i)
                        {
                            e->text[i - 1] = e->text[i];
                        }
                        e->text[len - 1] = '\0';
                    }
                }
                else if (key_pressed[KEY_HOME])
                {
                    e->text_index = 0;
                }
                else if (key_pressed[KEY_END])
                {
                    e->text_index = len;
                }
                else if (key_pressed[KEY_ENTER])
                {
                    e->active = false;
                    result.finished = true;
                }
                else
                {
                    u32 key = 0;
                    for (u32 j = 44; j <= 93; ++j)
                    {
                        if (key_pressed[j])
                        {
                            key = j;
                            break;
                        }
                    }

                    if (key_pressed[32]) 
                        key = 32;


                    if (!key_down[KEY_LEFT_SHIFT] && (key >= 'A' && key <= 'Z'))
                    {
                        key += 32;
                    }


                    if (key != 0)
                    {
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
                            for (s32 i = len; i-- > e->text_index; )
                            {
                                e->text[i + 1] = e->text[i];
                            }
                            e->text[e->text_index] = (char) key;
                            len += 1;
                            e->text_index += 1;
                        }
                    }
                }

                s32 center_y = e->y + e->h / 2;
                s32 text_width = glyph_width * len;

                s32 text_start_y = center_y - font_size / 2;


                DrawRectangle(e->x + 7 + glyph_width * e->text_index, text_start_y, 1, font_size, RED);

                // draw selected area


                if (e->text_mark)
                {
                    s32 start_index = 0;
                    s32 end_index = 0;
                    if (e->text_index < e->text_mark_start_index)
                    {
                        start_index = e->text_index;
                        end_index = e->text_mark_start_index;
                    }
                    else
                    {
                        start_index = e->text_mark_start_index;
                        end_index = e->text_index;
                    }
                    s32 mark_len = end_index - start_index;

                    DrawRectangle(e->x + 7 + glyph_width * start_index, text_start_y, glyph_width * mark_len, font_size, BLUE);
                }
            }
        }


        if (e->flags & UI_Flags_draw_text)
        {
            s32 center_y = e->y + e->h / 2;

            s32 len = 0;
            {
                usize len_ = strlen(e->text);
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


    memcpy(last_stack, element_list, sizeof(element_list));
    last_count = element_count;
    element_count = 0;
    // TODO(Johan) remove this memset. it is only for debugging.
    memset(element_list, 0, sizeof(element_list));
}




int main()
{
    int fps = 30;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1366, 768, "test");
    SetTargetFPS(fps);

    char buf[5][32] = {"test1","test2","test3","test4","test5"};

    char menu_buf[] = "menu1";

    char button_bufs[4][16] = {"button1", "button2", "button3", "button4"};

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();
        PollInputEvents();
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

        if (dropdown_menu(menu_buf, sizeof(menu_buf), 128, 128).active)
        {
            push_sub_layout(128, 4*128, DOWN);
            begin_children();

            for (s32 i = 0;  i < 4; ++i)
            {
                if (button(button_bufs[i], sizeof(*button_bufs), 126, 128).released)
                {
                    printf("%.*s was pressed\n", (int)sizeof(*button_bufs), button_bufs[i]);
                }
            }


            end_children();
            pop_layout();
        }
        push_sub_layout(128, 5*48, DOWN);
        for (s32 i = 0; i < 5; ++i)
        {
            if (text_input(buf[i], sizeof(*buf), 128, 48).finished)
            {
                printf("buf%d: %.*s\n", i, (int)sizeof(*buf), buf[i]);
            }
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

    CloseWindow();
}