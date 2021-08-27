/* date = October 9th 2020 11:00 am */

// NOTE(Jai): A lot of the structure of this utils file is inspired by
// the work of Casey Muratori and Allen Webster (aka Mr.4thDimension)

#ifndef UTILS_H
#define UTILS_H

// NOTE(Jai): STB-style implementation: #define "JAI_UTILS_IMPLEMENTATION" once in the c/cpp file,
// above the #include of this file.

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

//~ CONTEXT DETECTION
// NOTE(Jai): Only supporting x86/x64 on Windows/Linux cuz Apple can suck it.

#define COMPILER_CLANG 0
#define COMPILER_CL 0
#define COMPILER_GCC 0

#define OS_WINDOWS 0
#define OS_LINUX 0

#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 0

#if defined(__clang__)
#undef COMPILER_CLANG
#define COMPILER_CLANG 1
#if defined (_WIN32)
#undef OS_WINDOWS
#define OS_WINDOWS 1

#if defined(__amd64__)
#undef ARCH_X64
#define ARCH_X64 1
#elif defined(__i386__)
#undef ARCH_X86
#define ARCH_X86 1
#elif defined(__arm__)
#undef ARCH_ARM
#define ARCH_ARM 1
#elif defined(__aarch64__)
#undef ARCH_ARM64
#define ARCH_ARM64 1
#else
#error missing ARCH detection
#endif // ARCH

#elif defined (__gnu_linux__)
#undef OS_LINUX
#define OS_LINUX 1

#if defined(__amd64__)
#undef ARCH_X64
#define ARCH_X64 1
#elif defined(__i386__)
#undef ARCH_X86
#define ARCH_X86 1
#elif defined(__arm__)
#undef ARCH_ARM
#define ARCH_ARM 1
#elif defined(__aarch64__)
#undef ARCH_ARM64
#define ARCH_ARM64 1
#else
#error missing ARCH detection
#endif // ARCH

#else
#error missing OS detection
#endif // OS

#elif defined(_MSC_VER)
#undef COMPILER_CL
#define COMPILER_CL 1
#if defined (_WIN32)
#undef OS_WINDOWS
#define OS_WINDOWS 1

#if defined(_M_AMD64)
#undef ARCH_X64
#define ARCH_X64
#elif defined(_M_IX86)
#undef ARCH_X86
#define ARCH_X86 1
#elif defined(_M_ARM)
#undef ARCH_ARM
#define ARCH_ARM 1
#else
#error missing ARCH detection
#endif // ARCH

#else
#error missing OS detection
#endif // OS

#elif defined(__GNUC__)
#undef COMPILER_GCC
#define COMPILER_GCC 1
#if defined (_WIN32)
#undef OS_WINDOWS
#define OS_WINDOWS 1

#if defined(__amd64__)
#undef ARCH_X64
#define ARCH_X64 1
#elif defined(__i386__)
#undef ARCH_X86
#define ARCH_X86 1
#elif defined(__arm__)
#undef ARCH_ARM
#define ARCH_ARM 1
#elif defined(__aarch64__)
#undef ARCH_ARM64
#define ARCH_ARM64 1
#else
#error missing ARCH detection
#endif // ARCH
#elif defined (__gnu_linux__)
#undef OS_LINUX
#define OS_LINUX 1

#if defined(__amd64__)
#undef ARCH_X64
#define ARCH_X64 1
#elif defined(__i386__)
#undef ARCH_X86
#define ARCH_X86 1
#elif defined(__arm__)
#undef ARCH_ARM
#define ARCH_ARM 1
#elif defined(__aarch64__)
#undef ARCH_ARM64
#define ARCH_ARM64 1
#else
#error missing ARCH detection
#endif // ARCH

#else
#error missing OS detection
#endif // OS

#else
#error no context cracking for this compiler
#endif // COMPILER

