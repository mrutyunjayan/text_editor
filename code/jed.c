#include "jed.h"

global jed_FileIO fileIO = {0};

internal void
paint_pixel(jed_Backbuffer* videoBackbuffer,
            u32 x, u32 y,
            u32 colour) {
    u32* pixel = (u32*)((u8*)videoBackbuffer->pixels
                        + x * videoBackbuffer->pixelStride
                        + y * videoBackbuffer->pitch);
    *pixel = colour;
}

internal void
draw_rectangle(jed_Backbuffer* videoBackbuffer,
               u32 x1, u32 y1,
               u32 x2, u32 y2,
               u32 colour) {
    if (x1 > x2) SWAP(x1, x2);
    if (y1 > y2) SWAP(y1, y2);
    x1 = CLAMP_BOT(x1, 0);
    y1 = CLAMP_BOT(y1, 0);
    x2 = CLAMP_TOP(x2, videoBackbuffer->width);
    y2 = CLAMP_TOP(y2, videoBackbuffer->height);
    
    for (u32 y =  y1; y < y2; y++) {
        for (u32 x = x1; x < x2; x++) {
            paint_pixel(videoBackbuffer,
                        x, y,
                        colour);
        }
    }
}

global char* alphabets[26][16] = {
    { "0001100000111100011001100110011001111110011001100110011001100110" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "0011110001111110010000100100000001000000010000100111111000111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" },
    { "1111110001111110011001100110011001111100011001100110011011111100" }
};

#pragma pack(push, 1)
typedef struct BitmapHeader {
	u16 fileType;
	u32 fileSize;
	u16 reserved1;
	u16 reserved2;
	u32 bitmapOffset;
	u32 size;
	i32 width;
	i32 height;
	u16 planes;
	u16 bitsPerPixel;
	u32 compression;
	u32 sizeOfBitmap;
	i32 horzResolution;
	i32 vertResolution;
	u32 coloursUsed;
	u32 coloursImportant;
    
	u32 redMask;
	u32 greenMask;
	u32 blueMask;
} BitmapHeader;
#pragma pack(pop)

internal Bitmap
bmp_makeEmpty(Arena* arena,
              u32 width, u32 height) {
    memoryIndex totalBitmapSize = (memoryIndex)(width * height * BITMAP_BYTES_PER_PIXEL);
    Bitmap result = {
        .width = width,
        .height = height,
        .pixels = alloc_arena_alloc(arena, totalBitmapSize)
    };
    alloc_zeroSize(totalBitmapSize, (void*)result.pixels);
    
    return result;
}

#if 0
internal void
draw_character(jed_Backbuffer* videoBackbuffer,
               u32 x, u32 y,
               u32 colour,
               i32 size,
               char* character) {
    u32 xIndex = 0;
    u32 yIndex = 0;
    i32 xMoved = 0;
    i32 yMoved = 0;
    for (i32 j = (8 * size); j > 0; j -= 1) {
        xIndex = 0;
        xMoved = 0;
        for (i32 i = 0; i < (8 * size); i += 1) {
            if (*character == '1') {
                paint_pixel(videoBackbuffer,
                            x + xIndex, y + yIndex,
                            colour);
            }
            xMoved++;
            xIndex++;
            if (xMoved == size) {
                xMoved = 0;
                character++;
            }
        }
        yMoved++;
        yIndex++;
        if (yMoved == size) {
            yMoved = 0;
            continue;
        }
        character -= 8;
    }
}
#else
internal void
draw_character(jed_Backbuffer* videoBackbuffer,
               u32 x, u32 y,
               Bitmap* characterBitmap) {
    u32* pixel = characterBitmap->pixels;
    for (u32 column = 0; column < characterBitmap->height; column++) {
        for (u32 row = 0; row < characterBitmap->width; row++) {
            if (*pixel++ != 0) {
                paint_pixel(videoBackbuffer,
                            x + row, y + column,
                            0xFFFFFFFF);
            }
        }
    }
}
#endif

