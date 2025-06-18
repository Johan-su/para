#pragma once
#include "common.h"
#include "string.h"
struct Window {
    char d[16];
};

struct ButtonState {
    u8 transitions;
    bool ended_down;
};
enum Button {
    BUTTON_A,
    BUTTON_B,
    BUTTON_C,
    BUTTON_D,
    BUTTON_E,
    BUTTON_F,
    BUTTON_G,
    BUTTON_H,
    BUTTON_I,
    BUTTON_J,
    BUTTON_K,
    BUTTON_L,
    BUTTON_M,
    BUTTON_N,
    BUTTON_O,
    BUTTON_P,
    BUTTON_Q,
    BUTTON_R,
    BUTTON_S,
    BUTTON_T,
    BUTTON_U,
    BUTTON_V,
    BUTTON_W,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_Z,
    BUTTON_ESC,
    BUTTON_SPACE,
    BUTTON_BACKSPACE,
    BUTTON_LSHIFT,
    BUTTON_LCTRL,
    BUTTON_LEFT,
    BUTTON_UP,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_HOME,
    BUTTON_END,
    BUTTON_DELETE,
    BUTTON_ML, // mouse
    BUTTON_MM,
    BUTTON_MR,
    BUTTON_M4,
    BUTTON_M5,



    BUTTON_COUNT,
};

struct Input {
    bool quit;

    bool resized;
    u32 screen_width;
    u32 screen_height;

    s16 mx;
    s16 my;

    // f64 deltamx;
    // f64 deltamy;

    // s32 scrollx;
    s32 scrolly;

    u32 chars[16];
    u32 char_count;

    u32 keys[16];
    u32 key_count;

    ButtonState buttons[BUTTON_COUNT];
};

enum MouseCursor {
    MOUSE_CURSOR_ARROW,
    MOUSE_CURSOR_IBEAM,
    MOUSE_CURSOR_RESIZE_NWSE,
    MOUSE_CURSOR_RESIZE_NESW,
    MOUSE_CURSOR_RESIZE_WE,
    MOUSE_CURSOR_RESIZE_NS,
    MOUSE_CURSOR_HAND,
};

Input get_inputs(Window *window);
bool button_released(ButtonState *s);
bool button_pressed(ButtonState *s);
u8 get_button_presses(ButtonState *s);
void swap_buffers(Window *window);
bool create_window(s32 w, s32 h, String title, Window *window_output);
// bool destroy_window(Window *window);

bool set_clipboard_text(Window *window, String s);
String get_clipboard_text(Window *window);
void set_mouse_cursor(MouseCursor cursor);