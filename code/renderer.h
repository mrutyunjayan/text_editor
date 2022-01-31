/* date = September 24th 2021 8:48 am */

#ifndef RENDERER_H
#define RENDERER_H

internal void
renderer_paintPixel(jed_Backbuffer* videoBackbuffer,
                    u32 x, u32 y,
                    u32 colour);

internal void
renderer_drawRectangle(jed_Backbuffer* videoBackbuffer,
                       u32 x1, u32 y1,
                       u32 x2, u32 y2,
                       u32 colour);

internal void
renderer_displayCodepointGlyph(jed_Backbuffer* videoBackbuffer,
                               FontGlyphSet fontGlyphSet,
                               u32* row, i32* col,
                               char codepoint);

internal void
renderer_render_string(jed_Backbuffer* videoBackbuffer,
                       FontGlyphSet fontGlyphSet,
                       u32* row, i32* col,
                       Str8 string);

#endif //RENDERER_H

#if defined(JED_RENDERER_IMPLEMENTATION)

internal void
renderer_paintPixel(jed_Backbuffer* videoBackbuffer,
                    u32 x, u32 y,
                    u32 colour) {
    u32* pixel = (u32*)((u8*)videoBackbuffer->pixels
                        + x * videoBackbuffer->pixelStride
                        + y * videoBackbuffer->pitch);
    *pixel = colour;
}

internal void
renderer_drawRectangle(jed_Backbuffer* videoBackbuffer,
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
            renderer_paintPixel(videoBackbuffer,
                                x, y,
                                colour);
        }
    }
}

internal void
renderer_displayCodepointGlyph(jed_Backbuffer* videoBackbuffer,
                               FontGlyphSet fontGlyphSet,
                               u32* row, i32* col,
                               char codepoint) {
    i32 advanceWidth = 0;
    if (codepoint == ' ') {
        advanceWidth = fontGlyphSet.advanceWidth[0];
    } else {
        i32 fontIndex = codepoint - 33;
        advanceWidth = fontGlyphSet.advanceWidth[fontIndex];
        i32 leftSideBearing = fontGlyphSet.leftSideBearing[fontIndex];
        i32 xOffset = fontGlyphSet.xOffset[fontIndex];
        i32 yOffset = fontGlyphSet.yOffset[fontIndex];
        u32 width = fontGlyphSet.glyphs[fontIndex].width;
        u32 height = fontGlyphSet.glyphs[fontIndex].height;
        
        u32* source = fontGlyphSet.glyphs[fontIndex].pixels;
        u8* destRow = (u8*)((u32*)videoBackbuffer->pixels
                            + *col
                            + (*row + (u32)yOffset) * videoBackbuffer->width);
        for (u32 y = 0; y < height; y++) {
            u32* dest = (u32*)destRow;
            for (u32 x = 0; x < width; x++) {
                *dest++ = *source++;
            }
            destRow += videoBackbuffer->pitch;
        }
    }
    
    if ((u32)(*col + (i32)fontGlyphSet.glyphs[0].width * 2) > videoBackbuffer->width) {
        *row += (u32)(fontGlyphSet.lineGap + fontGlyphSet.ascent);
        *col = 0;
    } else {
        *col += advanceWidth;
    }
}

// TODO(Jai): Alpha blend the character when advancing by kerning
internal void
renderer_render_string(jed_Backbuffer* videoBackbuffer,
                       FontGlyphSet fontGlyphSet,
                       u32* row, i32* col,
                       Str8 string) {
    stbtt_fontinfo* fontInfo = &fontGlyphSet.fontInfo;
    for (i32 i = 0; i < (i32)string.length - 1; i++) {
        char ch1 = string.str[i];
        char ch2 = string.str[i + 1];
        renderer_displayCodepointGlyph(videoBackbuffer,
                                       fontGlyphSet,
                                       row, col,
                                       ch1);
        i32 kerning = stbtt_GetCodepointKernAdvance(fontInfo, ch1, ch2);
        kerning += round_floatToI32(fontGlyphSet.scale * (f32)kerning);
        *col += kerning;
    }
    renderer_displayCodepointGlyph(videoBackbuffer,
                                   fontGlyphSet,
                                   row, col,
                                   string.str[string.length - 1]);
}

#undef JED_RENDERER_IMPLEMENTATION
#endif // IMPLEMENTATION