// TODO(Jai): Complete this loader
// NOTE(Jai): NOT A COMPLETE LOADER!
internal Bitmap
bmp_load(char* fileName) {
    Bitmap result = {0};
    FileReadResult file = fileIO.readFull(fileName);
    if (file.contentSize) {
        BitmapHeader* header = (BitmapHeader*)file.contents;
        result.pixels = (u32*)((u8*)file.contents + header->bitmapOffset);
        result.width = (u32)header->width;
		result.height = (u32)header->height;
        
        ASSERT(header->compression == 3);
        // NOTE(Jai): Byte order in memory is determined by the
        // header itself so we have to read out the masks and
        // convert it ourselves, it is also stored bottom up
        // NOTE(Jai): If you are using this generically, please remember
        // that BMP files can go in either direction and the height
        // will be negative for top-down.
        // Also there can be compression, etc., etc.
        // NOTE(Jai): NOT a complete BMP loader.
        u32 redMask = header->redMask;
        u32 greenMask = header->greenMask;
        u32 blueMask = header->blueMask;
        u32 alphaMask = ~(redMask | greenMask | blueMask);
        
        BitscanResult redShift = findLeastSignificantSetBit(redMask);
        BitscanResult greenShift = findLeastSignificantSetBit(greenMask);
        BitscanResult blueShift = findLeastSignificantSetBit(blueMask);
        BitscanResult alphaShift = findLeastSignificantSetBit(alphaMask);
        
        ASSERT(redShift.found);
        ASSERT(greenShift.found);
        ASSERT(blueShift.found);
        ASSERT(alphaShift.found);
        
        // NOTE(Jai): Rearrange the colour encoding to our expected format
        u32* sourceDest = result.pixels;
        for (i32 y = 0; y < header->height; y++) {
            for (i32 x = 0; x < header->width; x++) {
                u32 pixelColour = *sourceDest;
                *sourceDest++ = ((((pixelColour >> alphaShift.index) & 0xFF) << 24)
                                 | (((pixelColour >> redShift.index) & 0xFF) << 16)
                                 | (((pixelColour >> greenShift.index) & 0xFF) << 8)
                                 | (((pixelColour >> blueShift.index) & 0xFF) << 0));
            }
        }
    }
    
    return result;
}

internal FontGlyphSet
font_load(Arena* arena,
          char* fontFileFullName,
          f32 size) {
    FontGlyphSet result = {
        .glyphs = ARENA_PUSH_ARRAY(arena, 95, Bitmap),
        .advanceWidth =  ARENA_PUSH_ARRAY(arena, 95, i32),
        .leftSideBearing = ARENA_PUSH_ARRAY(arena, 95, i32),
        .xOffset = ARENA_PUSH_ARRAY(arena, 95, i32),
        .yOffset = ARENA_PUSH_ARRAY(arena, 95, i32),
    };
    
    FileReadResult ttfFile = fileIO.readFull(fontFileFullName);
    if (ttfFile.contentSize) {
        u8 tempBuffer[100*100];
        stbtt_fontinfo* fontInfo = &result.fontInfo;
        stbtt_InitFont(fontInfo, ttfFile.contents, 0);
        result.scale = stbtt_ScaleForPixelHeight(fontInfo, size);
        f32 scale = result.scale;
        stbtt_GetFontVMetrics(fontInfo, &result.ascent, &result.descent, &result.lineGap);
        result.ascent = (i32)(round_floatToI32((f32)result.ascent * scale));
        result.descent = (i32)(round_floatToI32((f32)result.descent * scale));
        result.lineGap = (i32)(round_floatToI32((f32)result.lineGap * scale));
        i32 baseline = round_floatToI32((f32)result.ascent * result.scale);
        
        for (i32 codepoint = '!'; codepoint <= '~'; codepoint++) {
            i32 fontIndex = codepoint - 33;
            i32* advanceWidth = &result.advanceWidth[fontIndex];
            i32* leftSideBearing = &result.leftSideBearing[fontIndex];
            i32* xOffset = &result.xOffset[fontIndex];
            i32* yOffset = &result.yOffset[fontIndex];
            i32 width = 0;
            i32 height = 0;
            
            stbtt_GetCodepointHMetrics(fontInfo,
                                       codepoint,
                                       advanceWidth, leftSideBearing);
            *advanceWidth = (i32)(round_floatToI32((f32)*advanceWidth * scale));
            *leftSideBearing = (i32)(round_floatToI32((f32)*leftSideBearing * scale));
            u8* monoBitmap = stbtt_GetCodepointBitmap(fontInfo,
                                                      0, scale,
                                                      codepoint,
                                                      &width, &height,
                                                      xOffset, yOffset);
            
            // Convert from 1 channel  bitmap to 4 channel bitmap
            Bitmap bitmap = bmp_makeEmpty(arena, (u32)width, (u32)height);
            for (i32 i = 0; i < (width * height); i++) {
                u8 alpha = monoBitmap[i];
                bitmap.pixels[i] = (u32)((alpha << 24)
                                         | (alpha << 16)
                                         | (alpha << 8)
                                         | (alpha << 0));
            }
            
            result.glyphs[fontIndex] = bitmap;
        }
    }
    
    return result;
}

