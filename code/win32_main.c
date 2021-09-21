#include "pch.h"
#include "platform.h"
#include "jed.c"
#include "win32_main.h"
#include <stdio.h>

#define PERSISTENT_ALLOCATION_SIZE MB(64)
#define TRANSIENT_ALLOCATION_SIZE MB(500)

//~ GLOLBALS

global b32 running = false;
global f32 opacity = 0.95f;
global win32_Backbuffer win32_backbuffer = { .pixelStride = 4 };

//~ LOAD APP LIBRARY

#if 0
internal win32_AppCode
win32_loadAppLibrary() {
    win32_AppCode result = {
        .updateAndRender = (app_updateAndRender*)jed_updateAndRender
    };
    
    return result;
}
#endif

//~ WINDOWING

internal win32_WindowDimensions
win32_window_getDimensions(HWND window) {
    win32_WindowDimensions result;
    RECT clientRect;
    GetClientRect(window, &clientRect);
    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;
    
    return result;
}

internal void
win32_window_resizeDIBSection(u32 width, u32 height) {
    if (win32_backbuffer.memory) {
        VirtualFree(win32_backbuffer.memory,
                    0,
                    MEM_RELEASE);
    }
    
    win32_backbuffer.width = width;
    win32_backbuffer.height = height;
    
    // NOTE(Jai): When the 'bmiHeight' field is negative, this is a clue to
    // Windows to treat this bitmap as top-down, and not bottom-down - meaning that
    // the first 3 bytes of the image are the color for the opt left pixel
    // in the bitmap, not the bottom left!
    win32_backbuffer.info.bmiHeader.biSize = sizeof(win32_backbuffer.info.bmiHeader);
    win32_backbuffer.info.bmiHeader.biWidth = (LONG)win32_backbuffer.width;
    win32_backbuffer.info.bmiHeader.biHeight = -(LONG)win32_backbuffer.height;
    win32_backbuffer.info.bmiHeader.biPlanes = 1;
    win32_backbuffer.info.bmiHeader.biBitCount = 32; // we only need 24 bits (8 bits each for RGB
    // 8 extra bits for padding to align ourselves to 4 bytes
    win32_backbuffer.info.bmiHeader.biCompression = BI_RGB;
    
    memoryIndex bitmapMemorySize = win32_backbuffer.width * win32_backbuffer.height * win32_backbuffer.pixelStride;
    
    win32_backbuffer.memory = VirtualAlloc(0,
                                           bitmapMemorySize,
                                           MEM_RESERVE | MEM_COMMIT,
                                           PAGE_READWRITE);
    
    // TODO(Jai): Probably clear this to black
    win32_backbuffer.pitch = width * win32_backbuffer.pixelStride;
}

internal void
win32_videoBuffer_clear() {
    memset(win32_backbuffer.memory,
           (int)0xFF000000,
           (win32_backbuffer.width * win32_backbuffer.height * win32_backbuffer.pixelStride));
}

internal void
win32_window_update(HDC deviceContext,
                    int windowWidth, int windowHeight) {
#if 0
    PatBlt(deviceContext,
           0, 0,
           windowWidth, windowHeight,
           BLACKNESS);
#endif
    StretchDIBits(deviceContext,
                  0 ,0,
                  windowWidth, windowHeight,
                  0, 0,
                  (int)win32_backbuffer.width, (int)win32_backbuffer.height,
                  win32_backbuffer.memory,
                  &win32_backbuffer.info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
    win32_videoBuffer_clear();
}

//~ WINDOW CALLBACK
internal LRESULT CALLBACK
win32_mainWindowCallback(HWND window,
                         UINT message,
                         WPARAM wParam,
                         LPARAM lParam) {
    LRESULT result = 0;
    
    switch (message) {
        case WM_CLOSE: {
            running = false;
        } break;
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC deviceContext =  BeginPaint(window, &paint);
            win32_WindowDimensions windowDimensions = win32_window_getDimensions(window);
            win32_window_update(deviceContext,
                                windowDimensions.width, windowDimensions.height);
            EndPaint(window, &paint);
        } break;
        case WM_SIZE: {
            win32_WindowDimensions windowDimensions = win32_window_getDimensions(window);
            HDC deviceContext =  GetDC(window);
            win32_window_update(deviceContext,
                                windowDimensions.width, windowDimensions.height);
            win32_window_resizeDIBSection((u32)windowDimensions.width,
                                          (u32)windowDimensions.height);
        } break;
        
        default: {
            result = DefWindowProc(window, message, wParam, lParam);
        }
    }
    
    return result;
}

