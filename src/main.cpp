
#include "common.h"
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glad/glad.h>

LRESULT window_callback( HWND wnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_CREATE: {

        } break;

    }
    return DefWindowProc(wnd, msg, w_param, l_param);
}

int main(void) {

    HINSTANCE handle = GetModuleHandle(nullptr);
    WNDCLASS window_class = {};

    window_class.style = CS_OWNDC;
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
        LOG_ERROR("Failed to register window class\n");
        return 1;
    }

    HWND window = CreateWindowEx(
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
    if (!window) {
        LOG_ERROR("Failed to create window %ld\n", GetLastError());
        return 1;
    }
    HDC hdc = GetDC(window);

    PIXELFORMATDESCRIPTOR pfd {};

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

    int pixel_format_index = ChoosePixelFormat(hdc, &pfd);
    if (pixel_format_index == 0) {
        LOG_ERROR("Failed to choose pixel format\n");
        return 1;
    }

    if (!SetPixelFormat(hdc, pixel_format_index, &pfd)) {
        LOG_ERROR("Failed to set pixel format\n");
        return 1;
    }

    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    if (!gladLoadGL()) {
        LOG_ERROR("Failed to load newer OpenGl functions\n");
        return 1;
    }

    printf("OpenGl version %s\n", glGetString(GL_VERSION));
    printf("OpenGl renderer %s\n", glGetString(GL_RENDERER));
    

    MSG msg;
    BOOL ret = GetMessage(&msg, window, 0, 0);
    while (true) {
        if (ret == -1) {
            LOG_ERROR("getmessage erroed\n");
            return 1;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    wglMakeCurrent(hdc, nullptr),
    wglDeleteContext(hglrc);
    DestroyWindow(window);
    return 0;
}