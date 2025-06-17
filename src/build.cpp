#include "arena.cpp"
#include "meta.cpp"
#include "common.cpp"
#include "string.cpp"
#include "main.cpp"
#include "window.cpp"

#define NODE_INVALID WIN_NODE_INVALID
#include <glad/glad.c>
#define libGL wgl_libGL
#define gladGetProcAddressPtr wgl_gladGetProcAddressPtr
#define get_proc wgl_get_proc
#define get_exts wgl_get_exts
#define free_exts wgl_free_exts
#define has_ext wgl_has_ext

#include <glad/glad_wgl.c>