/* date = August 31st 2021 7:06 pm */

#ifndef WIN32_MAIN_H
#define WIN32_MAIN_H

#include "utils.h"
#include "allocators.h"

//~ BACKBUFFER
typedef struct win32_Backbuffer {
    // NOTE(Jai): Pixels are always 32 bits wide, Memory Order: BB GG RR xx
    u32 width;
    u32 height;
    u32 pitch;
    u32 pixelStride;
    BITMAPINFO info;
    void* memory;
} win32_Backbuffer;

typedef struct Clock {
    f64 frequency;
    f64 startTime; // in seconds
    f64 elapsed; // in seconds
} Clock;

//~ PLATFORM STATE
typedef struct win32_State {
    void* memoryBlock;
    u64 totalSize;
    
    char exeFileName[MAX_PATH];
    char* onePastLastSlash;
    
    Clock clock;
    f64 lastTime;
} win32_State;

typedef struct win32_WindowDimensions {
    i32 width, height;
} win32_WindowDimensions;

typedef struct win32_AppCode {
    // either of these callbacks can be 0, check validity before call
    app_updateAndRender* updateAndRender;
    
    FILETIME lastModifiedTime;
    HMODULE appDLL;
    
    b32 isValid;
} win32_AppCode;
#endif //WIN32_MAIN_H
