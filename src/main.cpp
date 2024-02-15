#include <raylib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
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



enum UI_Flags : u32
{
    UI_Flags_clickable = (1 << 0),
    UI_Flags_text_input = (1 << 1),
    UI_Flags_draw_text = (1 << 2),
    UI_Flags_draw_border = (1 << 3),
    UI_Flags_draw_background = (1 << 4),
    UI_Flags_brighten_background_when_hot = (1 << 5),
    UI_Flags_brighten_background_when_active = (1 << 6),
};

struct UI_Element
{
    u32 flags;

    u64 id;
    s32 index;
    s32 parent_index;


    s32 text_index;
    s32 text_capacity;
    bool text_mark;
    bool text_select;
    s32 text_mark_start_index;
    char *text;

    bool active;
    Color background_color;

    s32 x, y, w, h;
};

static UI_Element hot = {};
static UI_Element parent = {};


static UI_Element last_stack[512] = {};
static s32 last_count = 0;

static UI_Element element_stack[512] = {};
static s32 element_count = 0;



static void push_element(UI_Element element)
{
    assert(element_count < (s32) sizeof(element_stack));
    element_stack[element_count++] = element;
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
    parent = element_stack[element_count - 1];
}

static void end_children()
{
    parent = {};
}

static bool button(char *buf, s32 buf_size, s32 x, s32 y, s32 w, s32 h)
{
    u64 id = (u64) buf;
    UI_Element e = {};

    e.flags = UI_Flags_clickable|UI_Flags_draw_background|UI_Flags_brighten_background_when_hot|UI_Flags_draw_border|UI_Flags_draw_text;
    e.id = id;
    e.index = element_count;
    e.parent_index = parent.index;
    e.text_capacity = buf_size;
    e.text = buf;

    e.active = last_stack[e.index].active;
    e.background_color = ColorFromHSV(236.0f, 0.45f, 0.45f);

    e.x = x;
    e.y = y;
    e.w = w;
    e.h = h;

    bool is_hover = hover(x + 1, y + 1, w - 1, h - 1);


    bool result = false;

    if (is_active(id, e.index))
    {
        if (mouse_left_released)
        {
            e.active = false;
        }
        result = true;
    }
    else if (is_hot(id))
    {
        if (mouse_left_released && is_hover)
        {
            e.text_index = 0;
            e.active = true;
        }
    }
    else if (is_hover)
    {
        set_hot(e);
    }





    push_element(e);
    return result;
}

static bool dropdown_menu(char *buf, s32 buf_size, s32 x, s32 y, s32 w, s32 h)
{
    u64 id = (u64) buf;
    UI_Element e = {};

    e.flags = UI_Flags_clickable|UI_Flags_draw_background|UI_Flags_brighten_background_when_hot|UI_Flags_brighten_background_when_active|UI_Flags_draw_border|UI_Flags_draw_text;
    e.id = id;
    e.index = element_count;
    e.parent_index = parent.index;
    e.text_capacity = buf_size;
    e.text = buf;

    e.active = last_stack[e.index].active;
    e.background_color = ColorFromHSV(236.0f, 0.45f, 0.45f);

    e.x = x;
    e.y = y;
    e.w = w;
    e.h = h;

    bool is_hover = hover(x + 1, y + 1, w - 1, h - 1);

    bool result = false;




    if (is_active(id, e.index))
    {
        if (mouse_left_released)
        {
            e.active = false;
        }
        result = true;
    }
    else if (is_hot(id))
    {
        if (mouse_left_released && is_hover)
        {
            e.text_index = 0;
            e.active = true;
        }
    }
    else if (is_hover)
    {
        set_hot(e);
    }





    push_element(e);
    return result;
}


static bool slider(f32 *data, f32 min, f32 max, s32 x, s32 y, s32 w, s32 h)
{
     
}


