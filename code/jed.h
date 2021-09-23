/* date = August 31st 2021 7:14 pm */

#ifndef JED_H
#define JED_H

#define JAI_UTILS_IMPLEMENTATION 1
#include "utils.h"
#define JAI_ALLOCATORS_IMPLEMENTATION 1
#include "allocators.h"
#include "jed_intrinsics.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define STB_TRUETYPE_IMPLEMENTATION 1
#include "stb_truetype.h"
#pragma clang diagnostic pop

#define BITMAP_BYTES_PER_PIXEL 4

typedef struct jed_Backbuffer {
    void* pixels;
    u32 width;
    u32 height;
	u32 aspectRatio;
    u32 pitch;
    u32 pixelStride;
} jed_Backbuffer;

typedef struct jed_Memory jed_Memory;
typedef struct jed_Input jed_Input;

typedef struct Bitmap {
    u32 width;
    u32 height;
    u32* pixels;
} Bitmap;

typedef struct FontGlyphSet {
    FileReadResult* ttfFile;
    stbtt_fontinfo fontInfo;
    i32 lineGap;
    i32 ascent;
    f32 scale;
    Bitmap* glyphs;
    i32* advanceWidth;
    i32* leftSideBearing;
    i32* xOffset;
    i32* yOffset;
} FontGlyphSet;

typedef struct EditorState {
    Arena arena;
    FontGlyphSet fontGlyphSet;
    u32 row;
    i32 col;
} EditorState;

void
jed_updateAndRender(jed_Memory* memory,
                    jed_Backbuffer* videoBackbuffer,
                    jed_Input* input);

#if 0
void freeMemory(void* memory);
FileReadResult readFull(char* fileName);
b32 writeFull(char* fileName, u32 memorySize, void* memory);
#endif


#endif //JED_H
