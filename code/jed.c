#include "platform.h"
#define JED_RENDERER_IMPLEMENTATION 1
#include "renderer.h"
global jed_FileIO fileIO = {0};
#define JED_FILE_IMPLEMENTATION 1
#include "file.h"

global Str8 test;
global b32 pressed = true;
global memoryIndex testIndex;

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
        char fontFileFullName[255] = "C:/Windows/Fonts/arialbd.ttf";
        //char fontFileFullName[255] = "C:/Users/Nandu/Downloads/rbm/RobotoMono-Regular.ttf";
        editorState->fontGlyphSet = file_font_load(&editorState->arena,
                                                   fontFileFullName,
                                                   48.0f);
        editorState->row = (u32)editorState->fontGlyphSet.ascent;
        editorState->col = 0;
        test = STR8_LITERAL("Hello!, I am Jai. !@#$%^&*().................................................................");
    }
    
    
#if 1
    
    renderer_render_string(videoBackbuffer,
                           editorState->fontGlyphSet,
                           &editorState->row, &editorState->col,
                           test);
#endif
    
    editorState->row = (u32)editorState->fontGlyphSet.ascent;
    editorState->col = 0;
    i32 y = 0;
}
