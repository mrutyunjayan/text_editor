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

//~ LOAD APP DLL

internal void
win32_buildExePathFileName(win32_State* state,
                           char* fileName,
                           i32 destCount,
                           char* dest) {
    catStrings(state->onePastLastSlash - state->exeFileName,
               state->exeFileName,
               stringLength(fileName), fileName,
               destCount, dest);
}

internal void
win32_getExeFileName(win32_State* state) {
    DWORD fileNameSize = GetModuleFileNameA(0,
                                            state->exeFileName,
                                            sizeof(state->exeFileName));
    state->onePastLastSlash = state->exeFileName;
    for (char* scan = state->exeFileName; *scan; ++scan) {
        if (*scan == '\\') {
            state->onePastLastSlash = scan + 1;
        }
    }
    
}

internal FILETIME
win32_getLastWriteTime(char* fileName) {
    FILETIME result = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &data)) {
        result = data.ftLastWriteTime;
    }
    
    return result;
}

internal win32_AppCode
win32_loadAppDLL(char* sourceDLLName,
                 char* tempDLLName,
                 char* lockFileName) {
    win32_AppCode result = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (!GetFileAttributesEx(lockFileName, GetFileExInfoStandard, &ignored)) {
        result.lastModifiedTime = win32_getLastWriteTime(sourceDLLName);
        CopyFile(sourceDLLName, tempDLLName, FALSE);
        result.appDLL = LoadLibraryA(tempDLLName);
        
        if (result.appDLL) {
            result.updateAndRender 
                = (app_updateAndRender*)GetProcAddress(result.appDLL, "jed_updateAndRender");
            result.isValid = true;
        }
    }
    
    return result;
}

internal void
win32_unloadAppCode(win32_AppCode* appCode) {
    if (appCode->appDLL) {
        FreeLibrary(appCode->appDLL);
        appCode->appDLL = 0;
    }
    appCode->isValid = false;
    appCode->updateAndRender = 0;
}

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

//~ MAIN
int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prevInstance,
        LPSTR cmdLine,
        int cmdShow) {
    win32_State platformState = {0};
    
    win32_getExeFileName(&platformState);
    char appDLLFullPath[MAX_PATH];
    char tempDLLFullPath[MAX_PATH];
    char lockFullPath[MAX_PATH];
    win32_buildExePathFileName(&platformState,
                               "jed.dll",
                               sizeof(appDLLFullPath), appDLLFullPath);
    win32_buildExePathFileName(&platformState,
                               "jed_temp.dll",
                               sizeof(tempDLLFullPath), tempDLLFullPath);
    win32_buildExePathFileName(&platformState,
                               "lock.tmp",
                               sizeof(lockFullPath), lockFullPath);
    
    
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
    
    if (!appMemory.persistentStorage || !appMemory.transientStorage) {
        // TODO(Jai): Logging
    }
    
    win32_AppCode app = win32_loadAppDLL(appDLLFullPath,
                                         tempDLLFullPath,
                                         lockFullPath);
    u32 loadCounter = 0;
    
    jed_Input appInput = {0};
    
    running =  true;
    while (running) {
        FILETIME newDLLWriteTime = win32_getLastWriteTime(appDLLFullPath);
        if (CompareFileTime(&newDLLWriteTime,
                            &app.lastModifiedTime) != 0) {
            win32_unloadAppCode(&app);
            app = win32_loadAppDLL(appDLLFullPath,
                                   tempDLLFullPath,
                                   lockFullPath);
            loadCounter = 0;
        }
        
        win32_processWindowMessages();
        jed_Backbuffer appBackbuffer = {
            .pixels = win32_backbuffer.memory,
            .width = win32_backbuffer.width,
            .height = win32_backbuffer.height,
            .pitch = win32_backbuffer.pitch,
            .pixelStride = win32_backbuffer.pixelStride
        };
        if (app.updateAndRender) {
            app.updateAndRender(&appMemory,
                                &appBackbuffer,
                                &appInput);
        }
        
        HDC deviceContext = GetDC(window);
        win32_WindowDimensions windowDimensions = win32_window_getDimensions(window);
        win32_window_update(deviceContext,
                            windowDimensions.width, windowDimensions.height);
        ReleaseDC(window, deviceContext);
    }
    
    return 0;
}
