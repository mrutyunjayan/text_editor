#ifndef SDL2_MAIN_H
#define SDL2_MAIN_H

#include "utils.h"
#include "allocators.h"

//~ BACKBUFFER
typedef struct sdl2_Backbuffer {
    // NOTE(Jai): Pixels are always 32 bits wide, Memory Order: BB GG RR xx
    u32 width;
    u32 height;
    u32 pitch;
    u32 pixelStride;
    void* memory;
} sdl2_Backbuffer;

typedef struct Clock {
    f64 frequency;
    f64 startTime; // in seconds
    f64 elapsed; // in seconds
} Clock;

//~ PLATFORM STATE
typedef struct sdl2_State {
    void* memoryBlock;
    u64 totalSize;
    
    Clock clock;
    f64 lastTime;
} sdl2_State;

typedef struct sdl2_WindowDimensions {
    i32 width, height;
} sdl2_WindowDimensions;

#endif // SDL2_MAIN_H
