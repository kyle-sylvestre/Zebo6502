#ifndef CPP_COMMON_H
#define CPP_COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32 bool32;
typedef bool bool8;
typedef uint32_t uint;
typedef float f32;
typedef double f64;

// internal logging functions defined in log.cpp
void impl_print_debug(const char *format, ...);
void impl_print_warning(const char *file, int line, const char *format, ...);
void impl_print_error(const char *file, int line, const char *format, ...);

// return formatted thread_local buffer
// USE ONCE, DON'T STORE THE RETURN VALUE
const char *impl_tempf(int dummy_snprintf_result, const char *format, ...);

uint64 fnv64_1a(const uint8 *ptr, size_t byte_count);

// MSVC 2015 compile time format family checking (printf, scanf)
#define CHECK_FORMAT(format, ...) snprintf(NULL, 0, format, __VA_ARGS__)

#define BRK __debugbreak()

#ifndef NDEBUG
    #define azzert(Expression)\
        do {                                                \
            if(!(Expression)){                              \
                errorf("assert fail \"" #Expression "\"");  \
                __debugbreak();                             \
                exit(0);                                    \
            }                                               \
        } while(0)
#else
    #define azzert(...) ((void)0)
#endif

#define tempf(format, ...) impl_tempf(CHECK_FORMAT(format, __VA_ARGS__), format, __VA_ARGS__)

#ifndef NDEBUG
    #define debugf(format, ...) do {            \
        CHECK_FORMAT(format, __VA_ARGS__);      \
        impl_print_debug(format, __VA_ARGS__);  \
    } while(0)
#else 
    #define debugf(format, ...) ((void)0)
#endif

#define warningf(format, ...) do {                              \
    CHECK_FORMAT(format, __VA_ARGS__);                          \
    impl_print_warning(__FILE__, __LINE__, format, __VA_ARGS__);\
} while(0)

#define errorf(format, ...) do {                                \
    CHECK_FORMAT(format, __VA_ARGS__);                          \
    impl_print_error(__FILE__, __LINE__, format, __VA_ARGS__);  \
    __debugbreak();                                             \
    exit(0);                                                    \
} while(0)

#define DEFAULT_INVALID default: errorf("hit invalid default case"); break;
#define Unimplemented() errorf("not implemented yet");
#define FatalPath() errorf("hit invalid path");

#define OffsetOf(struct_name, struct_member) ((size_t)&(((struct_name*)0)->struct_member))

template<typename T, size_t N>
constexpr size_t ArrayCount(T(&array)[N]) { (void)array; return N; }

#define ElemSize(arr) (sizeof(arr[0]))

#define ZeroArray(arr) do { for(auto &a : arr) a = {}; }while(0)
#define ZeroStruct(str) memset(str, 0, sizeof(str))


#endif
