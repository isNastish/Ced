

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <gl/gl.h>

// external
#define STB_TRUETYPE_IMPLEMENTATION 
#include "external/stb_truetype.h"

#define global static
#define internal static
#define local_persist static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 bool32;
typedef bool32 b32;

typedef float real32;
typedef double real64;

typedef real32 f32;
typedef real64 f64;

typedef struct W32_WindowDim{
    s32 width;
    s32 height;
}W32_WindowDim;

typedef struct W32_Bitmap{
    BITMAPINFO info;
    void *mem;
    u32 width;
    u32 height;
    u32 pitch;
}W32_Bitmap;

global s8 global_running;
global HWND global_window_handle;
global HINSTANCE global_instance_handle;
global HDC global_device_context;
global HGLRC global_opengl_rendering_context;
global W32_Bitmap global_offscreen_bitmap;

// OpenGL
#include "w32_opengl.c"

internal char *read_entire_file_into_mem(char *file_name){
    char *result = 0;

    FILE *file = fopen(file_name, "rb");
    if(file){
        size_t file_size = 0;
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        result = (char *)malloc(file_size + 1);
        fread(result, file_size, 1, file);
        result[file_size] = 0;

        fclose(file);
    }
    
    return(result);
}

internal void stbtt_test(W32_Bitmap *bitmap){
    char *ttf_buffer = read_entire_file_into_mem("c:/windows/fonts/arial.ttf");
    
    stbtt_fontinfo font;

    stbtt_InitFont(&font, (u8 *)ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));

    

#if 0
    // NOTE: top-to-bottom bitmap. So we have to do a flip.
    u8 *mono_bitmap = (u8 *)stbtt_GetCodepointBitmap(&font,
                                                     0,
                                                     stbtt_ScaleForPixelHeight(&font, 256.0f),
                                                     'a', // glyph to be rendered.
                                                     &width,
                                                     &height,
                                                     &x_offset,
                                                     &y_offset);
#endif
    
    // Kavabanga

    enum {N=13};
    
    u8 *kavabanga_glyphs[N];
    s32 kavabanga_spelling[] = {'k', 'a', 'v', 'a', 'b', 'a', 'n', 'g', 'a', '1', '2', '3', 555};
    s32 width, height, x_offset, y_offset;
    u8 *starting_point = (u8 *)bitmap->mem;
    
    
    
    for(s32 x = 0;
        x < N;
        ++x){
        kavabanga_glyphs[x] =
            (u8 *)stbtt_GetCodepointBitmap(&font,
                                           0,
                                           stbtt_ScaleForPixelHeight(&font, 128.0f),
                                           kavabanga_spelling[x],
                                           &width,
                                           &height,
                                           &x_offset,
                                           &y_offset);
        
        u8 *source = kavabanga_glyphs[x];
        u8 *dest_row = (u8 *)starting_point;
        for(u32 y = 0;
            y < height;
            ++y){
            u32 *dest = (u32 *)dest_row;
            for(u32 x = 0;
                x < width;
                ++x){
                u8 alpha = *source++;
                if(alpha){
                    *dest = ((alpha << 24) |
                             (alpha << 16) |
                             (alpha << 8) |
                             (alpha << 0));
                }
                dest++;
            }
            dest_row += bitmap->pitch;
        }

        (u32 *)starting_point += width;
        
        stbtt_FreeBitmap(kavabanga_glyphs[x], 0);
    }

#if 0
    u8 *source = mono_bitmap;
    u8 *dest_row = (u8 *)bitmap->mem + (bitmap->pitch * 50) + 100;
    for(u32 y = 0;
        y < height;
        ++y){
        u32 *dest = (u32 *)dest_row;
        for(u32 x = 0;
            x < width;
            ++x){
            u8 alpha = *source++;
            if(alpha){
                *dest = ((alpha << 24) |
                         (alpha << 16) |
                         (alpha << 8) |
                         (alpha << 0));
            }
            dest++;
        }
        dest_row += bitmap->pitch;
    }
    stbtt_FreeBitmap(mono_bitmap, 0);
#endif
}

internal void w32_render(W32_Bitmap *bitmap){
    u32 dest_width = bitmap->width;
    u32 dest_height = bitmap->height;
    u32 *dest = (u32 *)bitmap->mem;
    for(u32 y = 0;
        y < dest_height;
        y++){
        for(u32 x = 0;
            x < dest_width;
            x++){
            *dest++ = (64 | (64 << 8) | (64 << 16));
        }
    }
}

internal W32_WindowDim w32_get_window_dimension(HWND window){
    W32_WindowDim dimension = {0};
    
    RECT client_rect = {0};
    GetClientRect(window, &client_rect);
    dimension.width = client_rect.right - client_rect.left;
    dimension.height = client_rect.bottom - client_rect.top;
    
    return(dimension);
}

