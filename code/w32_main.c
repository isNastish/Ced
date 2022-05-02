

#include <windows.h>
#include <stdint.h>
#include <gl/gl.h>

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

global s8 global_running;
global HINSTANCE global_instance_handle;
global HDC global_device_context;
global HGLRC global_opengl_rendering_context;

internal W32_WindowDim w32_get_window_dimension(HWND window){
    W32_WindowDim dimension = {0};
    
    RECT client_rect = {0};
    GetClientRect(window, &client_rect);
    dimension.width = client_rect.right - client_rect.left;
    dimension.height = client_rect.bottom - client_rect.top;
    
    return(dimension);
}

internal b32 w32_init_opengl(HWND window){
    b32 result = 0;

    s32 pixel_format = 0;
    PIXELFORMATDESCRIPTOR pfd = {0};
    
    {
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA; // The kind of framebuffer, RGBA or palette.
        pfd.cColorBits = 32; // Color depth of the framebuffer.
        pfd.cDepthBits = 24; // Number of bits for the depthbuffer.
        pfd.cStencilBits = 8; // Number of bits for the stencilbuffer.
        pfd.iLayerType = PFD_MAIN_PLANE;
    }

    
    global_device_context = GetDC(window);

    pixel_format = ChoosePixelFormat(global_device_context, &pfd);
    SetPixelFormat(global_device_context,
                   pixel_format,
                   &pfd);
    
    global_opengl_rendering_context = wglCreateContext(global_device_context);
    if(wglMakeCurrent(global_device_context,
                      global_opengl_rendering_context)){
        MessageBox(0, (char *)glGetString(GL_VERSION), "OpenGL version", 0);
    }
    else{
        // TODO: error handling.
    }

    return(result);
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

#if 0
            // NOTE: describes the pixel format of a drawing surface. 
            PIXELFORMATDESCRIPTOR pfd = {0};
            
            {
                pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
                pfd.nVersion = 1;
                pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
                pfd.iPixelType = PFD_TYPE_RGBA; // the kind of framebuffer. RGBA or palette.
                pfd.cColorBits = 32; // colordepth of the framebuffer.
                
                pfd.cDepthBits = 24; // number of bits for the depthbuffer.
                pfd.cStencilBits = 8; // number of bits for the stencilbuffer.
                
                pfd.iLayerType = PFD_MAIN_PLANE;
            }
            
            global_device_context = GetDC(window);
            
            s32 format = ChoosePixelFormat(global_device_context, &pfd);
            SetPixelFormat(global_device_context, format, &pfd);
            
            global_opengl_rendering_context = wglCreateContext(global_device_context);
            
            wglMakeCurrent(global_device_context,
                           global_opengl_rendering_context);
            
            MessageBoxA(0, (char *)glGetString(GL_VERSION), "OpenGL Version", 0);
#else
            w32_init_opengl(window);
#endif
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
    
    global_instance_handle = instance;
    
    WNDCLASSA window_class = {0};
    
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW;
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
    
    global_running = 1;
    while(global_running){
        
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
            
            TranslateMessage(&message);
            DispatchMessageA(&message);
            
        }
        
        {
            W32_WindowDim dim = w32_get_window_dimension(window_handle);
            glViewport(0, 0, dim.width, dim.height); // NOTE: specifies the boundaries of what you're trying to render.
            glClearColor(0, 0xff, 0, 0); // NOTE: normalized automatically.
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(global_device_context);
        }
    }
    
    quit:;
    
    return(0);
}
