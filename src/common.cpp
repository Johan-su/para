#include <stdarg.h>
#include <stdint.h>

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


#ifdef _MSC_VER
    #define MSVC
#else
    #ifdef __clang__
        #define CLANG
    #else
        #ifdef __GNUC__ 
            #define GCC
        #endif
    #endif
#endif



#ifdef MSVC
    #ifdef _M_AMD64
    #define x86_64
    #endif
#endif

#if defined(GCC) || defined(CLANG)
    #ifdef __amd64__
    #define x86_64
    #endif
#endif


#define STR_(x) #x
#define STR(x) STR_(x)

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))
#define assert(condition)                               \
do {                                                    \
    if (!(condition)) {                                 \
        assert_function(STR(condition), __FILE__, __LINE__); \
    }                                                   \
} while (0)

#define todo() assert(false && "TODO")




void assert_function(const char *cond, const char *file, s32 line) {
#ifdef x86_64 
    asm("int3");
#else
    #error "assert function only works for x86_64 for now"
#endif
}