internal w32_resize_dib_section(W32_Bitmap *bitmap, u32 width, u32 height){
    // STUDY: Do I really need it?
#if 0
    if(bitmap->mem){
        VirtualFree(bitmap->mem, 0, MEM_RELEASE);
    }
#endif
    
    bitmap->width = width;
    bitmap->height = height;
    
    bitmap->info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap->info.bmiHeader.biWidth = bitmap->width;
    bitmap->info.bmiHeader.biHeight = -bitmap->height; // NOTE: top-down dib (origin in the upper-left corner).
    bitmap->info.bmiHeader.biPlanes = 1;
    bitmap->info.bmiHeader.biBitCount = 32;
    bitmap->info.bmiHeader.biCompression = BI_RGB; // NOTE: an uncompressed format.

    u32 bpp = 4; // NOTE: bytes per pixel.
    u32 bitmap_size = (bitmap->width * bitmap->height) * bpp;

    bitmap->mem = (void *)VirtualAlloc(0,
                                       bitmap_size,
                                       MEM_COMMIT | MEM_RESERVE,
                                       PAGE_READWRITE);

    bitmap->pitch = bitmap->width * bpp;
    
    if(!bitmap->mem){
        // TODO: error handling.
    }
    else{
        // Success.
        ;
        
    }
}

internal void w32_update_window(HDC device_context, u32 window_width, u32 window_height){
    s32 stretch_result = StretchDIBits(device_context,
                                       0, 0,
                                       window_width,
                                       window_height,
                                       0, 0,
                                       global_offscreen_bitmap.width,
                                       global_offscreen_bitmap.height,
                                       global_offscreen_bitmap.mem,
                                       &global_offscreen_bitmap.info,
                                       DIB_RGB_COLORS,
                                       SRCCOPY);
    if(!stretch_result){
        // TODO: error handling.
    }
    else{
        // Success.
        OutputDebugStringA("Success!");
    }
}

LRESULT w32_window_proc(HWND window,
                        UINT msg,
                        WPARAM w_param,
                        LPARAM l_param){
    LRESULT result = 0;
    
    switch(msg){
        case(WM_CLOSE):
        case(WM_DESTROY ):
        case(WM_QUIT):{
            global_running = 0;
        }break;
        case(WM_CREATE):{ 
            w32_init_opengl(window);
        }break;
        case(WM_SIZE):{
#if 0
            W32_WindowDim dim = w32_get_window_dimension(window);
            
            glViewport(0, 0, dim.width, dim.height);
            glClearColor(0, 0xff, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(global_device_context);
#endif
        }break;
        default:{
            result = DefWindowProc(window, msg, w_param, l_param);
        }break;
    }
    
    return(result);
}


INT WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE prev_instance,
                   PSTR cmd_line,
                   int show_code){

    //stbtt_test(0);
    
    global_instance_handle = instance;
    
    WNDCLASSA window_class = {0};
    
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        window_class.lpfnWndProc = w32_window_proc;
        window_class.hInstance = global_instance_handle;
        //window_class.hIcon = ;
        //window_class.hCursor = ;
        window_class.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
        window_class.lpszClassName = "AppWindowClass";
    }
    
    
    if(!RegisterClassA(&window_class)){
        // TODO: error handling.
        goto quit;
    }
    
    HWND window_handle = CreateWindowEx(0,
                                        window_class.lpszClassName, 
                                        "Ced",
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        0, 0,
                                        global_instance_handle,
                                        0);
    
    if(!window_handle){
        // TODO: error handling.
        goto quit;
        
    }

    W32_WindowDim dimension = w32_get_window_dimension(window_handle);
    
    w32_resize_dib_section(&global_offscreen_bitmap,
                           dimension.width,
                           dimension.height);
    w32_render(&global_offscreen_bitmap);

    stbtt_test(&global_offscreen_bitmap);
    
    global_running = 1;
    while(global_running){
        
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessageA(&message);
            
        }

        HDC device_context = GetDC(window_handle);

        w32_update_window(device_context,
                          dimension.width,
                          dimension.height);
        
        {
            
    
            //StretchDIBits();
#if 0
            W32_WindowDim dim = w32_get_window_dimension(window_handle);
            glViewport(0, 0, dim.width, dim.height); // NOTE: specifies the boundaries of what you're trying to render.
            glClearColor((f32)192/(f32)255, 0xff, (f32)62/(f32)255, 0); // NOTE: normalized automatically.
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(global_device_context);
#endif
        }
        ReleaseDC(window_handle, device_context);
    }
    
    quit:;

    // STUDY: Do I need it?
#if 0
    {
        wglMakeCurrent(global_device_context, 0);
        wglDeleteContext(global_opengl_rendering_context);
    }
#endif
    return(0);
}
