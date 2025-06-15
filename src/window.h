#include "common.h"

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
    BUTTON_LSHIFT,
    BUTTON_LCTRL,
    BUTTON_ML, // mouse
    BUTTON_MM,
    BUTTON_MR,
    BUTTON_M4,
    BUTTON_M5,



    BUTTON_COUNT,
};
struct Input {
    bool quit;
    s16 mx;
    s16 my;

    // f64 deltamx;
    // f64 deltamy;

    // s32 scrollx;
    s32 scrolly;

    u32 chars[16];
    u32 char_count;

    ButtonState buttons[BUTTON_COUNT];
};

Input get_inputs(Window *window);
u8 get_button_presses(ButtonState *s);
void swap_buffers(Window *window);