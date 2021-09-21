/* date = August 31st 2021 7:12 pm */

#ifndef PLATFORM_H
#define PLATFORM_H

#include "utils.h"

typedef struct FileReadResult {
    u32 contentSize;
    void* contents;
} FileReadResult;

#include "jed.h"

#if 0
FileReadResult
platformFileReadFull(char* fileName);

void
platformFileFreeMemory(void* bitmapMemory);

b32
platformFileWriteFull(char* fileName,
                      u32 memorySize,
                      void* memory);

i32
platformFileScanDelimited(FileReadResult file,
                          char delimiter,
                          i32 startOffset,
                          char* out);
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

#define PLATFORM_FILE_FREE_MEMORY(name) void name(FileReadResult file)
typedef PLATFORM_FILE_FREE_MEMORY(platformFileFreeMemory);

#define PLATFORM_FILE_READ_FULL(name) FileReadResult name(char* fileName)
typedef PLATFORM_FILE_READ_FULL(platformFileReadFull);

#define PLATFORM_FILE_WRITE_FULL(name) b32 name(char* fileName,\
u32 memorySize,\
void* memory)
typedef PLATFORM_FILE_WRITE_FULL(platformFileWriteFull);

#pragma clang diagnostic pop

typedef struct jed_Memory {
    void* transientStorage;
    void* persistentStorage;
    
    u64 transientStorageSize;
    u64 persistentStorageSize;
    
    b32 isInitialized;
    
    platformFileReadFull* fileReadFull;
    platformFileWriteFull* fileWriteFull;
    platformFileFreeMemory* fileFreeMemory;
} jed_Memory;

typedef struct jed_Keyboard{
    u32 filler;
} jed_Keyboard;

typedef struct jed_Input {
    i32 mouseX;
    i32 mouseY;
    i32 mouseZ;
    jed_Keyboard keyboard;
} jed_Input;

typedef struct jed_FileIO {
    platformFileReadFull* readFull;
    platformFileWriteFull* writeFull;
    platformFileFreeMemory* freeMemory;
} jed_FileIO;

#define UPDATE_AND_RENDER(name) void name(jed_Memory* memory,\
jed_Backbuffer* videoBackbuffer,\
jed_Input* input)
typedef UPDATE_AND_RENDER(app_updateAndRender);
#if 0
void
app_updateAndRender(jed_Memory* memory,
                    jed_Backbuffer* videoBackbuffer,
                    jed_Input* input);
#endif
#endif //PLATFORM_H