//~ TYPEDEFS
// NOTE(Jai): Typedefs for stdint types for brevity

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32; // Basic boolean type for C
typedef unsigned int uint;

typedef float f32;
typedef double f64;
typedef u64 uintptr_t;
typedef uintptr_t uintptr;
typedef size_t memoryIndex;

// NOTE(Jai): To differntiate between function pointers and data pointers
typedef void void_function(void);

// NOTE(Jai): Because C does not have an in-built String type
typedef struct String8 {
    char* str;
    memoryIndex length;
} String8;

//~ MACROS

#define localPersist static
#define global static
#define internal static
#define readOnly const
#define false 0
#define true 1

#define KB(value) (value * 1024LL)
#define MB(value) (KB(value) * 1024LL)
#define GB(value) (MB(value) * 1024LL)
#define TB(value) (GB(value) * 1024LL)

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))

#define STMT(s) do { (s) } while (0)

#define ASSERT_BREAK(expression)\
if(!(expression)) {\
*(volatile int*)0 = 0;\
}
#if JAI_INTERNAL
#undef ASSERT
#define ASSERT(expression) ASSERT_BREAK(expression)
#else
#define ASSERT(expression)
#endif

#define UINT_FROM_PTR(p) (u64)((u8*)(p) - (u8*)0)
#define PTR_FROM_UINT(n) (void*)((u8*)0 + (n))

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#define GLUE_(a, b) a##b
#define GLUE(a, b) GLUE_(a, b)

#define MEMBER(t, m) (((t*)0)->m)
#define OFFSET_OF_MEMBER(t, m) UINT_FROM_PTR(&MEMBER(t, m))

#define IS_POWER_OF_2(x) ((x & (x - 1)) == 0)

#define OG_FLT_MAX FLT_MAX
#define OG_INT_MAX INT_MAX

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)

#define CLAMP(a, x, b) ((x) < (a) ? (a) :\
((x) > (b) ? (b) : (x)))
#define CLAMP_TOP(a, b) MIN(a, b)
#define CLAMP_BOT(a, b) MAX(a, b)

//~ Basic Constants

global i8 i8_min = (i8)0x00;
global i16 i16_min = (i16)0x0000;
global i32 i32_min = (i32)0x00000000;
global i64 i64_min = (i64)0x0000000000000000llu;

global i8 i8_max = (i8)0x7f;
global i16 i16_max = (i16)0x7fff;
global i32 i32_max = (i32)0x7fffffff;
global i64 i64_max = (i64)0x7fffffffffffffffllu;

global u8 u8_min = (u8)0xff;
global u16 u16_min = (u16)0xffff;
global u32 u32_min = (u32)0xffffffff;
global u64 u64_min = (u64)0xffffffffffffffffllu;

global f32 machine_epsilon_f32 = 1.1920929e-7f;
global f64 machine_epsilon_f64 = 2.220446e-16;

global f32 pi_f32 = 3.14159265359f;
global f32 tau_f32 = 6.28318530718f;
global f64 pi_f64 = 3.14159265359;
global f64 tau_f64 = 6.28318530718;

//~ FLOAT CONSTANTS

global f32 f32_max = FLT_MAX;
global f32 f32_epsilon = FLT_EPSILON;
global f32 f32_max_exp = FLT_MAX_EXP;

global f64 f64_max = DBL_MAX;
global f64 f64_epsilon = DBL_EPSILON;
global f64 f64_max_exp = DBL_MAX_EXP;

//- MEMORY STUFF

// NOTE(Jai): p corresponds to a pointer
#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(p) MEMORY_ZERO((p), sizeof((p)))
#define MEMORY_ZERO_TYPED(p, n) MEMORY_ZERO((p), sizeof(*(p)) * (n))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define MEMORY_COPY(dest, source, n) memmove((dest), (source), (n))
#define MEMORY_COPY_STRUCT(dest, source) MEMORY_COPY((dest), (source),\
MIN(sizeof(*(dest)), sizeof(*(source))))
#define MEMORY_COPY_ARRAY(dest, source) MEMORY_COPY((dest), (source),\
MIN(sizeof((dest)), sizeof((source))))
#define MEMORY_COPY_TYPED(dest, source, n) MEMORY_COPY((dest), (source),\
MIN(sizeof(*(dest)), (sizeof(*(source)) * n)))

