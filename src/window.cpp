#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glad/glad.h>
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


    // g_inputs.scrollx = 0;
    g_inputs.scrolly = 0;

    return inputs;
}


u8 get_button_presses(ButtonState *s) {
    if (s->ended_down) {
        return s->transitions / 2;
    } else {
        return (s->transitions + 1) / 2;
    }
}


void update_button_state(ButtonState *s) {
    s->transitions += 1;
}

LRESULT window_callback( HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param) {
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

        case WM_KEYDOWN: 
        case WM_KEYUP: 
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {

            u32 VKCode = (u32)w_param;
            bool was_down = (l_param & (1 << 30)) != 0;
            bool is_down = (l_param & (1 << 31)) == 0;
            if (was_down == is_down) break;

            switch (VKCode) {
                case 'A': update_button_state(g_inputs.buttons + BUTTON_A); break;
                case 'B': update_button_state(g_inputs.buttons + BUTTON_B); break;
                case 'C': update_button_state(g_inputs.buttons + BUTTON_C); break;
                case 'D': update_button_state(g_inputs.buttons + BUTTON_D); break;
                case 'E': update_button_state(g_inputs.buttons + BUTTON_E); break;
                case 'F': update_button_state(g_inputs.buttons + BUTTON_F); break;
                case 'G': update_button_state(g_inputs.buttons + BUTTON_G); break;
                case 'H': update_button_state(g_inputs.buttons + BUTTON_H); break;
                case 'I': update_button_state(g_inputs.buttons + BUTTON_I); break;
                case 'J': update_button_state(g_inputs.buttons + BUTTON_J); break;
                case 'K': update_button_state(g_inputs.buttons + BUTTON_K); break;
                case 'L': update_button_state(g_inputs.buttons + BUTTON_L); break;
                case 'M': update_button_state(g_inputs.buttons + BUTTON_M); break;
                case 'N': update_button_state(g_inputs.buttons + BUTTON_N); break;
                case 'O': update_button_state(g_inputs.buttons + BUTTON_O); break;
                case 'P': update_button_state(g_inputs.buttons + BUTTON_P); break;
                case 'Q': update_button_state(g_inputs.buttons + BUTTON_Q); break;
                case 'R': update_button_state(g_inputs.buttons + BUTTON_R); break;
                case 'S': update_button_state(g_inputs.buttons + BUTTON_S); break;
                case 'T': update_button_state(g_inputs.buttons + BUTTON_T); break;
                case 'U': update_button_state(g_inputs.buttons + BUTTON_U); break;
                case 'V': update_button_state(g_inputs.buttons + BUTTON_V); break;
                case 'W': update_button_state(g_inputs.buttons + BUTTON_W); break;
                case 'X': update_button_state(g_inputs.buttons + BUTTON_X); break;
                case 'Y': update_button_state(g_inputs.buttons + BUTTON_Y); break;
                case 'Z': update_button_state(g_inputs.buttons + BUTTON_Z); break;
                case VK_ESCAPE: update_button_state(g_inputs.buttons + BUTTON_ESC); break;
                case VK_SPACE: update_button_state(g_inputs.buttons + BUTTON_SPACE); break;
                case VK_LSHIFT: update_button_state(g_inputs.buttons + BUTTON_LSHIFT); break;
                case VK_LCONTROL: update_button_state(g_inputs.buttons + BUTTON_LCTRL); break;
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
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP: {
                    update_button_state(g_inputs.buttons + BUTTON_ML);
                } break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP: {
                    update_button_state(g_inputs.buttons + BUTTON_MM);
                } break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP: {
                    update_button_state(g_inputs.buttons + BUTTON_MR);
                } break;
            }
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

bool create_window(Window *window_output) {
    if (!window_output) return false;

    HINSTANCE handle = GetModuleHandle(nullptr);
    WNDCLASS window_class = {};

    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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
    if (windowclass_atom) {
        Window_Internal window = {};

        window.handle = CreateWindowEx(
            0,
            "window_class",
            "window",
            WS_BORDER|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            nullptr,
            nullptr,
            handle,
            nullptr
        );
        if (window.handle) {
            window.device_context = GetDC(window.handle);
            memcpy(window_output, &window, sizeof(window));
        } else {
            LOG_ERROR("Failed to create window %ld\n", GetLastError());
            UnregisterClass(window_class.lpszClassName, nullptr);
            return false;
        }
    } else {
        LOG_ERROR("Failed to register window class\n");
        return false;
    }    
    return true;
}

bool set_window_context(Window *w) {
    Window_Internal *window = (Window_Internal *)w;

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

    int pixel_format_index = ChoosePixelFormat(window->device_context, &pfd);
    if (pixel_format_index == 0) {
        LOG_ERROR("Failed to choose pixel format\n");
        return false;
    }

    if (!SetPixelFormat(window->device_context, pixel_format_index, &pfd)) {
        LOG_ERROR("Failed to set pixel format\n");
        return false;
    }

    HGLRC hglrc = wglCreateContext(window->device_context);
    if (!hglrc) {
        LOG_ERROR("Failed to create context\n");
        return false;
    }
    if (!wglMakeCurrent(window->device_context, hglrc)) {
        LOG_ERROR("Failed to make context\n");
        return false;
    }

    return true;
}

void swap_buffers(Window *w) {
    Window_Internal *window = (Window_Internal *)w; 
    SwapBuffers(window->device_context);
}

int main2(void) {


    Window window;
    if (!create_window(&window)) return 1;
    if (!set_window_context(&window)) return 1;
    if (!gladLoadGL()) {
        LOG_ERROR("Failed to load newer OpenGl functions\n");
        return 1;
    }
    printf("OpenGl version %s\n", glGetString(GL_VERSION));
    printf("OpenGl renderer %s\n", glGetString(GL_RENDERER));
    


    bool running = true;
    f32 x = 0.0f;
    while (running) {
        Input input = get_inputs(&window);
        x += 0.001f;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(x, x, x, 1.0f);
        swap_buffers(&window);
    }

    // wglMakeCurrent(hdc, nullptr),
    // wglDeleteContext(hglrc);
    // DestroyWindow(window);
    return 0;
}