//~ MESSAGE PROCESSING
internal void
win32_processWindowMessages(void) {
    MSG message = {0};
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        switch (message.message) {
            case WM_QUIT: {
                running = false;
            } break;
            
            default: {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
    }
}

//~ FILE IO
internal FILETIME
win32_file_getModifiedTime(char* fileName) {
    FILETIME result = {0};
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(fileName,
                                       &findData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        result = findData.ftLastWriteTime;
        FindClose(findHandle);
    }
    return result;
}

internal inline void
win32_file_freeMemory(FileReadResult file) {
    if (file.contents) {
        VirtualFree(file.contents, 0, MEM_RELEASE);
        file.contentSize = 0;
    }
}

internal FileReadResult
win32_file_readFull(char* fileName) {
    FileReadResult result = {0};
    HANDLE fileHandle = CreateFileA(fileName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    (LPSECURITY_ATTRIBUTES)0,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize64;
        if (GetFileSizeEx(fileHandle, &fileSize64)) {
            u32 fileSize = truncate_safe_i64(fileSize64.QuadPart);
            result.contents = VirtualAlloc(0,
                                           fileSize,
                                           MEM_RESERVE | MEM_COMMIT,
                                           PAGE_READWRITE);
            if (result.contents) {
                DWORD bytesRead;
                if (ReadFile(fileHandle,
                             result.contents,
                             fileSize,
                             &bytesRead,
                             (LPOVERLAPPED)0)) {
                    result.contentSize = fileSize;
                } else {
                    // TODO(Jai): Logging
                }
            } else {
                // TODO(Jai): Logging
            }
        } else {
            // TODO(Jai): Logging
        }
        CloseHandle(fileHandle);
    } else {
        // TODO(Jai): Logging
    }
    
    return result;
}

internal b32
win32_file_writeFull(char* fileName,
                     u32 memorySize,
                     void* memory) {
    b32 result = false;
    HANDLE fileHandle = CreateFileA(fileName,
                                    GENERIC_WRITE,
                                    0, (LPSECURITY_ATTRIBUTES)0,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        if (WriteFile(fileHandle,
                      memory,
                      memorySize,
                      &bytesWritten,
                      0)) {
            result = (bytesWritten == memorySize);
        } else {
            // TODO(Jai): Logging
        }
        CloseHandle(fileHandle);
    } else {
        // TODO(Jai): Logging
    }
    
    return result;
}

#if 1
//~ MAIN
int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prevInstance,
        LPSTR cmdLine,
        int cmdShow) {
    win32_State platformState = {0};
    
    WNDCLASSA windowClass = {
        .lpfnWndProc =  win32_mainWindowCallback,
        .hInstance = instance,
        .lpszClassName = "JED_Window",
        .style = CS_HREDRAW | CS_VREDRAW
    };
    RegisterClassA(&windowClass);
    win32_window_resizeDIBSection(960, 960);
    HWND window = CreateWindowExA(WS_EX_LAYERED          ,           // Optional window styles.
                                  windowClass.lpszClassName,         // Window class
                                  "JED",                             // Window text
                                  // Window style
                                  WS_OVERLAPPEDWINDOW,
                                  // Size and position
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  960, 960,
                                  0,          // Parent window
                                  0,          // Menu
                                  instance,   // Instance handle
                                  0           // Additional application data
                                  );
    if (!window) {
        MessageBoxA(0,
                    "Window creation failed",
                    "Error",
                    MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }
    
    u8 windowAlpha = (u8)(opacity * 255.0f);
    SetLayeredWindowAttributes(window, RGB(0, 0, 0), windowAlpha, LWA_ALPHA);
    ShowWindow(window, cmdShow);
    
    
#if JAI_INTERNAL
    LPVOID baseAddress = (LPVOID)TB(6);
#else
    LPVOID baseAddress = (LPVOID)0;
#endif
    platformState.totalSize = PERSISTENT_ALLOCATION_SIZE + TRANSIENT_ALLOCATION_SIZE;
    platformState.memoryBlock = VirtualAlloc(baseAddress,
                                             (memoryIndex)platformState.totalSize,
                                             MEM_RESERVE | MEM_COMMIT,
                                             PAGE_READWRITE);
    jed_Memory appMemory = {
        .persistentStorage = platformState.memoryBlock,
        .persistentStorageSize = PERSISTENT_ALLOCATION_SIZE,
        .transientStorage = (u8*)platformState.memoryBlock + PERSISTENT_ALLOCATION_SIZE,
        .transientStorageSize = TRANSIENT_ALLOCATION_SIZE,
        .fileReadFull = (platformFileReadFull*)win32_file_readFull,
        .fileFreeMemory = (platformFileFreeMemory*)win32_file_freeMemory,
        .fileWriteFull = (platformFileWriteFull*)win32_file_writeFull
    };
    
    
    jed_Input appInput = {0};
    
    running =  true;
    while (running) {
        win32_processWindowMessages();
        jed_Backbuffer appBackbuffer = {
            .pixels = win32_backbuffer.memory,
            .width = win32_backbuffer.width,
            .height = win32_backbuffer.height,
            .pitch = win32_backbuffer.pitch,
            .pixelStride = win32_backbuffer.pixelStride
        };
        jed_updateAndRender(&appMemory,
                            &appBackbuffer,
                            &appInput);
        
        HDC deviceContext = GetDC(window);
        win32_WindowDimensions windowDimensions = win32_window_getDimensions(window);
        win32_window_update(deviceContext,
                            windowDimensions.width, windowDimensions.height);
        ReleaseDC(window, deviceContext);
    }
    
    return 0;
}
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wpointer-sign"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma clang diagnostic ignored "-Wdouble-promotion"
global char buffer[24<<20];
global unsigned char screen[20][79];

int main(int arg, char **argv)
{
    stbtt_fontinfo font;
    int i,j,ascent,baseline,ch=0;
    float scale, xpos=2; // leave a little padding in case the character extends left
    char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness
    
    fread(buffer, 1, 1000000, fopen("C:/Users/Nandu/Downloads/rbm/RobotoMono-Regular.ttf", "rb"));
    stbtt_InitFont(&font, buffer, 0);
    
    scale = stbtt_ScaleForPixelHeight(&font, 15);
    stbtt_GetFontVMetrics(&font, &ascent,0,0);
    baseline = (int) (ascent*scale);
    
    while (text[ch]) {
        int advance,lsb,x0,y0,x1,y1;
        float x_shift = xpos - (float) floor(xpos);
        stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
        stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
        stbtt_MakeCodepointBitmapSubpixel(&font,
                                          &screen[baseline + y0][(int) xpos + x0],
                                          x1 - x0, y1 - y0,
                                          79,
                                          scale, scale,
                                          x_shift, 0,
                                          text[ch]);
        // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's
        //wrong
        // because this API is really for baking character bitmaps into textures. if you want to
        //render
        // a sequence of characters, you really need to render each bitmap to a temp buffer, then
        // "alpha blend" that into the working buffer
        xpos += (advance * scale);
        if (text[ch+1])
            xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
        ++ch;
    }
    
    for (j=0; j < 20; ++j) {
        for (i=0; i < 78; ++i)
            putchar(" .:ioVM@"[screen[j][i]>>5]);
        putchar('\n');
    }
    
    return 0;
}
#pragma clang diagnostic pop
#endif