//~ SYMBOLIC CONSTANTS

typedef enum Axis {
    AXIX_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_W
} Axis;

typedef enum Side {
    SIDE_MIN,
    SIDE_MAX
} Side;

typedef enum Month {
    MONTH_JAN,
    MONTH_FEB,
    MONTH_MAR,
    MONTH_APR,
    MONTH_MAY,
    MONTH_JUN,
    MONTH_JUL,
    MONTH_AUG,
    MONTH_SEP,
    MONTH_OCT,
    MONTH_NOV,
    MONTH_DEC,
} Month;

typedef enum DayOfWeek {
    DAY_OF_WEEK_SUN,
    DAY_OF_WEEK_MON,
    DAY_OF_WEEK_TUE,
    DAY_OF_WEEK_WED,
    DAY_OF_WEEK_THU,
    DAY_OF_WEEK_FRI,
    DAY_OF_WEEK_SAT,
} DayOfWeek;

typedef enum OperatingSystem {
    OPERATING_SYSTEM_NULL,
    OPERATING_SYSTEM_WINDOWS,
    OPERATING_SYSTEM_LINUX,
    OPERATING_SYSTEM_COUNT
} OperatingSystem;

typedef enum Architecture {
    ARCHITECTURE_NULL,
    ARCHITECTURE_X86,
    ARCHITECTURE_X64,
    ARCHITECTURE_ARM,
    ARCHITECTURE_ARM64,
    ARCHITECTURE_COUNT
} Architecture;

global char operatingSystem_String[3][8] = {
    { "Null\0" },
    { "Windows\0" },
    { "Linux\0" }
};

global char architecture_String[5][6] = {
    { "Null\0" },
    { "x86\0" },
    { "x64\0" },
    { "arm\0" },
    { "arm64\0" }
};

global char dayOfWeek_String[7][10] = {
    { "Sunday\0" },
    { "Monday\0" },
    { "Tuesday\0" },
    { "Wednesday\0" },
    { "Thursday\0" },
    { "Friday\0" },
    { "Saturday\0" }
};

global char month_String[12][10] = {
    { "January\0" },
    { "February\0" },
    { "March\0" },
    { "April\0" },
    { "May\0" },
    { "June\0" },
    { "July\0" },
    { "August\0 "},
    { "September\0" },
    { "October\0" },
    { "November\0" },
    { "December\0" },
};


//- SYMBOLIC CONSTANT FUNCTIONS
internal OperatingSystem
operatinSystemFromContext(void);

internal Architecture
architectureFromContext(void);

internal char*
stringFromOperatingSystem(OperatingSystem os);

internal char*
stringFromArchitecure(Architecture arch);

internal char*
stringFromMonth(Month month);

internal char*
stringFromDayOfWeek(DayOfWeek dayOfWeek);

//~ LINKED LISTS MACROS

// NOTE(Jai): No bounds checking/automatic growing for linked lists implemented.

// TODO(Jai):
// Doubly linked lists
//  Push Back
//  Push Front
//  Remove
// Singly linked list queue
//  Push Back
//  Push Front
//  Pop
// Singly linked list stack
//  Push
//  Pop

