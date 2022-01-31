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

#if 0
typedef struct jed_Input {
    union {
        b32 keyPressed[128];
        struct {
            b32 key_0;
            b32 key_1;
            b32 key_2;
            b32 key_3;
            b32 key_4;
            b32 key_5;
            b32 key_6;
            b32 key_7;
            b32 key_8;
            b32 key_9;
            b32 key_a;
            b32 key_b;
            b32 key_c;
            b32 key_d;
            b32 key_e;
            b32 key_f;
            b32 key_g;
            b32 key_h;
            b32 key_i;
            b32 key_j;
            b32 key_k;
            b32 key_l;
            b32 key_m;
            b32 key_n;
            b32 key_o;
            b32 key_p;
            b32 key_q;
            b32 key_r;
            b32 key_s;
            b32 key_t;
            b32 key_u;
            b32 key_v;
            b32 key_w;
            b32 key_x;
            b32 key_y;
            b32 key_z;
            b32 key_A;
            b32 key_B;
            b32 key_C;
            b32 key_D;
            b32 key_E;
            b32 key_F;
            b32 key_G;
            b32 key_H;
            b32 key_I;
            b32 key_J;
            b32 key_K;
            b32 key_L;
            b32 key_M;
            b32 key_N;
            b32 key_O;
            b32 key_P;
            b32 key_Q;
            b32 key_R;
            b32 key_S;
            b32 key_T;
            b32 key_U;
            b32 key_V;
            b32 key_W;
            b32 key_X;
            b32 key_Y;
            b32 key_Z;
            b32 key_exclamation;
            b32 key_doubelQuote;
            b32 key_hash;
            b32 key_dollar;
            b32 key_percentage;
            b32 key_ampersand;
            b32 key_singleQuote;
            b32 key_openBracket;
            b32 key_closeBraket;
            b32 key_star;
            b32 key_plus;
            b32 key_comma;
            b32 key_minus;
            b32 key_period;
            b32 key_forwardSlash;
            b32 key_colon;
            b32 key_semiColon;
            b32 key_lesserThan;
            b32 key_equals;
            b32 key_greaterThan;
            b32 key_questionMark;
            b32 key_at;
            b32 key_openSquareBracket;
            b32 key_backwardSlash;
            b32 key_closeSquareBracket;
            b32 key_carrotTop;
            b32 key_underScore;
            b32 key_backTick;
            b32 key_openCurlyBracket;
            b32 key_pipe;
            b32 key_closeCurlyBracket;
            b32 key_tilde;
            b32 key_space;
            b32 key_capsLock;
            b32 key_leftShift;
            b32 key_rightShift;
            b32 key_leftCtrl;
            b32 key_rightCtrl;
            b32 key_leftAlt;
            b32 key_rightAlt;
            b32 key_tab;
            b32 key_return;
            b32 key_esc;
            b32 key_del;
            b32 key_insert;
            b32 key_delete;
            b32 key_home;
            b32 key_end;
            b32 key_pageUp;
            b32 key_pageDown;
            b32 key_arrowUp;
            b32 key_arrowDown;
            b32 key_arrowLeft;
            b32 key_arrowRight;
            b32 key_f1;
            b32 key_f2;
            b32 key_f3;
            b32 key_f4;
            b32 key_f5;
            b32 key_f6;
            b32 key_f7;
            b32 key_f8;
            b32 key_f9;
            b32 key_f10;
            b32 key_f11;
            b32 key_f12;
        };
    };
} jed_Input;
#else
typedef struct jed_Input {
    char pressedChar;
} jed_Input;

#endif




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