static bool text_output(char *buf, s32 buf_size, s32 x, s32 y, s32 w, s32 h)
{
    u64 id = (u64) buf;
    UI_Element e = {};

    e.flags = UI_Flags_draw_text|UI_Flags_draw_background|UI_Flags_draw_border;
    e.id = id;
    e.index = element_count;
    e.parent_index = parent.index;
    e.text_capacity = buf_size;
    e.text = buf;
    e.active = last_stack[e.index].active;
    e.background_color = ColorFromHSV(236.0f, 0.45f, 0.45f);
    e.x = x;
    e.y = y;
    e.w = w;
    e.h = h;

    bool is_hover = hover(x + 1, y + 1, w - 1, h - 1);


    if (is_active(id, e.index))
    {
        if (mouse_left_pressed && !is_hover)
        {
            e.active = false;
        }
    }
    else if (is_hot(id))
    {
        if (mouse_left_pressed)
        {
            e.text_index = 0;
            e.active = true;
        }
    }
    else if (is_hover)
    {
        set_hot(e);
    }


    push_element(e);
    return false;
}

// uses buf pointer as unique id
static bool text_input(char *buf, s32 buf_size, s32 x, s32 y, s32 w, s32 h)
{
    u64 id = (u64) buf;
    UI_Element e = {};
    const UI_Element *last_e = last_stack +element_count;

    e.flags = UI_Flags_text_input|UI_Flags_clickable|UI_Flags_draw_text|UI_Flags_draw_background|UI_Flags_draw_border|UI_Flags_brighten_background_when_hot;
    e.id = id;
    e.index = element_count;
    e.parent_index = parent.index;
    e.text_index = last_e->text_index;
    e.text_mark = last_e->text_mark;
    e.text_select = last_e->text_select;
    e.text_mark_start_index = last_e->text_mark_start_index;
    e.text_capacity = buf_size;
    e.text = buf;
    e.active = last_e->active;
    e.background_color = ColorFromHSV(236.0f, 0.47f, 0.45f);
    e.x = x;
    e.y = y;
    e.w = w;
    e.h = h;

    bool is_hover = hover(x + 1, y + 1, w - 1, h - 1);

    bool result = false;

    if (is_active(id, e.index))
    {
        if ((mouse_left_pressed && !is_hover ) || key_pressed[KEY_ENTER])
        {
            e.active = false;
            result = true;
        }
    }
    else if (is_hot(id))
    {
        if (mouse_left_pressed)
        {
            e.text_index = 0;
            e.active = true;
        }
    }
    if (is_hover)
    {
        set_hot(e);
    }


    push_element(e);
    return false;
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


    push_element(e);
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
    for (UI_Element *e = element_stack; e < element_stack + element_count; ++e)
    {
         
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
                if (mouse_left_pressed && hover(e->x + 1, e->y + 1, e->w - 1, e->h - 1))
                {
                    // maybe set according to mouse position
                    e->text_index = 0;
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

                            s32 end_index = e->text_index - 1; 
                            s32 start_index = e->text_index - 1;

                            while (start_index >= 0 && is_whitespace(e->text[start_index])) 
                            {
                                start_index -= 1;
                            }
                            while (start_index >= 0 && !is_whitespace(e->text[start_index])) 
                            {
                                start_index -= 1;
                            }

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

                        remove_text_region_and_set_cursor(e->text, &len, &e->text_index, start_index, end_index);
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
                    element_stack[e->index].active = false;
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


    }

    memcpy(last_stack, element_stack, sizeof(element_stack));
    last_count = element_count;
    element_count = 0;
    // TODO(Johan) remove this memset. it is only for debugging.
    memset(element_stack, 0, sizeof(element_stack));
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
        

        if (dropdown_menu(menu_buf, sizeof(menu_buf), 128, 0, 128, 128))
        {
            begin_children();

            for (s32 i = 0;  i < 4; ++i)
            {
                if (button(button_bufs[i], sizeof(*button_bufs), 130, (i + 1) * 128, 126, 128))
                {
                    printf("%.*s was pressed\n", (int)sizeof(*button_bufs), button_bufs[i]);
                }
            }
        



            end_children();
        }

        for (s32 i = 0; i < 5; ++i)
        {
            if (text_input(buf[i], sizeof(*buf), 0, i * 48, 128, 48))
            {
                printf("buf%d: %.*s\n", i, (int)sizeof(*buf), buf[i]);
            }
        }

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