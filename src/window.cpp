#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include "window.h"
Input g_inputs = {};

struct Window_Internal {
    HWND handle;
    HDC device_context;
};

Input get_inputs(Window *w) {
    Window_Internal *window = (Window_Internal *)w;

    MSG msg;
    while (PeekMessage(&msg, window->handle, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            g_inputs.quit = true;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Input inputs = g_inputs;

    for (u64 i = 0; i < ARRAY_SIZE(g_inputs.buttons); ++i) {
        ButtonState *b = g_inputs.buttons + i;


        if (b->transitions % 2 == 1) {
            b->ended_down = !b->ended_down;
        }
        b->transitions = 0;
    }

    g_inputs.char_count = 0;
    g_inputs.key_count = 0;


    // g_inputs.scrollx = 0;
    g_inputs.scrolly = 0;

    return inputs;
}
bool button_released(ButtonState *s) {
    return s->ended_down && s->transitions > 0;
}
bool button_pressed(ButtonState *s) {
    return !s->ended_down && s->transitions > 0;
}

char clipboard_text[4096];

bool set_clipboard_text(Window *w, String s) {
    Window_Internal *window = (Window_Internal *)w;

    if (OpenClipboard(window->handle)) {
        EmptyClipboard();
        snprintf(clipboard_text, sizeof(clipboard_text), "%.*s", (s32)s.count, s.dat);

        SetClipboardData(CF_TEXT, clipboard_text);

        CloseClipboard();
        return true;
    }

    return false;
}
String get_clipboard_text(Window *w) {
    Window_Internal *window = (Window_Internal *)w;

    String s = {};
    if (OpenClipboard(window->handle)) {
        void *data = GetClipboardData(CF_TEXT);
        u64 len = strlen((char *)data);
        memcpy(clipboard_text, data, len);
        CloseClipboard();
    }

    s.dat = (u8 *)clipboard_text;
    s.count = strlen(clipboard_text);
    return s;
}

u8 get_button_presses(ButtonState *s) {
    if (s->ended_down) {
        return s->transitions / 2;
    } else {
        return (s->transitions + 1) / 2;
    }
}


void set_mouse_cursor(MouseCursor cursor) {
    const char *ms = nullptr;
    switch (cursor) {

        case MOUSE_CURSOR_ARROW: ms = IDC_ARROW; break;
        case MOUSE_CURSOR_IBEAM: ms = IDC_IBEAM; break;
        case MOUSE_CURSOR_RESIZE_NWSE: ms = IDC_SIZENWSE; break;
        case MOUSE_CURSOR_RESIZE_NESW: ms = IDC_SIZENESW; break;
        case MOUSE_CURSOR_RESIZE_WE: ms = IDC_SIZEWE; break;
        case MOUSE_CURSOR_RESIZE_NS: ms = IDC_SIZENS; break;
        case MOUSE_CURSOR_HAND: ms = IDC_HAND; break;
    }
    
    HCURSOR cursor_handle = (HCURSOR)LoadImage(nullptr, ms, IMAGE_CURSOR, 0, 0, LR_SHARED);
    SetCursor(cursor_handle);
}
void update_button_state(Button button, bool is_down) {
    g_inputs.buttons[button].transitions += 1;
    if (button < BUTTON_ML && is_down) {
        g_inputs.keys[g_inputs.key_count++] = button;
    }
}

LRESULT window_callback(HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (msg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP: {
        } break;
        case WM_PAINT: {
            PAINTSTRUCT p;
            BeginPaint(wnd, &p);
            EndPaint(wnd, &p);
        } break;

        case WM_SIZE: {
            g_inputs.resized = true;
            g_inputs.screen_width = LOWORD(l_param); 
            g_inputs.screen_height = HIWORD(l_param); 
        } break;

        case WM_KEYDOWN: 
        case WM_KEYUP: 
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {

            u32 VKCode = (u32)w_param;
            bool was_down = (l_param & (1 << 30)) != 0;
            bool is_down = (l_param & (1 << 31)) == 0;
            if (was_down == is_down) break;

            switch (VKCode) {
                case 'A': update_button_state(BUTTON_A, is_down); break;
                case 'B': update_button_state(BUTTON_B, is_down); break;
                case 'C': update_button_state(BUTTON_C, is_down); break;
                case 'D': update_button_state(BUTTON_D, is_down); break;
                case 'E': update_button_state(BUTTON_E, is_down); break;
                case 'F': update_button_state(BUTTON_F, is_down); break;
                case 'G': update_button_state(BUTTON_G, is_down); break;
                case 'H': update_button_state(BUTTON_H, is_down); break;
                case 'I': update_button_state(BUTTON_I, is_down); break;
                case 'J': update_button_state(BUTTON_J, is_down); break;
                case 'K': update_button_state(BUTTON_K, is_down); break;
                case 'L': update_button_state(BUTTON_L, is_down); break;
                case 'M': update_button_state(BUTTON_M, is_down); break;
                case 'N': update_button_state(BUTTON_N, is_down); break;
                case 'O': update_button_state(BUTTON_O, is_down); break;
                case 'P': update_button_state(BUTTON_P, is_down); break;
                case 'Q': update_button_state(BUTTON_Q, is_down); break;
                case 'R': update_button_state(BUTTON_R, is_down); break;
                case 'S': update_button_state(BUTTON_S, is_down); break;
                case 'T': update_button_state(BUTTON_T, is_down); break;
                case 'U': update_button_state(BUTTON_U, is_down); break;
                case 'V': update_button_state(BUTTON_V, is_down); break;
                case 'W': update_button_state(BUTTON_W, is_down); break;
                case 'X': update_button_state(BUTTON_X, is_down); break;
                case 'Y': update_button_state(BUTTON_Y, is_down); break;
                case 'Z': update_button_state(BUTTON_Z, is_down); break;
                case VK_ESCAPE: update_button_state(BUTTON_ESC, is_down); break;
                case VK_SPACE: update_button_state(BUTTON_SPACE, is_down); break;
                case VK_LSHIFT: update_button_state(BUTTON_LSHIFT, is_down); break;
                case VK_LCONTROL: update_button_state(BUTTON_LCTRL, is_down); break;
                case VK_BACK: update_button_state(BUTTON_BACKSPACE, is_down); break;
                case VK_LEFT: update_button_state(BUTTON_LEFT, is_down); break;
                case VK_UP: update_button_state(BUTTON_UP, is_down); break;
                case VK_RIGHT: update_button_state(BUTTON_RIGHT, is_down); break;
                case VK_DOWN: update_button_state(BUTTON_DOWN, is_down); break;
                case VK_HOME: update_button_state(BUTTON_HOME, is_down); break;
                case VK_END: update_button_state(BUTTON_END, is_down); break;
                case VK_DELETE: update_button_state(BUTTON_DELETE, is_down); break;
            }




        } break;

        case WM_CHAR: {
            g_inputs.chars[g_inputs.char_count++] = (u32)w_param;
        } break;

        //TODO this does not handle mouse clicks out of the client window area
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            g_inputs.mx = (s16)(l_param & 0xffff);
            g_inputs.my = (s16)((l_param >> 16) & 0xffff);
            bool is_down = false;
            Button button;
            switch (msg) {
                case WM_LBUTTONDOWN: {
                    is_down = true;
                    button = BUTTON_ML;
                } break;
                case WM_LBUTTONUP: {
                    button = BUTTON_ML;
                } break;
                case WM_MBUTTONDOWN: {
                    is_down = true;
                    button = BUTTON_MM;
                } break;
                case WM_MBUTTONUP: {
                    button = BUTTON_MM;
                } break;
                case WM_RBUTTONDOWN: {
                    is_down = true;
                    button = BUTTON_MR;
                } break;
                case WM_RBUTTONUP: {
                    button = BUTTON_MR;
                } break;
            }
            update_button_state(button, is_down);
        } break;
        case WM_MOUSEMOVE: {
            g_inputs.mx = (s16)(l_param & 0xffff);
            g_inputs.my = (s16)((l_param >> 16) & 0xffff);
        } break;


        case WM_MOUSEWHEEL: {
            s32 wheel_delta = (s32)((w_param >> 16) & 0xffff) / WHEEL_DELTA;
            g_inputs.scrolly += wheel_delta;
            POINT p = {};
            p.x = l_param & 0xffff;
            p.y = (l_param >> 16) & 0xffff;
            ScreenToClient(wnd, &p);
            g_inputs.mx = (s16)p.x;
            g_inputs.my = (s16)p.y;
            printf("%d %d\n", g_inputs.mx, g_inputs.my);
        } break;



        default: {
            result = DefWindowProc(wnd, msg, w_param, l_param);
        } break;

    }
    return result;
}



static_assert(sizeof(Window) == sizeof(Window_Internal), "window and window_internal sizes have to be the same");

bool init_wgl_extensions(WNDCLASS *window_class, HINSTANCE module_handle) {

    HWND dummy_hwnd = CreateWindowEx(
        0,
        window_class->lpszClassName,
        "Dummy",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        module_handle,
        nullptr
    );
    if (!dummy_hwnd) {
        LOG_ERROR("Failed to create window %ld\n", GetLastError());
        UnregisterClass(window_class->lpszClassName, nullptr);
        return false;
    }

    HDC dummy_hdc = GetDC(dummy_hwnd);


    PIXELFORMATDESCRIPTOR pfd = {};

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 0;
    pfd.cRedShift = 0;
    pfd.cGreenBits = 0;
    pfd.cGreenShift = 0;
    pfd.cBlueBits = 0;
    pfd.cBlueShift = 0;
    pfd.cAlphaBits = 0;
    pfd.cAlphaShift = 0;
    pfd.cAccumBits = 0;
    pfd.cAccumRedBits = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0;
    pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.cAuxBuffers = 0;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask = 0;
    pfd.dwDamageMask = 0;

    int pixel_format_index = ChoosePixelFormat(dummy_hdc, &pfd);
    if (pixel_format_index == 0) {
        LOG_ERROR("Failed to choose pixel format\n");
        return false;
    }
    if (!SetPixelFormat(dummy_hdc, pixel_format_index, &pfd)) {
        LOG_ERROR("Failed to set pixel format\n");
        return false;
    }
    HGLRC dummy_hglrc = wglCreateContext(dummy_hdc);
    if (!dummy_hglrc) {
        LOG_ERROR("Failed to create context\n");
        return false;
    }
    if (!wglMakeCurrent(dummy_hdc, dummy_hglrc)) {
        LOG_ERROR("Failed to make context\n");
        return false;
    }
    if (!gladLoadWGL(dummy_hdc)) {
        LOG_ERROR("Failed to load wgl extensions\n");
        return false;
    }
    if (!wglDeleteContext(dummy_hglrc)) {
        LOG_ERROR("Failed to delete context\n");
        return false;
    }
    DestroyWindow(dummy_hwnd);
    return true;
};

bool create_window(s32 w, s32 h, String title, Window *window_output) {
    if (!window_output) return false;

    HINSTANCE handle = GetModuleHandle(nullptr);
    WNDCLASS window_class = {};

    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = window_callback;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = handle;
    window_class.hIcon = nullptr;
    window_class.hCursor = nullptr;
    window_class.hbrBackground = nullptr;
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = "window_class";

    ATOM windowclass_atom = RegisterClass(&window_class);
    if (!windowclass_atom) {
        LOG_ERROR("Failed to register class");
        return false;
    }

    if (!init_wgl_extensions(&window_class, handle)) {
        return false;
    }

    Window_Internal window = {};
    window.handle = CreateWindowEx(
        0,
        window_class.lpszClassName,
        (const char *)title.dat,
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        w,
        h,
        nullptr,
        nullptr,
        handle,
        nullptr
    );
    if (!window.handle) {
        LOG_ERROR("Failed to create window %ld\n", GetLastError());
        UnregisterClass(window_class.lpszClassName, nullptr);
        return false;
    }
    window.device_context = GetDC(window.handle);

    s32 pixel_format;
    u32 num_formats;
    {
        s32 attrib_list[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0, // End
        };

        if (!wglChoosePixelFormatARB(window.device_context, attrib_list, nullptr, 1, &pixel_format, &num_formats)) {
            LOG_ERROR("Failed to choose pixel format\n");
            return false;
        }
    }

    PIXELFORMATDESCRIPTOR pfd;
    if (!SetPixelFormat(window.device_context, pixel_format, &pfd)) {
        LOG_ERROR("Failed to set pixel format\n");
        return false;
    }

    HGLRC new_hglrc;
    {
        s32 attrib_list[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0, // End
        };
        new_hglrc = wglCreateContextAttribsARB(window.device_context, 0, attrib_list);
    }
    if (!wglMakeCurrent(window.device_context, new_hglrc)) {
        LOG_ERROR("Failed to set new context\n");
        return false;
    }

    memcpy(window_output, &window, sizeof(window));
    ShowWindow(window.handle, SW_SHOW);
    return true;
}

// bool destroy_window(Window *w) {
//     Window_Internal *window = (Window_Internal *)w;

//     wglMakeCurrent(hdc, nullptr),
//     wglDeleteContext(hglrc);
//     DestroyWindow(window);
// }

void swap_buffers(Window *w) {
    Window_Internal *window = (Window_Internal *)w; 
    SwapBuffers(window->device_context);
}

// int main2(void) {

//     Window window;
//     if (!create_window(500, 500, str_lit("Window title"), &window)) return 1;
//     if (!set_window_context(&window)) return 1;
//     if (!gladLoadGL()) {
//         LOG_ERROR("Failed to load newer OpenGl functions\n");
//         return 1;
//     }
//     printf("OpenGl version %s\n", glGetString(GL_VERSION));
//     printf("OpenGl renderer %s\n", glGetString(GL_RENDERER));
    


//     bool running = true;
//     f32 x = 0.0f;
//     while (running) {
//         Input input = get_inputs(&window);
//         x += 0.001f;
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//         glClearColor(x, x, x, 1.0f);
//         swap_buffers(&window);
//     }

//     return 0;
// }