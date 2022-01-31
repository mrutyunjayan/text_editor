/* date = September 24th 2021 9:11 am */

#ifndef FILE_H
#define FILE_H

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
file_bmp_makeEmpty(Arena* arena,
                   u32 width, u32 height);

internal Bitmap
file_bmp_load(char* fileName);

internal FontGlyphSet
file_font_load(Arena* arena,
               char* fontFileFullName,
               f32 size);


#endif //FILE_H

#if defined(JED_FILE_IMPLEMENTATION)

internal Bitmap
file_bmp_makeEmpty(Arena* arena,
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


// TODO(Jai): Complete this loader
// NOTE(Jai): NOT A COMPLETE LOADER!
internal Bitmap
file_bmp_load(char* fileName) {
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
file_font_load(Arena* arena,
               char* fontFileFullName,
               f32 size) {
    FontGlyphSet result = {
        .glyphs = ARENA_PUSH_ARRAY(arena, 96, Bitmap),
        .advanceWidth =  ARENA_PUSH_ARRAY(arena, 96, i32),
        .leftSideBearing = ARENA_PUSH_ARRAY(arena, 96, i32),
        .xOffset = ARENA_PUSH_ARRAY(arena, 96, i32),
        .yOffset = ARENA_PUSH_ARRAY(arena, 96, i32),
    };
    
    FileReadResult ttfFile = fileIO.readFull(fontFileFullName);
    if (ttfFile.contentSize) {
        result.ttfFile = &ttfFile;
        u8 tempBuffer[100*100];
        stbtt_fontinfo fontInfo = {0};
        stbtt_InitFont(&fontInfo, ttfFile.contents, 0);
        result.fontInfo = fontInfo;
        result.scale = stbtt_ScaleForPixelHeight(&fontInfo, size);
        f32 scale = result.scale;
        i32 ascent, descent;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &result.lineGap);
        result.ascent = (i32)(round_floatToI32((f32)ascent * scale));
        result.lineGap = (i32)(round_floatToI32((f32)result.lineGap * scale));
        
        for (i32 codepoint = ' '; codepoint <= '~'; codepoint++) {
            i32 fontIndex = codepoint - 33;
            i32* advanceWidth = &result.advanceWidth[fontIndex];
            i32* leftSideBearing = &result.leftSideBearing[fontIndex];
            i32* xOffset = &result.xOffset[fontIndex];
            i32* yOffset = &result.yOffset[fontIndex];
            i32 width = 0;
            i32 height = 0;
            
            stbtt_GetCodepointHMetrics(&fontInfo,
                                       codepoint,
                                       advanceWidth, leftSideBearing);
            *advanceWidth = (i32)(round_floatToI32((f32)*advanceWidth * scale));
            *leftSideBearing = (i32)(round_floatToI32((f32)*leftSideBearing * scale));
            u8* monoBitmap = stbtt_GetCodepointBitmap(&fontInfo,
                                                      0, scale,
                                                      codepoint,
                                                      &width, &height,
                                                      xOffset, yOffset);
            
            // Convert from 1 channel  bitmap to 4 channel bitmap
            Bitmap bitmap = file_bmp_makeEmpty(arena, (u32)width, (u32)height);
            for (i32 i = 0; i < (width * height); i++) {
                u8 alpha = monoBitmap[i];
                bitmap.pixels[i] = (u32)((alpha << 24)
                                         | (alpha << 16)
                                         | (alpha << 8)
                                         | (alpha << 0));
            }
            
            result.glyphs[fontIndex] = bitmap;
            stbtt_FreeBitmap(monoBitmap, 0);
        }
    }
    
    return result;
}

#undef JED_FILE_IMPLEMENTATION
#endif