global u32 global_baseline =  300;
global u32 characterCurrentPoint = 50;

internal void
displayCodepointGlyph(jed_Backbuffer* videoBackbuffer,
                      FontGlyphSet fontGlyphSet,
                      u8 codepoint){
    i32 fontIndex = codepoint - 33;
    i32 advanceWidth = fontGlyphSet.advanceWidth[fontIndex];
    i32 leftSideBearing = fontGlyphSet.leftSideBearing[fontIndex];
    i32 xOffset = fontGlyphSet.xOffset[fontIndex];
    i32 yOffset = fontGlyphSet.yOffset[fontIndex];
    i32 ascent = fontGlyphSet.ascent;
    u32 width = fontGlyphSet.glyphs[fontIndex].width;
    u32 height = fontGlyphSet.glyphs[fontIndex].height;
    
    u32* source = fontGlyphSet.glyphs[fontIndex].pixels;
    u8* destRow = (u8*)((u32*)videoBackbuffer->pixels
                        + characterCurrentPoint
                        + (global_baseline + (u32)yOffset) * videoBackbuffer->width);
    for (u32 y = 0; y < height; y++) {
        u32* dest = (u32*)destRow;
        for (u32 x = 0; x < width; x++) {
            *dest++ = *source++;
        }
        destRow += videoBackbuffer->pitch;
    }
    
    characterCurrentPoint += (u32)advanceWidth;
}

#if 0
void
jed_updateAndRender(jed_Memory* memory,
                    jed_Backbuffer* videoBackbuffer,
                    jed_Input* input);
#endif
UPDATE_AND_RENDER(jed_updateAndRender) {
    EditorState* editorState = (EditorState*)memory->persistentStorage;
    if (!memory->isInitialized) {
        memory->isInitialized = true;
        fileIO.readFull = memory->fileReadFull;
        fileIO.writeFull = memory->fileWriteFull;
        fileIO.freeMemory = memory->fileFreeMemory;
        
        alloc_arena_initialize(&editorState->arena,
                               (u8*)memory->persistentStorage + sizeof(EditorState),
                               memory->persistentStorageSize - sizeof(EditorState));
        //char fontFileFullName[255] = "C:/Windows/Fonts/arial.ttf";
        char fontFileFullName[255] = "C:/Users/Nandu/Downloads/rbm/RobotoMono-Regular.ttf";
        editorState->fontGlyphSet = font_load(&editorState->arena,
                                              fontFileFullName,
                                              48.0f);
    }
    
#if 0
    // NOTE(Jai): Draw Baseline for debugging
    u8* destRow = (u8*)((u32*)videoBackbuffer->pixels
                        + ((u32)global_baseline * videoBackbuffer->width));
    u32* dest = (u32*)destRow;
    for (u32 x = 0; x < videoBackbuffer->width; x++) {
        *dest++ = 0xFFFFFF00;
    }
#endif
    for (u8 codepoint = 'a'; codepoint < 'z'; codepoint++) {
        displayCodepointGlyph(videoBackbuffer,
                              editorState->fontGlyphSet,
                              codepoint);
    }
    
    global_baseline += 50;
    characterCurrentPoint = 50;
    for (u8 codepoint = 'A'; codepoint < 'Z'; codepoint++) {
        displayCodepointGlyph(videoBackbuffer,
                              editorState->fontGlyphSet,
                              codepoint);
    }
    
    global_baseline += 50;
    characterCurrentPoint = 50;
    for (u8 codepoint = '!'; codepoint < 'A'; codepoint++) {
        displayCodepointGlyph(videoBackbuffer,
                              editorState->fontGlyphSet,
                              codepoint);
    }
    
    global_baseline = 300;
    characterCurrentPoint = 50;
    int y = 0;
}