#define DLL_PUSH_BACK_NP(first, last, node, next, prev) do {\
if ((first) == 0) { (first) = (last) = (node); (node)->next = 0; (node)->prev = 0; }\
else { (node)->prev = (last); (last)->next = (node); (last) = (node); (node)->next = 0; }\
} while(0)
#define DLL_PUSH_BACK(first, last, node) DLL_PUSH_BACK_NP(first, last, node, next, prev)
#define DLL_PUSH_FRONT(first, last, node) DLL_PUSH_BACK_NP(last, first, node, prev, next)
#define DLL_REMOVE_NP(first, last, node, next, prev) do {\
if (((first) == (last)) && ((first) == (node))) { (first) = (last) = 0; }\
else if ((node) == (first)) { (first) = (node)->next; (first)->prev = 0;}\
else if ((node) == (last)) { (last) = (node)->prev; (last)->next = 0; }\
else { (node)->prev->next = (node)->next; (node)->next->prev = (node)->prev; }\
} while(0)
#define DLL_REMOVE(first, last, node) DLL_REMOVE_NP(first, last, node, next,prev)

#define SLL_QUEUE_PUSH_N(first, last, node, next) do {\
if ((first) == 0) { (first) = (last) = (node);}\
else { (last)->next = (node); (last) = (node); (node)->next = 0; }\
} while(0)
#define SLL_QUEUE_PUSH(first, last, node) SLL_QUEUE_PUSH_N(first, last, node, next)
#define SLL_QUEUE_PUSH_FRONT_N(first, last, node, next) do {\
if ((first) == 0) { (first) = (last) = (node); }\
else { (node)->next = (first); (first) = (node); }\
} while(0)
#define SLL_QUEUE_PUSH_FRONT(first, last, node) SLL_QUEUE_PUSH_FRONT_N(first, last, node, next)
#define SLL_QUEUE_POP_N(first, last, next) do {\
if ((first) == (last)) { (first) = (last) = 0; }\
else { (first) = (first)->next; }\
} while(0)
#define SLL_QUEUE_POP(first, last) SLL_QUEUE_POP_N(first, last, next)

#define SLL_STACK_PUSH_N(first, last, node, next) do {\
(node)->next = (first);\
(first) = (node);\
} while(0)
#define SLL_STACK_PUSH(first, last, node) SLL_STACK_PUSH_N(first, last, node, next)
#define SLL_STACK_POP_N(first, last, next) do {\
if((first) == 0) { break; }\
else { (first) = (first)->next; }\
} while(0)
#define SLL_STACK_POP(first, last) SLL_STACK_POP_N(first, last, next)

//~ STRING STUFF
// NOTE(Jai): Concatenate two strings
internal inline void
catStrings(i64 sourceACount, char* sourceA,
           i64 sourceBCount, char* sourceB,
           i64 destCount, char* dest);
// NOTE(Jai): Find the length of a string
internal inline i32
stringLength(char* string);


//~------------ Dynamic Array -------------

// TODO(Jai): Implement own dynamic array

//~-------------Swap ----------------------
#define SWAP(a, b) do { \
u8 _swapTemp[(sizeof(a) == sizeof(b)) ? sizeof(a) : -1]; \
memcpy(_swapTemp, &a, sizeof(a)); \
memcpy(&a, &b, sizeof(a)); \
memcpy(&b, _swapTemp, sizeof(a)); \
} while(0)


//~----------- Number Stuff ----------------
// NOTE(Jai): Round up the float value to a 32 bit integer
internal inline i32
round_floatToI32(f32 number);

// NOTE(Jai): Truncate a float to a 32 bit integer
internal inline i32
truncate_floatToI32(f32 number);

// NOTE(Jai): Floor a float to a 32 bit integer
internal inline i32
floor_floatToI32(f32 number);

// NOTE(Jai): Convert RGBA values expressed in unsigned 32 bit integers
// to a hexadecimal value
internal inline u32
rgba_to_hex(u32 red, u32 green, u32 blue, u32 alpha);

//- Truncate
// NOTE(Jai): Truncate a 64 bit integer to a 32 bit unsigned integer safely
internal inline u32
truncate_safe_i64(i64 value);

// NOTE(Jai): Wrappers around stdlib functions for now, plan on reimplementing them myself
// TODO(Jai): reimplement myself
//- string to numbers
internal inline i64
jai_strtoll(readOnly char* str, char** endPtr, i32 base);

internal inline f32
jai_strtof(readOnly char* str, char** endPtr);

//~ IMPLEMENTATION
#if defined(JAI_UTILS_IMPLEMENTATION)

//- SYMBOLIC CONSTANT FUNCTIONS

internal OperatingSystem
operatinSystemFromContext(void) {
    OperatingSystem result = OPERATING_SYSTEM_NULL;
#if OS_WINDOWS
    result = OPERATING_SYSTEM_WINDOWS;
#elif OS_LINUX
    result = OPERATING_SYSTEM_LINUX;
#endif
    return result;
}

internal Architecture
architectureFromContext(void) {
    Architecture result =  ARCHITECTURE_NULL;
#if ARCH_X64
    result = ARCHITECTURE_X64;
#elif ARCH_X86
    result = ARCHITECTURE_X86;
#elif ARCH_ARM
    result = ARCHITECTURE_ARM;
#elif ARCH_ARM64
    result = ARCHITECTURE_ARM64;
#endif
    return result;
}

internal char*
stringFromOperatingSystem(OperatingSystem os) {
    char* result = operatingSystem_String[os];
    return result;
}

internal char*
stringFromArchitecure(Architecture arch) {
    char* result = architecture_String[arch];
    return result;
}

internal char*
stringFromMonth(Month month) {
    char* result = month_String[month];
    return result;
}

internal char*
stringFromDayOfWeek(DayOfWeek dayOfWeek) {
    char* result = dayOfWeek_String[dayOfWeek];
    return result;
}
//- STRING STUFF

// NOTE(Jai): Concatenate two strings
internal inline void
catStrings(i64 sourceACount, char* sourceA,
           i64 sourceBCount, char* sourceB,
           i64 destCount, char* dest) {
    // TODO(Jai): dest bounds checking
    for (int i = 0; i < sourceACount; ++i) {
        *dest++ = *sourceA++;
    }
    
    for (int i = 0; i < sourceBCount; ++i) {
        *dest++ = *sourceB++;
    }
    
    *dest++ = 0;
}

// NOTE(Jai): Find the length of a string
internal inline i32
stringLength(char* string) {
    int count = 0;
    while (*string++) { ++count; }
    return count;
}

//~----------- Number Stuff ----------------
// NOTE(Jai): Round up the float value to a 32 bit integer
internal inline i32
round_floatToI32(f32 number) {
    return ((i32)(number + 0.5f));
}

// NOTE(Jai): Truncate a float to a 32 bit integer
internal inline i32
truncate_floatToI32(f32 number) {
    return (i32)number;
}

// NOTE(Jai): Floor a float to a 32 bit integer
internal inline i32
floor_floatToI32(f32 number) {
    return (i32)(floorf(number));
}

// NOTE(Jai): Convert RGBA values expressed in unsigned 32 bit integers
// to a hexadecimal value
internal inline u32
rgba_to_hex(u32 red, u32 green, u32 blue, u32 alpha) {
    u32 result = alpha << 24
		| red << 16
        | green << 8
        | blue  << 0;
    
	return result;
}

//- Truncate
// NOTE(Jai): Truncate a 64 bit integer to a 32 bit unsigned integer safely
internal inline u32
truncate_safe_i64(i64 value) {
    // TODO(Jai): Defines for maximum values (u32)
    ASSERT(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    
    return result;
}

// NOTE(Jai): Wrappers around stdlib functions for now, plan on reimplementing them myself
// TODO(Jai): reimplement myself
//- string to numbers
internal inline i64
jai_strtoll(readOnly char* str, char** endPtr, i32 base) {
    return strtoll(str, endPtr, base);
}

internal inline f32
jai_strtof(readOnly char* str, char** endPtr) {
    return strtof(str, endPtr);
}

#endif // IMPLEMENTATION

#endif //UTILS_H
